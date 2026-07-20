# UI concept

![Pool dashboard mockup](assets/pool-dashboard-mockup.png)

The approved design direction is inspired by the current Loxone App without
copying an existing screen or using Loxone branding.

## Visual language

- Canvas: 480 x 480 pixels
- Background: near-black charcoal
- Cards: slightly lighter charcoal with subtle borders
- Primary text: white
- Secondary text: muted light gray
- Active and positive state: bright lime green
- Warning: amber
- Error, timeout, or offline state: red
- Spacing: consistent 8-pixel grid
- Large touch targets with clear separation
- Flat appearance without gradients or decorative effects

The dark Loxone-inspired direction, stronger green accents and enlarged touch
controls have been approved in an interactive 480 x 480 draft. Final pixel-level
adjustments remain subject to verification on the Waveshare panel.

## Main screen

The main screen contains:

1. Header with title and Wi-Fi/MQTT status
2. Current water temperature
3. Filter pump, heating pump, and heating permission status
4. Operating modes: Off, Automatic, and Manual
5. Target temperature with minus and plus buttons
6. Connection or warning message

Heating is never shown as an operating mode.

## Interaction rules

- Operating modes are enabled only with a known retained mode and MQTT.
- Target temperature is adjustable only in Automatic mode.
- Target range is 20.0 to 32.0 degrees Celsius in 0.5-degree steps.
- Filter pump control is enabled only in Manual mode.
- All commands remain pending until Loxone confirms the requested status.
- Pending commands show `Wird übernommen ...`.
- Confirmed commands show `Übernommen` for three seconds.
- A timeout shows `Keine Bestätigung von Loxone`.
- The affected control remains disabled while its command is pending.
- Unknown values display `--` instead of defaults.
- Retained values remain usable while MQTT is connected. A command without a
  matching Loxone status response times out after five seconds.
- Offline state disables all command controls.

## Screen power behavior

The wall display should not stay fully lit permanently.

- After 5 minutes without touch input, the framebuffer is cleared to black and
  the backlight is switched off.
- A touch while off wakes the display only and must not trigger a pool command.
- Once awake, the next touch is handled normally.
- The screen-power logic is independent from pool control. It changes only
  display brightness and touch forwarding, never `PoolState`.
- Critical warnings may wake the screen later, but this should be used
  sparingly to avoid unwanted light at night.

## Manual mode variation

When Manual mode is selected:

- The target-temperature buttons are disabled.
- The Filter pump status tile becomes an interactive on/off control.
- Heating status, heating pump, and heating permission remain informational.

## Automatic mode variation

When Automatic mode is selected:

- Target-temperature controls are enabled.
- Filter pump remains informational and cannot be switched manually.

## Implementation boundary

The future LVGL screen reads `PanelViewModel` for control availability,
warnings, and command progress. It must not duplicate permission rules or
modify `PoolState` directly.

Backlight and touch wake behavior uses `ScreenPowerPolicy`. The policy retains
optional dimming for other hardware, but dimming is disabled on the Waveshare
4B because every tested PWM duty/frequency combination visibly flickered. This
panel therefore uses only stable static GPIO levels for fully on and off.

## Current implementation status

The main-screen widget tree is implemented in `lib/Gui/GuiManager.cpp` using
LVGL 8.4. It includes status cards, mode buttons, target-temperature controls,
connection warnings, MQTT command callbacks, and command-progress feedback.

`DisplayManager` registers the LVGL display flush and GT911 input drivers, and
`AppController` starts `GuiManager` in the normal firmware. The approved
dashboard revision uses a compact horizontal water-temperature row, clearly
separated status and control surfaces, a taller operating-mode section, large
target-temperature buttons, and stronger but restrained green accents. It
builds successfully; visual layout and real touch behavior of this revision
still need verification on the physical panel.
