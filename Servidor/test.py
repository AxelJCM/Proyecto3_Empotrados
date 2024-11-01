from flask import Flask, request, jsonify
import jwt
import datetime
from flask_cors import CORS
import random
import base64

app = Flask(__name__)
CORS(app)

SECRET_KEY = "supersecretkey"
VALID_USERNAME = "abc"
VALID_PASSWORD = "123"

# Ruta para iniciar sesión y obtener un token JWT
@app.route('/login', methods=['POST'])
def login():
    data = request.json
    username = data.get('username')
    password = data.get('password')

    if username == VALID_USERNAME and password == VALID_PASSWORD:
        token = jwt.encode({
            'user': username,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(minutes=30)
        }, SECRET_KEY, algorithm="HS256")
        return jsonify({"token": token}), 200
    else:
        return jsonify({"message": "Invalid Credentials"}), 401

# Middleware para verificar el token JWT
def token_required(f):
    def wrapper(*args, **kwargs):
        token = request.headers.get('Authorization')
        if not token:
            return jsonify({"message": "Token is missing!"}), 401
        try:
            token = token.split()[1]
            jwt.decode(token, SECRET_KEY, algorithms=["HS256"])
        except Exception as e:
            return jsonify({"message": "Invalid token!"}), 401
        return f(*args, **kwargs)
    wrapper.__name__ = f.__name__
    return wrapper

# Ruta para simular tomar una foto
@app.route('/take-photo', methods=['POST'])
@token_required
def take_photo():
    simulated_photo_base64 = (
        "iVBORw0KGgoAAAANSUhEUgAAAMgAAADICAYAAACt..."
    )
    return jsonify({"photo": simulated_photo_base64}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)
