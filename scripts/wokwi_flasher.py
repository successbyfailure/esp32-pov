import json
from pathlib import Path

from SCons.Script import DefaultEnvironment


env = DefaultEnvironment()


def _write_wokwi_flasher_args(source, target, env):
    build_dir = Path(env.subst("$BUILD_DIR"))
    framework_dir = Path(env.PioPlatform().get_package_dir("framework-arduinoespressif32"))
    boot_app0_src = framework_dir / "tools" / "partitions" / "boot_app0.bin"
    boot_app0_dst = build_dir / "boot_app0.bin"

    if boot_app0_src.exists():
        boot_app0_dst.write_bytes(boot_app0_src.read_bytes())

    flasher_args = {
        "flash_settings": {
            "flash_mode": "dio",
            "flash_freq": "40m",
            "flash_size": "4MB",
        },
        "flash_files": {
            "0x1000": "bootloader.bin",
            "0x8000": "partitions.bin",
            "0xE000": "boot_app0.bin",
            "0x10000": "firmware.bin",
        },
    }

    (build_dir / "flasher_args.json").write_text(
        json.dumps(flasher_args, indent=2) + "\n", encoding="utf-8"
    )


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", _write_wokwi_flasher_args)
