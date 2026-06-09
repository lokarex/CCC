import argparse
import os

def get_user_environment_variable(name: str) -> str:
    try:
        with open(os.path.expanduser('~/.bashrc'), 'r') as file:
            for line in file:
                if line.startswith(f'export {name}='):
                    return line.split('=', 1)[1].strip().strip('"\'')
    except:
        return ""
    
    return ""

def set_user_environment_variable(name: str, value: str):
    with open(os.path.expanduser('~/.bashrc'), 'a') as file:
        file.write(f'export {name}="{value}"\n')

if __name__ == '__main__':  
    # Parse the command line arguments.
    parser = argparse.ArgumentParser(description="")
    parser.add_argument("--target_path", type=str, required=True, help="The path of the target directory.")
    args = parser.parse_args()

    # Set the CCC_HOME environment variable.
    if get_user_environment_variable('CCC_HOME') == '':
        set_user_environment_variable('CCC_HOME', args.target_path)
    
    # Add the directory where the ccc executable program is located to the Path variable.
    if get_user_environment_variable('PATH') != '$CCC_HOME/build/bin'+':$PATH':
        set_user_environment_variable('PATH', '$CCC_HOME/build/bin'+':$PATH')
    
    # Add the directory where the ccc library is located to the LIBRARY_PATH variable.
    if get_user_environment_variable('LD_LIBRARY_PATH') != '$CCC_HOME/build/lib'+':$LD_LIBRARY_PATH':
        set_user_environment_variable('LD_LIBRARY_PATH', '$CCC_HOME/build/lib'+':$LD_LIBRARY_PATH')
    pass