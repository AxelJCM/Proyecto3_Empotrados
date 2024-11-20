from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import os
import cv2
import numpy as np
from PIL import Image
import re

app = Flask(__name__)
CORS(app)

# Ajustar la ruta de la carpeta de imágenes procesadas
BASE_DIR = os.path.dirname(os.path.abspath(__file__))  # Directorio base del archivo actual
UPLOAD_FOLDER = os.path.join(BASE_DIR, '..', 'uploaded_images')  # Carpeta en el nivel raíz
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

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
        if not normalized_filename.lower().endswith(('.jpg', '.jpeg', '.png', '.bmp')):
            normalized_filename += '.jpg'  # Asignar extensión predeterminada
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

# Endpoint para aplicar filtro y guardar la imagen procesada
@app.route('/apply-filter', methods=['POST'])
def apply_filter():
    if 'image' not in request.files or 'filter' not in request.form:
        return jsonify({'error': 'Image or filter not provided'}), 400

    # Leer la imagen y el filtro
    file = request.files['image']
    filter_type = request.form['filter']
    img = Image.open(file.stream)
    img = cv2.cvtColor(np.array(img), cv2.COLOR_RGB2BGR)

    # Normalizar el nombre del archivo
    original_filename = file.filename
    normalized_filename = re.sub(r'[^\w\-_\.]', '_', original_filename)
    if not normalized_filename.lower().endswith(('.jpg', '.jpeg', '.png', '.bmp')):
        normalized_filename += '.jpg'
    processed_filename = f"processed_{normalized_filename}"
    processed_filepath = os.path.join(UPLOAD_FOLDER, processed_filename)

    # Aplicar el filtro
    if filter_type == 'grayscale':
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)  # Convertir de nuevo a 3 canales
    elif filter_type == 'invert':
        img = cv2.bitwise_not(img)

    # Guardar la imagen procesada
    cv2.imwrite(processed_filepath, img)

    print(f"Aplicando filtro: {filter_type} a la imagen: {normalized_filename}")
    print(f"Imagen procesada guardada como: {processed_filename}")

    return jsonify({'filename': processed_filename}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
