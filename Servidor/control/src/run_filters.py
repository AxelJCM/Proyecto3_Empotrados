import subprocess

def apply_filters(input_file, output_file, filters):
    # Ruta al ejecutable compilado de C
    executable = './execute_filter'

    # Construir la lista de argumentos
    args = [executable] + filters + [input_file, output_file]

    try:
        # Ejecutar el comando
        result = subprocess.run(args, check=True, capture_output=True, text=True)
        print("Filter applied successfully!")
        print("Output:", result.stdout)
    except subprocess.CalledProcessError as e:
        print("Error applying filters:")
        print("Return code:", e.returncode)
        print("Error message:", e.stderr)

# Llamada al script con los filtros y archivos deseados
apply_filters('punk.bmp', 'output.bmp', ['-z'])
