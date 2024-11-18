from flask import Flask, request, jsonify, send_file
from flask_cors import CORS
from werkzeug.security import check_password_hash, generate_password_hash
import cv2
import numpy as np
import io
from PIL import Image
import jwt
import datetime

app = Flask(__name__)
CORS(app)

users_db = {
    "user": generate_password_hash("1234")
}

SECRET_KEY = "your_secret_key_here"

# Login endpoint
@app.route('/login', methods=['POST'])
def login():
    data = request.get_json()
    username = data.get('username')
    password = data.get('password')

    # Validate credentials
    if username in users_db and check_password_hash(users_db[username], password):
        # Generate token
        token = jwt.encode({
            'username': username,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=1)
        }, SECRET_KEY, algorithm='HS256')

        return jsonify({"token": token})
    else:
        return jsonify({"message": "Invalid credentials"}), 401

# Apply filter endpoint
@app.route('/apply-filter', methods=['POST'])
def apply_filter():
    # Obtener el filtro que se quiere aplicar
    filter_type = request.form.get('filter')

    # Leer la imagen desde la solicitud
    file = request.files['image']
    img = Image.open(file.stream)
    img = cv2.cvtColor(np.array(img), cv2.COLOR_RGB2BGR)

    # Aplicar el filtro solicitado
    if filter_type == 'grayscale':
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    elif filter_type == 'sepia':
        sepia_filter = np.array([[0.272, 0.534, 0.131],
                                 [0.349, 0.686, 0.168],
                                 [0.393, 0.769, 0.189]])
        img = cv2.transform(img, sepia_filter)
        img = np.clip(img, 0, 255)
    elif filter_type == 'invert':
        img = cv2.bitwise_not(img)

    # Convertir de vuelta a imagen
    _, img_encoded = cv2.imencode('.jpg', img)
    return send_file(io.BytesIO(img_encoded), mimetype='image/jpeg')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
