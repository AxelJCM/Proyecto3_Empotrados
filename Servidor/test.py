from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import os
import subprocess
import re

app = Flask(__name__)
CORS(app)

# Configuración de rutas
BASE_DIR = os.path.dirname(os.path.abspath(__file__))  # Directorio base del archivo actual
UPLOAD_FOLDER = os.path.join(BASE_DIR, '..', 'uploaded_images')  # Carpeta para guardar imágenes
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

# Ruta al ejecutable
EXECUTABLE_PATH = os.path.join(BASE_DIR, 'control', 'src', 'execute_filter')

# Endpoint para subir imágenes
@app.route('/upload-images', methods=['POST'])
def upload_images():
    if 'images' not in request.files:
        return jsonify({'error': 'No files provided'}), 400

    uploaded_files = request.files.getlist('images')
    saved_files = []
    for file in uploaded_files:
        # Normalizar el nombre del archivo
        original_filename = file.filename
        normalized_filename = re.sub(r'[^\w\-_\.]', '_', original_filename)
        if not normalized_filename.lower().endswith(('.bmp')):
            normalized_filename += '.bmp'  # Convertir a BMP si no tiene extensión válida
        filepath = os.path.join(UPLOAD_FOLDER, normalized_filename)
        
        file.save(filepath)
        saved_files.append(normalized_filename)

        print(f"Imagen recibida: {original_filename}")
        print(f"Imagen guardada como: {normalized_filename}")

    return jsonify(saved_files), 200

# Endpoint para listar las imágenes disponibles
@app.route('/list-images', methods=['GET'])
def list_images():
    try:
        images = os.listdir(UPLOAD_FOLDER)
        return jsonify(images), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 500

# Endpoint para servir imágenes estáticas
@app.route('/images/<filename>', methods=['GET'])
def get_image(filename):
    filepath = os.path.join(UPLOAD_FOLDER, filename)
    if not os.path.exists(filepath):
        print(f"Archivo no encontrado: {filepath}")
        return jsonify({'error': 'Archivo no encontrado'}), 404
    print(f"Sirviendo archivo desde: {filepath}")
    return send_from_directory(UPLOAD_FOLDER, filename)

# Endpoint para aplicar múltiples filtros
@app.route('/apply-filters', methods=['POST'])
def apply_filters():
    if 'image' not in request.files or 'filters' not in request.form:
        return jsonify({'error': 'Image or filters not provided'}), 400

    # Leer la imagen y los filtros
    file = request.files['image']
    filters = request.form['filters']  # Filtros separados por comas, por ejemplo: "-z,-b"

    # Normalizar el nombre del archivo
    original_filename = file.filename
    normalized_filename = re.sub(r'[^\w\-_\.]', '_', original_filename)
    if not normalized_filename.lower().endswith('.bmp'):
        normalized_filename += '.bmp'

    input_filepath = os.path.join(UPLOAD_FOLDER, normalized_filename)
    file.save(input_filepath)

    # Ruta para el archivo de salida
    processed_filename = f"processed_{normalized_filename}"
    output_filepath = os.path.join(UPLOAD_FOLDER, processed_filename)

    # Construir y ejecutar el comando para el ejecutable en C
    filters_list = filters.split(',')  # Separar los filtros en una lista
    command = [EXECUTABLE_PATH] + filters_list + [input_filepath, output_filepath]

    try:
        print(f"Ejecutando comando: {' '.join(command)}")
        result = subprocess.run(command, check=True, capture_output=True, text=True)

        print("Filtro aplicado correctamente!")
        print("Salida del ejecutable:", result.stdout)
        return jsonify({'filename': processed_filename}), 200

    except subprocess.CalledProcessError as e:
        print("Error al aplicar los filtros:")
        print("Código de retorno:", e.returncode)
        print("Mensaje de error:", e.stderr)
        return jsonify({'error': e.stderr}), 500

    except Exception as e:
        print(f"Error general al llamar al ejecutable: {str(e)}")
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
