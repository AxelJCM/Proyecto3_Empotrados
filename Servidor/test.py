from flask import Flask, request, jsonify
import jwt
import datetime
from flask_cors import CORS
import random
import base64

app = Flask(__name__)
CORS(app)  # Habilita CORS para todas las rutas

SECRET_KEY = "supersecretkey"
VALID_USERNAME = "abc"
VALID_PASSWORD = "123"

# Estado inicial de las luces, puertas y sensor de movimiento
lights = {
    'cuarto1': 'off',
    'cuarto2': 'off',
    'sala': 'off',
    'comedor': 'off',
    'cocina': 'off'
}
doors = {
    'cuarto1': 'closed',
    'delantera': 'closed',
    'trasera': 'closed',
    'cuarto2': 'closed'
}
motion_sensor = 'No motion'

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
            token = token.split()[1]  # Extraer el token después de 'Bearer'
            jwt.decode(token, SECRET_KEY, algorithms=["HS256"])
        except Exception as e:
            return jsonify({"message": "Invalid token!"}), 401

        return f(*args, **kwargs)

    wrapper.__name__ = f.__name__  # Evita que Flask sobrescriba los nombres de las funciones
    return wrapper

# Ruta para obtener el estado de las luces
@app.route('/status', methods=['GET'])
@token_required
def get_status():
    # Imprimir mensaje en la consola cada vez que se llama este endpoint
    print("Status endpoint called")

    # Simular cambios en luces y puertas
    simulated_lights = {k: random.choice(['on', 'off']) for k in lights}
    simulated_doors = {k: random.choice(['open', 'closed']) for k in doors}

    # Simular un estado de sensor de movimiento
    global motion_sensor
    if random.choice([True, False]):
        motion_sensor = "Motion detected"
    else:
        motion_sensor = "No motion"

    return jsonify({
        'lights': simulated_lights,
        'doors': simulated_doors,
        'motion': motion_sensor
    }), 200

# Ruta para cambiar el estado de una luz
@app.route('/lights/<light>', methods=['POST'])
@token_required
def change_light_status(light):
    if light not in lights:
        return jsonify({"message": "Invalid light"}), 400

    data = request.json
    new_state = data.get('state')

    if new_state not in ['on', 'off']:
        return jsonify({"message": "Invalid state"}), 400

    lights[light] = new_state
    return jsonify({"message": f"Light {light} {new_state}"}), 200

# Ruta para obtener el estado de las puertas
@app.route('/doors', methods=['GET'])
@token_required
def get_doors_status():
    simulated_doors = {k: random.choice(['open', 'closed']) for k in doors}
    return jsonify({"doors": simulated_doors}), 200

# Ruta para obtener el estado del sensor de movimiento
@app.route('/motion-sensor', methods=['GET'])
@token_required
def get_motion_sensor_status():
    global motion_sensor
    if random.choice([True, False]):
        motion_sensor = "Motion detected"
    else:
        motion_sensor = "No motion"
    return jsonify({"status": motion_sensor}), 200

# Ruta para simular tomar una foto (usando una imagen base64 de ejemplo)
@app.route('/take-photo', methods=['POST'])
@token_required
def take_photo():
    # Generar una imagen base64 simulada
    simulated_photo_base64 = (
        "iVBORw0KGgoAAAANSUhEUgAAAMgAAADICAYAAACt..."
        # Imagen base64 cortada para fines ilustrativos
    )
    return jsonify({"photo": simulated_photo_base64}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)
