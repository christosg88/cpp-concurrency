#!/usr/bin/env python3

import sys
import re

template_re = re.compile(r"<[^<>]*>")


def remove_templates() -> int:
    for line in sys.stdin:
        line = line.rstrip("\r\n")
        while True:
            new_line = re.sub(template_re, "", line)
            if new_line == line:
                break
            line = new_line
        print(line)
    return 0


def main():
    sys.exit(remove_templates())


if __name__ == "__main__":
    main()
