from flask import Flask, request, jsonify, send_file, Response
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

# Global variable to store the current filter
current_filter = "none"

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

# Endpoint to update the current filter type
@app.route('/set-filter', methods=['POST'])
def set_filter():
    global current_filter
    filter_type = request.json.get('filter')
    current_filter = filter_type
    return {"message": "Filter updated successfully"}, 200

# Apply filter to image endpoint
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

# Video streaming endpoint
def generate_frames():
    global current_filter
    cap = cv2.VideoCapture(0)  # Use the default camera

    while True:
        success, frame = cap.read()
        if not success:
            break

        # Apply filter based on the current filter setting
        if current_filter == 'grayscale':
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            frame = cv2.cvtColor(frame, cv2.COLOR_GRAY2BGR)  # Convert back to BGR for consistency
        elif current_filter == 'sepia':
            sepia_filter = np.array([[0.272, 0.534, 0.131],
                                     [0.349, 0.686, 0.168],
                                     [0.393, 0.769, 0.189]])
            frame = cv2.transform(frame, sepia_filter)
            frame = np.clip(frame, 0, 255)
        elif current_filter == 'invert':
            frame = cv2.bitwise_not(frame)

        # Encode frame as JPEG
        ret, buffer = cv2.imencode('.jpg', frame)
        frame = buffer.tobytes()

        # Use multipart format to stream the frames
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

@app.route('/video-feed')
def video_feed():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
