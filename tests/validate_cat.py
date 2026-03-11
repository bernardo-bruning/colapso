#!/usr/bin/env python3
import pathlib
import subprocess
import sys
import time


ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
IMAGE = BUILD / "colapso.img"
SERIAL_LOG = BUILD / "cat-serial.log"
MANUAL_TEXT = (ROOT / "MANUAL.txt").read_text()


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


def read_serial():
    if not SERIAL_LOG.exists():
        return ""
    return SERIAL_LOG.read_text(errors="replace")


def send_monitor_command(proc, command):
    if proc.stdin is None:
        raise ValidationError("qemu monitor stdin is unavailable")
    proc.stdin.write(command + "\n")
    proc.stdin.flush()
    time.sleep(0.1)


def process_output(proc):
    if proc.poll() is None:
        return "", ""
    try:
        stdout, stderr = proc.communicate(timeout=1)
    except Exception:
        return "", ""
    return stdout or "", stderr or ""


def main():
    run(["make", "all"], timeout=30)
    SERIAL_LOG.unlink(missing_ok=True)

    cmd = [
        "qemu-system-i386",
        "-drive",
        f"file={IMAGE},format=raw",
        "-display",
        "none",
        "-serial",
        f"file:{SERIAL_LOG}",
        "-monitor",
        "stdio",
        "-no-reboot",
    ]

    proc = subprocess.Popen(
        cmd,
        cwd=ROOT,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    try:
        deadline = time.time() + 5
        while time.time() < deadline:
            data = read_serial()
            if "root@colapso:/" in data:
                break
            if proc.poll() is not None:
                stdout, stderr = process_output(proc)
                raise ValidationError(
                    "qemu exited before the shell prompt appeared\n"
                    f"exit={proc.returncode}\n"
                    f"serial log:\n{data}\n"
                    f"monitor stdout:\n{stdout}\n"
                    f"monitor stderr:\n{stderr}"
                )
            time.sleep(0.1)
        else:
            raise ValidationError(f"serial log did not contain shell prompt\nserial log:\n{read_serial()}")

        for key in ["c", "a", "t", "spc", "m", "a", "n", "u", "a", "l", "dot", "t", "x", "t"]:
            send_monitor_command(proc, f"sendkey {key}")
        send_monitor_command(proc, "sendkey ret")

        deadline = time.time() + 5
        while time.time() < deadline:
            data = read_serial()
            if "root@colapso:/# cat manual.txt" in data and MANUAL_TEXT in data and data.count("root@colapso:/# ") >= 2:
                print("cat validation ok")
                return
            time.sleep(0.1)

        raise ValidationError(f"cat output not found in serial log\nserial log:\n{read_serial()}")
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


if __name__ == "__main__":
    try:
        main()
    except ValidationError as exc:
        print(f"validation failed: {exc}", file=sys.stderr)
        sys.exit(1)
