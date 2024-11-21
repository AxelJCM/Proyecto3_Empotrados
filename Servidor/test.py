from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import os
import re
from PIL import Image
import subprocess

app = Flask(__name__)
CORS(app)

# Configuración de rutas
BASE_DIR = os.path.dirname(os.path.abspath(__file__))  # Directorio base del archivo actual
UPLOAD_FOLDER = os.path.join(BASE_DIR, '..', 'uploaded_images')  # Carpeta para guardar imágenes
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

# Ruta al ejecutable (para aplicar filtros en C)
EXECUTABLE_PATH = os.path.join(BASE_DIR, 'control', 'execute_filter')

# Función para convertir imágenes a BMP
def convert_to_bmp(input_file, output_file):
    try:
        with Image.open(input_file) as img:
            img = img.convert('RGB')  # Convertir a formato compatible
            img.save(output_file, 'BMP')
            print(f"Imagen convertida a BMP: {output_file}")
    except Exception as e:
        print(f"Error al convertir la imagen a BMP: {str(e)}")
        raise e

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

    # Normalizar el nombre del archivo (sin la extensión original)
    original_filename = os.path.splitext(file.filename)[0]  # Remover extensión
    normalized_filename = re.sub(r'[^\w\-_\.]', '_', original_filename) + '.bmp'

    # Ruta para guardar la imagen BMP
    input_filepath = os.path.join(UPLOAD_FOLDER, normalized_filename)

    # Convertir la imagen cargada a BMP
    temp_filepath = os.path.join(UPLOAD_FOLDER, f"temp_{file.filename}")
    file.save(temp_filepath)  # Guardar temporalmente la imagen
    try:
        convert_to_bmp(temp_filepath, input_filepath)  # Convertir a BMP
        os.remove(temp_filepath)  # Eliminar el archivo temporal
    except Exception as e:
        os.remove(temp_filepath)  # Limpiar el archivo temporal en caso de error
        return jsonify({'error': f'Error al convertir la imagen a BMP: {str(e)}'}), 500

    # Ruta para el archivo de salida procesado
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

        # Eliminar el archivo original después de procesarlo
        if os.path.exists(input_filepath):
            os.remove(input_filepath)
            print(f"Imagen original eliminada: {input_filepath}")

        return jsonify({'filename': processed_filename}), 200

    except subprocess.CalledProcessError as e:
        print("Error al aplicar los filtros:")
        print("Código de retorno:", e.returncode)
        print("Mensaje de error:", e.stderr)
        return jsonify({'error': e.stderr}), 500

    except Exception as e:
        print(f"Error general al llamar al ejecutable: {str(e)}")
        return jsonify({'error': str(e)}), 500

@app.route('/delete-image/<filename>', methods=['OPTIONS', 'DELETE'])
def delete_image(filename):
    # Manejo de la solicitud OPTIONS (CORS preflight)
    if request.method == 'OPTIONS':
        response = jsonify({'message': 'CORS preflight passed'})
        response.headers.add("Access-Control-Allow-Origin", "*")
        response.headers.add("Access-Control-Allow-Methods", "DELETE, OPTIONS")
        response.headers.add("Access-Control-Allow-Headers", "Content-Type, Authorization")
        return response, 200

    # Manejo de la solicitud DELETE
    filepath = os.path.join(UPLOAD_FOLDER, filename)
    if not os.path.exists(filepath):
        return jsonify({'error': 'Archivo no encontrado'}), 404

    try:
        os.remove(filepath)
        print(f"Imagen eliminada: {filepath}")
        return jsonify({'message': f'Imagen {filename} eliminada correctamente'}), 200
    except Exception as e:
        print(f"Error al eliminar la imagen: {str(e)}")
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
