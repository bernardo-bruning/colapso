#!/usr/bin/env python3
import pathlib
import struct
import subprocess
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
IMAGE = BUILD / "colapso.img"
DIRECTORY = BUILD / "directory.bin"


class ValidationError(Exception):
    pass


def run(cmd, *, timeout=None, check=True):
    result = subprocess.run(
        cmd,
        cwd=ROOT,
        text=True,
        capture_output=True,
        timeout=timeout,
    )
    if check and result.returncode != 0:
        raise ValidationError(
            f"command failed: {' '.join(cmd)}\n"
            f"exit={result.returncode}\n"
            f"stdout:\n{result.stdout}\n"
            f"stderr:\n{result.stderr}"
        )
    return result


def sector_count_for_size(size):
    return (size + 511) // 512


def ensure_text(data):
    if data is None:
        return ""
    if isinstance(data, bytes):
        return data.decode("utf-8", errors="replace")
    return data


def load_directory_entries():
    data = DIRECTORY.read_bytes()
    if len(data) != 512:
        raise ValidationError(f"directory.bin must be 512 bytes, got {len(data)}")

    entries = []
    for offset in range(0, 512, 32):
        raw = data[offset : offset + 32]
        name, start_lba, count, is_exec, active = struct.unpack("<16sIIII", raw)
        name = name.split(b"\x00", 1)[0].decode("ascii", errors="strict")
        if active:
            entries.append(
                {
                    "name": name,
                    "start_lba": start_lba,
                    "sector_count": count,
                    "is_executable": is_exec,
                }
            )
    return entries


def assert_image_layout():
    if not IMAGE.exists():
        raise ValidationError("image was not produced")

    size = IMAGE.stat().st_size
    if size != 10 * 1024 * 1024:
        raise ValidationError(f"image size mismatch: expected 10485760, got {size}")

    image = IMAGE.read_bytes()
    if image[510:512] != b"\x55\xaa":
        raise ValidationError("boot signature 0xAA55 not found in sector 0")

    entries = load_directory_entries()
    expected = {
        "bash.bin": BUILD / "bash.bin",
        "bin/ls": BUILD / "ls.bin",
        "bin/hello": BUILD / "hello.bin",
    }

    if {entry["name"] for entry in entries} != set(expected):
        raise ValidationError(f"unexpected directory entries: {entries}")

    for entry in entries:
        binary_path = expected[entry["name"]]
        binary = binary_path.read_bytes()
        sectors_needed = sector_count_for_size(len(binary))
        if sectors_needed > entry["sector_count"]:
            raise ValidationError(
                f"{entry['name']} does not fit allocated sectors: "
                f"needs {sectors_needed}, directory has {entry['sector_count']}"
            )

        start = entry["start_lba"] * 512
        end = start + len(binary)
        if image[start:end] != binary:
            raise ValidationError(f"{entry['name']} payload mismatch at LBA {entry['start_lba']}")


def assert_boot_smoke():
    cmd = [
        "qemu-system-i386",
        "-drive",
        f"file={IMAGE},format=raw",
        "-nographic",
        "-monitor",
        "none",
        "-serial",
        "stdio",
        "-no-reboot",
    ]

    try:
        result = subprocess.run(
            cmd,
            cwd=ROOT,
            text=True,
            capture_output=True,
            timeout=3,
            check=False,
        )
    except subprocess.TimeoutExpired as exc:
        output = ensure_text(exc.stdout) + ensure_text(exc.stderr)
        if "Booting from Hard Disk..B" not in output or "K" not in output:
            raise ValidationError(
                "boot stayed alive but expected serial markers were missing\n"
                f"output:\n{output}"
            )
        return

    output = result.stdout + result.stderr
    raise ValidationError(
        "qemu exited before timeout; this usually means reboot/triple fault during boot\n"
        f"exit={result.returncode}\n"
        f"output:\n{output}"
    )


def main():
    run(["make", "clean"])
    run(["make", "all"], timeout=30)
    assert_image_layout()
    assert_boot_smoke()
    print("validation ok")


if __name__ == "__main__":
    try:
        main()
    except ValidationError as exc:
        print(f"validation failed: {exc}", file=sys.stderr)
        sys.exit(1)
