# this is the inner main, the real program

import argparse
import os.path
import sys

import diag

program_name = os.path.basename(__file__)
program_ini_name = program_name + ".ini"

def main():
    print(sys.path)
    parser = diff_commandline()

    diag.output(program_name)
    diag.output(program_ini_name)

    args = parser.parse_args()
    diag.output(args)

def diff_commandline() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()

    parser.add_argument("changelist", metavar='CL', help="changelist to diff")
    parser.add_argument("--output", "-o", default=None, help="write patch to FILE")

    return parser

# ------------------------------------------------------------------------------------------------

if __name__ == '__main__':
    main()
