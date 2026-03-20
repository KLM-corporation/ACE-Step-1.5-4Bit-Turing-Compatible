# ACE-Step VST3 Arturia-Inspired UI Direction

## Goal

Move the current UI away from neon sci-fi and toward a premium hardware-instrument look:

- brushed metal or matte charcoal faceplate
- restrained cream typography
- warm amber indicator lights
- tactile controls with depth and shadow
- clear module separation like a real desktop synth

This should feel closer to Arturia's software instruments than to a futuristic EDM plugin skin.

## Visual Principles

1. Hardware first

The plugin should look like a photographed instrument panel translated into software:

- thick faceplate
- inset display areas
- engraved labels
- physical spacing between control clusters
- stronger shadows under controls than around them

2. Warm premium tone

Use warm, slightly aged neutrals instead of bright cyan/glow-heavy accents.

3. Focused ornament

Keep decorative details purposeful:

- screw heads
- etched divider lines
- tiny LED status lamps
- subtle brushed texture

Avoid loud gradients, glassmorphism, and nightclub neon.

## Color System

- Chassis: `#2C2B2A`
- Panel Dark: `#1F1E1D`
- Panel Mid: `#3B3937`
- Metal Highlight: `#57524D`
- Label Primary: `#E7DFCF`
- Label Secondary: `#B8AD9B`
- Accent Amber: `#F2A65A`
- Accent Red: `#D96C5F`
- Accent Olive: `#7E8B63`
- Display Background: `#20261E`
- Display Text: `#B9D7A8`

## Typography

- Main title: bold geometric sans, tightly tracked
- Section labels: all-caps condensed sans
- Value readouts: mono or pseudo-LCD style
- Secondary helper text: smaller warm gray sans

The title should feel like product branding, not app UI.

## Component Language

### Knobs

- circular with metallic ring
- small highlight arc
- center cap darker than outer ring
- value marker in amber or cream

### Buttons

- chunky rectangular transport-style buttons
- mild bevel or inset shadow
- active state uses amber, not cyan

### Dropdowns and fields

- dark inset wells
- engraved border
- small right-side chevron
- labels above, values centered vertically

### Status

- small LED pill for backend state
- amber progress lamp for rendering
- red lamp for failure

## Layout Direction

Top strip:

- product badge on the left
- tiny preset/display strip in the center
- backend lock LED and engine label on the right

Left bank:

- prompt and lyric entry like a command/programmer section

Center bank:

- engine controls as a knob-and-switch cluster

Right bank:

- transport / generate / result routing

Bottom bank:

- preview deck with large display and transport controls

## Wireframe

```text
+----------------------------------------------------------------------------------+
| ACE-STEP 1.5                     GENERATIVE INSTRUMENT                 LOCK  ●   |
| [ preset / engine strip ..................................................... ] |
+-------------------------------+---------------------------+----------------------+
| PROMPT PROGRAMMER             | ENGINE                    | RENDER               |
| Prompt                        | Length      Seed          | Status LED           |
| [..........................]  | [ knob ]    [ knob ]      | Job state            |
| Lyrics                        | Model       Quality       | Error text           |
| [..........................]  | [ knob ]    [ switch ]    | [ GENERATE ]         |
| [..........................]  | Take select               |                      |
+-------------------------------+---------------------------+----------------------+
| PREVIEW DECK                                                                   |
| [ result display / file path / waveform hint ................................] |
| [ LOAD ]   [ PLAY ]   [ STOP ]   [ CLEAR ]   [ REVEAL ]                       |
+----------------------------------------------------------------------------------+
```

## Implementation Notes

To move the current JUCE UI toward this style:

1. Replace bright cyan-dominant accents with warm amber and cream.
2. Add faceplate texture or subtle stripe noise in `paint()`.
3. Turn the current right-side controls into pseudo-knob modules where possible.
4. Convert backend and render status into lamp indicators instead of plain text-first UI.
5. Reduce the number of rounded modern-app cards; use flatter hardware panels with inset wells.

## What To Change Next In Code

- Rework `PluginEditor.cpp` background painting to feel like machined hardware.
- Update `PluginLookAndFeel` in `PluginEditor.cpp` to use warmer controls.
- Rename visible UI copy in `PluginEditorState.cpp` away from cyberpunk terms.
- Add simple decorative hardware details: screws, divider rules, LED lamps.
