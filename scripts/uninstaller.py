import sys

from _installer_common import (
    get_bashrc_path, read_bashrc, write_bashrc,
    backup_bashrc, remove_existing_ccc_exports,
)


def main():
    bashrc_path = get_bashrc_path()
    lines = read_bashrc()

    original_count = len(lines)
    lines = remove_existing_ccc_exports(lines)

    if len(lines) == original_count:
        print(f"ccc is not installed (no ccc exports found in {bashrc_path}).")
        sys.exit(0)

    backup_bashrc()

    write_bashrc(lines)
    print(f"ccc uninstalled successfully. ccc exports removed from {bashrc_path}.")
    print("Run 'source ~/.bashrc' to apply changes to the current shell.")


if __name__ == "__main__":
    main()
