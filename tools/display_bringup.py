#!/usr/bin/env python3
"""Build, upload, or monitor the isolated display bring-up firmware."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
ENVIRONMENT = "esp32-s3-display-bringup"


def find_platformio() -> str:
    """Find PlatformIO on PATH or in its standard virtual environment."""
    command = shutil.which("pio") or shutil.which("platformio")
    if command:
        return command

    executable = "pio.exe" if os.name == "nt" else "pio"
    fallback = Path.home() / ".platformio" / "penv"
    fallback /= "Scripts" if os.name == "nt" else "bin"
    fallback /= executable
    if fallback.is_file():
        return str(fallback)

    raise SystemExit(
        "PlatformIO wurde nicht gefunden. Installiere zuerst PlatformIO Core "
        "oder die PlatformIO-Erweiterung für VS Code."
    )


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Waveshare-Display-Bring-up plattformunabhängig ausführen."
    )
    parser.add_argument(
        "action",
        nargs="?",
        choices=("build", "upload", "monitor"),
        default="build",
        help="Aktion (Standard: build)",
    )
    parser.add_argument(
        "--port",
        help="Serieller Port, z. B. /dev/cu.usbmodem1101 oder COM5",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    pio = find_platformio()
    env = os.environ.copy()
    env["PLATFORMIO_CORE_DIR"] = str(PROJECT_ROOT / ".pio" / "bringup-core")

    if args.action == "monitor":
        command = [
            pio,
            "device",
            "monitor",
            "--environment",
            ENVIRONMENT,
            "--baud",
            "115200",
        ]
        if args.port:
            command.extend(("--port", args.port))
    else:
        command = [pio, "run", "-e", ENVIRONMENT]
        if args.action == "upload":
            command.extend(("--target", "upload"))
            if args.port:
                command.extend(("--upload-port", args.port))

    print("PlatformIO:", pio)
    print("Core-Verzeichnis:", env["PLATFORMIO_CORE_DIR"])
    return subprocess.call(command, cwd=PROJECT_ROOT, env=env)


if __name__ == "__main__":
    sys.exit(main())
