"""Unit tests for optional 5Hz LM backend compatibility helpers."""

from __future__ import annotations

import unittest
from types import SimpleNamespace
from unittest.mock import MagicMock, patch

from acestep.llm_backend_compat import get_vllm_preflight_warning

try:
    from acestep.llm_inference import LLMHandler

    _IMPORT_ERROR = None
except ImportError as exc:  # pragma: no cover - dependency guard
    LLMHandler = None
    _IMPORT_ERROR = exc


class VllmBackendCompatTests(unittest.TestCase):
    """Verify backend preflight guards for optional Triton-dependent paths."""

    def test_get_vllm_preflight_warning_returns_message_on_windows_without_triton(self) -> None:
        """Windows CUDA should skip vLLM when Triton imports are unavailable."""
        with patch(
            "acestep.llm_backend_compat._has_working_triton_installation",
            return_value=False,
        ):
            warning = get_vllm_preflight_warning(device="cuda", platform="win32")

        self.assertIsNotNone(warning)
        self.assertIn("Falling back to the PyTorch backend", warning)

    def test_get_vllm_preflight_warning_returns_none_outside_windows_cuda(self) -> None:
        """Non-Windows or non-CUDA setups should not be blocked by the Triton preflight."""
        with patch(
            "acestep.llm_backend_compat._has_working_triton_installation",
            return_value=False,
        ):
            warning = get_vllm_preflight_warning(device="cpu", platform="win32")

        self.assertIsNone(warning)


@unittest.skipIf(LLMHandler is None, f"llm_inference import unavailable: {_IMPORT_ERROR}")
class LlmInitializeBackendCompatTests(unittest.TestCase):
    """Verify ``LLMHandler.initialize`` uses the Windows Triton preflight cleanly."""

    @patch("torch.cuda.synchronize")
    @patch("torch.cuda.empty_cache")
    @patch("torch.cuda.is_available", return_value=True)
    @patch("acestep.llm_inference.MetadataConstrainedLogitsProcessor")
    @patch("acestep.llm_inference.get_global_gpu_config")
    @patch("acestep.llm_inference.AutoTokenizer.from_pretrained")
    @patch("acestep.llm_inference.get_vllm_preflight_warning")
    def test_initialize_skips_vllm_when_windows_triton_is_unavailable(
        self,
        mock_preflight_warning: MagicMock,
        mock_tokenizer: MagicMock,
        mock_gpu_config: MagicMock,
        _mock_processor: MagicMock,
        _mock_cuda_available: MagicMock,
        _mock_empty_cache: MagicMock,
        _mock_synchronize: MagicMock,
    ) -> None:
        """Initialization should avoid nano-vllm when the Windows Triton preflight fails."""
        handler = LLMHandler()
        mock_tokenizer.return_value = MagicMock()
        mock_gpu_config.return_value = SimpleNamespace(max_duration_with_lm=600, tier="tier6")
        mock_preflight_warning.return_value = (
            "vLLM backend is unavailable on Windows because Triton is not installed "
            "or is incompatible. Falling back to the PyTorch backend. "
            "Use --backend pt to suppress this warning."
        )

        with patch("acestep.llm_inference.os.path.exists", return_value=True), patch.object(
            handler,
            "_load_pytorch_model",
            return_value=(
                True,
                "ok\nBackend: PyTorch\nDevice: cuda",
            ),
        ) as load_pytorch_model, patch.object(
            handler,
            "_initialize_5hz_lm_vllm",
        ) as init_vllm:
            status, ok = handler.initialize(
                checkpoint_dir="C:/repo/checkpoints",
                lm_model_path="acestep-5Hz-lm-0.6B",
                backend="vllm",
                device="cuda",
            )

        self.assertTrue(ok)
        load_pytorch_model.assert_called_once()
        init_vllm.assert_not_called()
        self.assertIn("Backend: PyTorch", status)
        self.assertIn("vLLM backend is unavailable on Windows", status)

    @patch("acestep.llm_inference.AutoModelForCausalLM.from_pretrained")
    def test_load_pytorch_model_uses_readable_status_markers(
        self,
        mock_from_pretrained: MagicMock,
    ) -> None:
        """PyTorch LM status strings should use readable ASCII markers."""
        handler = LLMHandler()
        mock_model = MagicMock()
        mock_model.to.return_value = mock_model
        mock_model.eval.return_value = None
        mock_from_pretrained.return_value = mock_model
        handler.dtype = "float16"
        handler.offload_to_cpu = False

        ok, status = handler._load_pytorch_model("C:/repo/checkpoints/model", "cuda")

        self.assertTrue(ok)
        self.assertIn("[OK] 5Hz LM initialized successfully", status)
        self.assertTrue(status.isascii())

        mock_from_pretrained.side_effect = RuntimeError("boom")
        ok, status = handler._load_pytorch_model("C:/repo/checkpoints/model", "cuda")

        self.assertFalse(ok)
        self.assertIn("[ERROR] Error initializing 5Hz LM", status)
        self.assertTrue(status.isascii())


if __name__ == "__main__":
    unittest.main()
