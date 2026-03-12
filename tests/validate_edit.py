#!/usr/bin/env python3
import pathlib
import subprocess
import sys
import time


ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
IMAGE = BUILD / "colapso.img"


class ValidationError(Exception):
    pass


def run(cmd, *, timeout=None):
    result = subprocess.run(
        cmd,
        cwd=ROOT,
        text=True,
        capture_output=True,
        timeout=timeout,
    )
    if result.returncode != 0:
        raise ValidationError(
            f"command failed: {' '.join(cmd)}\n"
            f"exit={result.returncode}\n"
            f"stdout:\n{result.stdout}\n"
            f"stderr:\n{result.stderr}"
        )
    return result


def send_monitor_command(proc, command):
    if proc.stdin is None:
        raise ValidationError("qemu monitor stdin is unavailable")
    proc.stdin.write(command + "\n")
    proc.stdin.flush()
    time.sleep(0.1)


def type_text(proc, text):
    keymap = {" ": "spc", ".": "dot", ",": "comma"}
    for ch in text:
        send_monitor_command(proc, f"sendkey {keymap.get(ch, ch)}")


def run_session(commands, expected_tokens, serial_name):
    serial_log = BUILD / serial_name
    serial_log.unlink(missing_ok=True)

    proc = subprocess.Popen(
        [
            "qemu-system-i386",
            "-drive",
            f"file={IMAGE},format=raw",
            "-display",
            "none",
            "-serial",
            f"file:{serial_log}",
            "-monitor",
            "stdio",
            "-no-reboot",
        ],
        cwd=ROOT,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    def read_serial():
        if not serial_log.exists():
            return ""
        return serial_log.read_text(errors="replace")

    try:
        deadline = time.time() + 5
        while time.time() < deadline:
            if "root@colapso:/" in read_serial():
                break
            time.sleep(0.1)
        else:
            raise ValidationError(f"shell prompt not found\nserial log:\n{read_serial()}")

        for command in commands:
            type_text(proc, command)
            send_monitor_command(proc, "sendkey ret")

        deadline = time.time() + 5
        while time.time() < deadline:
            data = read_serial()
            if all(token in data for token in expected_tokens):
                return
            time.sleep(0.1)

        raise ValidationError(f"edit output not found in serial log\nserial log:\n{read_serial()}")
    finally:
        try:
            send_monitor_command(proc, "quit")
        except Exception:
            pass
        if proc.stdin is not None:
            try:
                proc.stdin.close()
            except Exception:
                pass
        try:
            proc.wait(timeout=2)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait(timeout=2)


def main():
    run(["make", "all"], timeout=30)

    run_session(
        ["edit manual.txt", "1p", "2p", "1,2n", "q"],
        [
            "root@colapso:/# edit manual.txt",
            "141",
            "1p",
            "Colapso OS 0.1",
            "2p",
            "===============",
            "1,2n",
            "1\tColapso OS 0.1",
            "2\t===============",
        ],
        "edit-existing-serial.log",
    )

    print("edit validation ok")


if __name__ == "__main__":
    try:
        main()
    except ValidationError as exc:
        print(f"validation failed: {exc}", file=sys.stderr)
        sys.exit(1)
