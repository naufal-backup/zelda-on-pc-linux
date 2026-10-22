#!/usr/bin/env python3
"""Send commands to the dev bridge of Zelda on PC (127.0.0.1:28770).

Usage: pce.py <command> [<command> ...]
Each command is answered on its own line.
"""
import socket
import sys

HOST, PORT = "127.0.0.1", 28770


def main() -> int:
    cmds = sys.argv[1:]
    if not cmds:
        print("no command", file=sys.stderr)
        return 2
    try:
        s = socket.create_connection((HOST, PORT), timeout=5)
    except OSError as e:
        print(f"err {e}", file=sys.stderr)
        return 1
    with s:
        s.settimeout(10)
        for c in cmds:
            s.sendall((c + "\n").encode())
        buf = b""
        out = []
        while len(out) < len(cmds):
            chunk = s.recv(65536)
            if not chunk:
                break
            buf += chunk
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                out.append(line.decode(errors="replace"))
        for line in out:
            print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
