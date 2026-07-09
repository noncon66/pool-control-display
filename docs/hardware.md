# Hardware status

The intended target is an ESP32-S3 wall panel with a 480 x 480 display. The
exact board revision, display controller, touch controller and pin mapping are
not known yet.

The current code in `lib/Display` is an experimental placeholder and is
excluded from builds. Do not enable it based only on the generic ESP32-S3
product name.

Before enabling display support, record:

- exact product name and shop link
- clear photos of both sides of the board
- PCB revision and controller markings
- manufacturer demo or schematic
- flash and PSRAM size
- display and touch controller types
- backlight pin and active level
