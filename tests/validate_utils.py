#!/usr/bin/env python3
import pathlib
import subprocess
import sys
import time


ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
IMAGE = BUILD / "colapso.img"
SERIAL_LOG = BUILD / "utils-serial.log"


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


def type_text(proc, text):
    keymap = {" ": "spc", ".": "dot", ",": "comma"}
    for ch in text:
        send_monitor_command(proc, f"sendkey {keymap.get(ch, ch)}")


def main():
    run(["make", "all"], timeout=30)
    SERIAL_LOG.unlink(missing_ok=True)

    proc = subprocess.Popen(
        [
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
        ],
        cwd=ROOT,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    try:
        deadline = time.time() + 5
        while time.time() < deadline:
            if "root@colapso:/" in read_serial():
                break
            time.sleep(0.1)
        else:
            raise ValidationError(f"shell prompt not found\nserial log:\n{read_serial()}")

        for command in [
            "help",
            "echo hello utils",
            "pwd",
            "which cat",
            "cd bin",
            "pwd",
            "ls",
            "cd /",
            "stat manual.txt",
            "grep Colapso manual.txt",
            "wc manual.txt",
            "head manual.txt",
            "tail manual.txt",
            "hexdump manual.txt",
            "uname",
            "dmesg",
            "meminfo",
            "touch note.txt",
            "write note.txt hello",
            "cat note.txt",
            "mkdir docs",
            "ls",
            "date",
            "env",
        ]:
            type_text(proc, command)
            send_monitor_command(proc, "sendkey ret")

        deadline = time.time() + 5
        while time.time() < deadline:
            data = read_serial()
            if all(token in data for token in [
                "Comandos disponiveis:",
                "echo hello utils",
                "hello utils",
                "root@colapso:/# pwd",
                "[EXEC bin/pwd]/",
                "root@colapso:/bin# pwd",
                "[EXEC bin/pwd]/bin",
                "root@colapso:/bin# ls",
                "- ls",
                "- cat",
                "bin/cat",
                "nome: MANUAL.txt",
                "lba: 90",
                "setores: 1",
                "Colapso OS",
                "linhas: 5",
                "bytes: 141",
                "43 6F 6C 61 70 73 6F 20",
                "Colapso OS i386",
                "[EXEC bin/dmesg]",
                "kernel: 0x00001000",
                "root@colapso:/# cat note.txt",
                "hello",
                "note.txt",
                "docs",
                "date: RTC ainda nao suportado",
                "env: nenhuma variavel de ambiente definida",
            ]) and data.count("# ") >= 19:
                print("utils validation ok")
                return
            time.sleep(0.1)

        raise ValidationError(f"utility output not found in serial log\nserial log:\n{read_serial()}")
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
