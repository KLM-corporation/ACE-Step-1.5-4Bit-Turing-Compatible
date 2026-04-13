"""Regression tests for the ``_load_scoring_model_context`` lifecycle.

These protect the Autoscore unified-memory fix from issue #1081 by
asserting three invariants that are easy to regress silently:

1. ``_load_scoring_model_context`` performs exactly one load/offload
   cycle per outermost entry, regardless of how deeply nested the
   inner re-entries are.
2. Nested re-entries from the same thread are no-ops and do not
   trigger additional CPU↔accelerator migrations.
3. On the MLX backend with ``offload_to_cpu=True`` the cached HF
   scoring model is released on outermost exit so the ~8 GB duplicate
   PyTorch copy does not remain resident between generations.
"""

import unittest

from acestep.core.scoring.lm_score import _load_scoring_model_context


class _FakeModel:
    """Recorder stand-in for an ``AutoModelForCausalLM``.

    Captures every ``.to(device)`` invocation so tests can assert the
    exact migration sequence produced by a scoring context.
    """

    def __init__(self):
        """Initialise with an empty migration history."""
        self.device_calls = []

    def to(self, device):
        """Record the target device and return ``self`` (chain-friendly)."""
        self.device_calls.append(str(device))
        return self


class _FakeHandler:
    """Handler stub exposing just enough surface for the scoring context.

    Stands in for ``LLMInferenceHandler`` so the context-manager tests
    avoid loading any real model weights.  The handler tracks how many
    times ``get_hf_model_for_scoring`` is queried, which lets nested-
    entry tests prove that inner re-entries do not re-fetch the model.
    """

    def __init__(self, backend: str, offload: bool = True):
        """Configure backend / offload flags and install a fake model.

        Args:
            backend: One of ``"mlx"``, ``"vllm"``, ``"pt"``.  Selects the
                branch of ``_load_scoring_model_context`` under test.
            offload: Whether ``offload_to_cpu`` should be reported as
                enabled.  Controls whether the context manager performs
                any ``.to()`` migrations and whether the MLX release
                path fires on outermost exit.
        """
        self.llm_backend = backend
        self.llm_initialized = True
        self.offload_to_cpu = offload
        # Synthetic target device -- ``_FakeModel.to`` is a recorder so
        # the string never has to correspond to a real torch device.
        self.device = "cuda"
        self._hf_model_for_scoring = _FakeModel()
        self.get_calls = 0

    def get_hf_model_for_scoring(self):
        """Return the cached fake model and bump the query counter."""
        self.get_calls += 1
        return self._hf_model_for_scoring


class LoadScoringModelContextTests(unittest.TestCase):
    """Lifecycle-contract tests for ``_load_scoring_model_context``."""

    def test_single_load_offload_per_outer_context_mlx(self):
        """One outer entry should trigger exactly one load and one offload."""
        handler = _FakeHandler("mlx")
        model = handler._hf_model_for_scoring
        with _load_scoring_model_context(handler):
            pass
        # Exactly one load (to accelerator) and one offload (to cpu).
        self.assertEqual(
            model.device_calls, ["cuda", "cpu"],
            "expected exactly one load+offload cycle",
        )

    def test_nested_entries_are_noops_mlx(self):
        """Nested re-entries must not move the model again."""
        handler = _FakeHandler("mlx")
        model = handler._hf_model_for_scoring
        with _load_scoring_model_context(handler):
            calls_after_outer_load = list(model.device_calls)
            # Deep nesting (simulating _get_logits_and_target_for_scoring
            # called many times per Autoscore pass).
            with _load_scoring_model_context(handler):
                with _load_scoring_model_context(handler):
                    # Inner contexts must not migrate the model again.
                    self.assertEqual(model.device_calls, calls_after_outer_load)
        # After the outermost exit the handler has dropped the cached
        # model, so use ``model`` (the captured reference) to verify the
        # offload.
        self.assertEqual(
            model.device_calls, ["cuda", "cpu"],
            "nested entries should not add extra migrations",
        )

    def test_mlx_outermost_exit_drops_cached_model(self):
        """On MLX+offload, outer exit must clear ``_hf_model_for_scoring``."""
        handler = _FakeHandler("mlx", offload=True)
        self.assertIsNotNone(handler._hf_model_for_scoring)
        with _load_scoring_model_context(handler):
            # Still cached while we're inside the context.
            self.assertIsNotNone(handler._hf_model_for_scoring)
        # Released after outermost exit so unified memory is returned to OS.
        self.assertIsNone(handler._hf_model_for_scoring)

    def test_vllm_outermost_exit_keeps_cached_model(self):
        """vllm backend must NOT drop the cached HF model (CUDA is fine)."""
        handler = _FakeHandler("vllm", offload=True)
        cached = handler._hf_model_for_scoring
        with _load_scoring_model_context(handler):
            pass
        # vllm keeps the cached HF scoring model between passes; the
        # MLX-specific release path must not fire here.
        self.assertIs(handler._hf_model_for_scoring, cached)

    def test_mlx_no_offload_keeps_cached_model(self):
        """Without offload_to_cpu the MLX release path must not fire."""
        handler = _FakeHandler("mlx", offload=False)
        cached = handler._hf_model_for_scoring
        with _load_scoring_model_context(handler):
            pass
        # Not offloading means no load/offload transitions and no drop.
        self.assertIs(handler._hf_model_for_scoring, cached)
        self.assertEqual(cached.device_calls, [])

    def test_get_hf_called_only_by_outermost_entry(self):
        """Nested entries must not re-query ``get_hf_model_for_scoring``."""
        handler = _FakeHandler("mlx")
        with _load_scoring_model_context(handler):
            with _load_scoring_model_context(handler):
                with _load_scoring_model_context(handler):
                    pass
        # Outer entry queries once; nested entries are pure no-ops.
        self.assertEqual(handler.get_calls, 1)


if __name__ == "__main__":
    unittest.main()
