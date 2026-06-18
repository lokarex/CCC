import argparse
import os
import sys

from _installer_common import (
    get_bashrc_path, read_bashrc, write_bashrc,
    build_marker_block, backup_bashrc,
    remove_existing_ccc_exports,
)


def main():
    parser = argparse.ArgumentParser(description="Install ccc environment variables to ~/.bashrc")
    parser.add_argument("--target_path", type=str, required=True,
                        help="The path of the target directory.")
    args = parser.parse_args()

    target_path = args.target_path

    if not os.path.isdir(target_path):
        print(f"Error: target path does not exist or is not a directory: {target_path}",
              file=sys.stderr)
        sys.exit(1)

    bashrc_path = get_bashrc_path()
    lines = read_bashrc()

    backup_bashrc()

    lines = remove_existing_ccc_exports(lines)

    block = build_marker_block(target_path)
    if lines and not lines[-1].endswith("\n"):
        lines.append("\n")
    lines.extend(block)

    write_bashrc(lines)
    print(f"ccc installed successfully. Environment variables written to {bashrc_path}.")
    print("Run 'source ~/.bashrc' to apply changes to the current shell.")


if __name__ == "__main__":
    main()
