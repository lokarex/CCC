import os
import shutil
import sys

MARKER_START = "# >>> ccc installer >>>"
MARKER_END = "# <<< ccc installer <<<"

VARIABLES = ["CCC_HOME", "PATH", "LD_LIBRARY_PATH", "LIBRARY_PATH"]


def get_bashrc_path():
    return os.path.expanduser("~/.bashrc")


def read_bashrc():
    path = get_bashrc_path()
    if not os.path.exists(path):
        return []
    with open(path, "r", encoding="utf-8") as f:
        return f.readlines()


def write_bashrc(lines):
    path = get_bashrc_path()
    with open(path, "w", encoding="utf-8") as f:
        f.writelines(lines)


def find_marker_range(lines):
    start = None
    end = None
    for i, line in enumerate(lines):
        if line.strip() == MARKER_START:
            start = i
        if start is not None and line.strip() == MARKER_END:
            end = i
            break
    if start is not None and end is not None:
        return start, end
    return None


def normalize_for_bashrc(target_path):
    if sys.platform == "win32":
        target_path = target_path.replace("\\", "/")
        if len(target_path) >= 2 and target_path[1] == ":":
            target_path = "/" + target_path[0].lower() + target_path[2:]
    return target_path


def build_marker_block(target_path):
    target_path = normalize_for_bashrc(target_path)
    lines = [f"{MARKER_START}\n"]
    lines.append(f'export CCC_HOME="{target_path}"\n')
    lines.append(f'export PATH="$CCC_HOME/build/bin:$CCC_HOME/build/lib:$PATH"\n')
    lines.append(f'export LD_LIBRARY_PATH="$CCC_HOME/build/lib:$LD_LIBRARY_PATH"\n')
    lines.append(f'export LIBRARY_PATH="$CCC_HOME/build/lib:$LIBRARY_PATH"\n')
    lines.append(f"{MARKER_END}\n")
    return lines


def remove_existing_ccc_exports(lines):
    marker_range = find_marker_range(lines)
    if marker_range is not None:
        start, end = marker_range
        del lines[start:end + 1]

    result = []
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("export ") and "CCC_HOME" in stripped:
            continue
        result.append(line)
    return result


def backup_bashrc():
    path = get_bashrc_path()
    if os.path.exists(path):
        backup_path = path + ".bak"
        shutil.copy2(path, backup_path)
        return backup_path
    return None
