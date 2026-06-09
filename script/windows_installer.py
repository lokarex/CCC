import argparse
import os
import winreg


def get_user_environment_variable(name: str) -> str:
    try:
        with winreg.ConnectRegistry(None, winreg.HKEY_CURRENT_USER) as hkey:
            with winreg.OpenKey(hkey, r'Environment', 0, winreg.KEY_ALL_ACCESS) as sub_key:
                value, _ = winreg.QueryValueEx(sub_key, name)
    except:
        return ""
    
    return value

def set_user_environment_variable(name: str, value: str):
    with winreg.ConnectRegistry(None, winreg.HKEY_CURRENT_USER) as hkey:
        with winreg.OpenKey(hkey, r'Environment', 0, winreg.KEY_ALL_ACCESS) as sub_key:
            winreg.SetValueEx(sub_key, name, 0, winreg.REG_EXPAND_SZ, value)

if __name__ == '__main__':
    # Parse the command line arguments.
    parser = argparse.ArgumentParser(description="")
    parser.add_argument("--target_path", type=str, required=True, help="The path of the target directory.")
    args = parser.parse_args()

    # Set the CCC_HOME environment variable.
    if get_user_environment_variable('CCC_HOME') == '':
        set_user_environment_variable('CCC_HOME', args.target_path)

    # Add the directory where the ccc executable program is located to the Path variable.
    Path_Value = get_user_environment_variable('Path')
    if r"%CCC_HOME%\build\bin" not in Path_Value.split(';'):
        set_user_environment_variable('Path', r"%CCC_HOME%\build\bin;" + Path_Value)

    # Add the directory where the ccc library is located to the LIBRARY_PATH variable.
    Path_Value = get_user_environment_variable('Path')
    if r"%CCC_HOME%\build\lib" not in Path_Value.split(';'):
        set_user_environment_variable('Path', Path_Value + r";%CCC_HOME%\build\lib")
    Library_Path_Value = get_user_environment_variable('LIBRARY_PATH')
    if r"%CCC_HOME%\build\lib" not in Library_Path_Value.split(';'):
        set_user_environment_variable('LIBRARY_PATH', r"%CCC_HOME%\build\lib;" + Library_Path_Value)