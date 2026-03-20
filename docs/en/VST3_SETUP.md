# ACE-Step VST3 Setup and Validation

This guide documents the current ACE-Step VST3 MVP workflow, including contributor build steps,
backend setup expectations, user-facing behavior, and the validation matrix for the draft plugin
shell.

## Current MVP Scope

The current VST3 draft implementation is intentionally narrow:

- isolated JUCE/CMake VST3 shell
- state-driven plugin UI for prompt, lyrics, duration, seed, preset, and quality mode
- persisted plugin state across DAW reopen
- local preview-file playback
- reveal-file handoff through the host operating system

The following items are explicitly deferred:

- direct backend requests from the plugin
- reference-audio workflows
- repaint/edit workflows
- drag-and-drop into the DAW timeline
- AU, CLAP, and AAX formats

## Contributor Build

Build requirements:

- CMake 3.22 or newer
- a C++20-capable compiler
- Git access during configure so JUCE can be fetched

Build commands:

```bash
cmake -S plugins/acestep_vst3 -B build/acestep_vst3
cmake --build build/acestep_vst3 --config Release
```

Platform notes:

- Windows: Visual Studio 2022 or Ninja is recommended
- macOS: Xcode or Ninja is recommended
- The plugin workspace is isolated under `plugins/acestep_vst3/` and does not modify the repo's
  Python packaging flow

## Backend Setup

The VST3 MVP architecture assumes a separate ACE-Step backend/API process.

Recommended backend startup for future integration work:

```bash
uv run acestep-api
```

Default local endpoint:

- `http://localhost:8001`

The current draft plugin UI persists the backend URL and status state, but it does not yet make
live API calls. Backend reachability and generation state are currently represented through the MVP
UI shell and mock job-state flow.

## Current User Workflow

In the current draft implementation:

1. Open the plugin inside a supported VST3 host.
2. Enter prompt, lyrics, duration, seed, preset, and quality mode.
3. Set the backend URL and backend status in the UI shell.
4. Trigger the mock generation flow to exercise state transitions.
5. Load a local preview audio file.
6. Play, stop, clear, or reveal the preview file from the plugin UI.

The preview path is intentionally local-file based. Drag-and-drop into the DAW timeline is deferred
for MVP stability.

## Validation Matrix

| Area | Target | Current status | Notes |
|------|--------|----------------|-------|
| Plugin shell build | Windows | Pending manual validation | Build instructions are present; no Windows validation was run in this environment |
| Plugin shell build | macOS | Blocked locally | Local configure attempt was blocked by missing Xcode command line tools |
| Host load | Reaper (Windows) | Pending manual validation | This is the default Windows validation host for the MVP |
| Host load | Reaper (macOS) | Pending manual validation | This is the default macOS validation host for the MVP |
| State persistence | DAW save/reopen | Implemented, pending host validation | Prompt, lyrics, controls, result slots, and preview metadata are serialized |
| Backend offline UX | Plugin UI shell | Implemented | Currently state-driven, not based on real API requests |
| Prompt/job workflow | Plugin UI shell | Implemented | Mock submission, queued/running, success, and failure states are present |
| Preview playback | Local file playback | Implemented, pending host validation | Uses processor-owned JUCE transport primitives |
| File handoff | Reveal file in OS | Implemented, pending host validation | Uses file-based handoff; drag-and-drop is deferred |

## Known Limitations

- The current plugin branch is a shell, not a real backend-integrated generator yet
- Local validation in this workspace is limited by missing Xcode command line tools
- No plugin CI pipeline is active yet
- Preview currently relies on a user-selected local audio file rather than backend-generated output
- Drag-and-drop into the DAW timeline is intentionally out of scope for this milestone

## Tracking

This guide corresponds to the VST3 umbrella issue and its current child issues:

- `#890` umbrella tracker
- `#893` plugin shell
- `#894` plugin UI shell
- `#895` preview playback and file handoff
