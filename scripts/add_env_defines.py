import os
import sys
import subprocess


def parse_env_file(path=".env"):
    env_vars = {}
    if not os.path.exists(path):
        print("⚠️ No se encontró .env")
        return env_vars

    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if "=" not in line:
                continue

            key, value = line.split("=", 1)
            env_vars[key.strip()] = value

    return env_vars


def load_env_defines():
    env_vars = parse_env_file()

    if not env_vars:
        return
    
    if env_vars.get("ROOT_CA") is None:
        print("⚠️ ROOT_CA no definido en .env")
        return
    

    for key, value in env_vars.items():

        if "ROOT_CA" == key:
            raw = env_vars["ROOT_CA"].replace("\\n", "\n")
            os.makedirs("src", exist_ok=True)
            with open("src/root_ca.h", "w") as f:
                f.write('const char ROOT_CA[] = R"EOF(\n')
                f.write(raw)
                f.write('\n)EOF";\n')
            print("✔ Archivo generado: src/root_ca.h")
            continue
        os.environ[key] = value
        print(f"✔ Definido: {key}={value}")
            
    if len(sys.argv) > 1 and sys.argv[1] == 'upload':
        subprocess.run(['pio', 'run', '-t', 'upload'], check=True)
    else:
        subprocess.run(['pio', 'run'], check=True)


if __name__ == "__main__":
    load_env_defines()