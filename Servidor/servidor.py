from flask import Flask, request, jsonify
import jwt
import datetime
from flask_cors import CORS
import ctypes as ct
import time
from control import *
import json
import threading
from PIL import Image
import io
import base64

app = Flask(__name__)
CORS(app)  # Aplica CORS después de definir `app`

# Clave secreta para firmar los tokens
SECRET_KEY = "supersecretkey"

# Credenciales válidas
VALID_USERNAME = "abc"
VALID_PASSWORD = "123"

# Flag hilo
stop = True

# Estado inicial de las luces, puertas y sensor de movimiento
lights = {'Cuarto 1': 'off', 'Cuarto 2': 'off', 'Sala': 'off', 'Baño': 'off', 'Cocina': 'off'}  # 5 luces
lights_pins = [b"517", b"518", b"525", b"531", b"538"]  # 5 luces
doors = {'Principal': 'closed', 'Baño': 'closed', 'Cuarto 1': 'closed', 'Cuarto 2': 'closed'}  # 4 puertas
doors_pins = [b"529", b"539", b"534", b"537"]
motion_sensor = 'No motion'  # Sensor de movimiento

# Pin configuration
pinMode(lights_pins[0], b"out")
pinMode(lights_pins[1], b"out")
pinMode(lights_pins[2], b"out")
pinMode(lights_pins[3], b"out")
pinMode(lights_pins[4], b"out")

pinMode(doors_pins[0], b"in")
pinMode(doors_pins[1], b"in")
pinMode(doors_pins[2], b"in")
pinMode(doors_pins[3], b"in")

trigger_pin = b"535"    # GPIO23
echo_pin = b"536"       # GPIO24

pinMode(trigger_pin, b"out")
pinMode(echo_pin, b"in")

# Funcion para convertir la imagen a Base64 para enviarla por el Jsonify.
def conversephoto(image_path):
    with Image.open(image_path) as img:
        buffered = io.BytesIO()
        img.save(buffered, format="JPEG")
        img_str = base64.b64encode(buffered.getvalue()).decode('utf-8')
    return img_str

# Funcion para la lectura constante del sensor
def sensor_state():
    while stop:
        distancia = getDistance(trigger_pin, echo_pin)
        if 0 < distancia < 10:
            takePicture(b"sensorphoto.jpg")
            print("Save Photo")
        time.sleep(1)

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

# Ruta para obtener el estado de las luces y puertas
@app.route('/status', methods=['GET'])
def get_status():
    print("Sending status response:", {'lights': lights, 'doors': doors, 'motion': motion_sensor})
    return jsonify({
        'lights': lights,
        'doors': doors,
        'motion': motion_sensor
    }), 200

# Ruta para obtener el estado de las luces
@app.route('/lights', methods=['GET'])
@token_required
def get_lights_status():
    light_status = {}
    for i in range(len(lights_pins)):
        value_read = ct.create_string_buffer(4)
        pinl = lights_pins[i]
        digitalRead(pinl, value_read)
        light_status[list(lights.keys())[i]] = 'on' if value_read.value.decode('utf-8') == '1' else 'off'
    return jsonify({"lights": light_status}), 200

# Ruta para cambiar el estado de una luz
@app.route('/lights/<light_name>', methods=['POST'])
@token_required
def change_light_status(light_name):
    if light_name not in lights:
        return jsonify({"message": "Invalid light name"}), 400

    data = request.json
    new_state = data.get('state')

    if new_state not in ['on', 'off']:
        return jsonify({"message": "Invalid state"}), 400

    light_id = list(lights.keys()).index(light_name)
    pinl = lights_pins[light_id]

    digitalWrite(pinl, b"1" if new_state == 'on' else b"0")
    lights[light_name] = new_state
    return jsonify({"message": f"{light_name} is now {new_state}"}), 200

@app.route('/doors', methods=['GET'])
@token_required
def get_doors_status():
    for i in range(len(doors_pins)):
        value_read = ct.create_string_buffer(4)  # string buffer para almacenar el valor leído
        pind = doors_pins[i]  # selecciona el pin correspondiente
        digitalRead(pind, value_read)  # lee el estado del pin
        
        # Actualiza el diccionario `doors` usando la clave correcta
        if value_read.value.decode('utf-8') == '1':
            door_name = list(doors.keys())[i]
            doors[door_name] = 'open'
        elif value_read.value.decode('utf-8') == '0':
            door_name = list(doors.keys())[i]
            doors[door_name] = 'closed'
    
    return jsonify({"doors": doors}), 200

# Ruta para obtener el estado del sensor de movimiento
@app.route('/motion-sensor', methods=['POST'])
@token_required
def get_motion_sensor_status():
    picture = conversephoto("/sensorphoto.jpg")
    return jsonify({"photo": picture}), 200

# Ruta para simular tomar una foto
@app.route('/take-photo', methods=['POST'])
@token_required
def take_photo():
    takePicture(b"camera.jpg")
    picture = conversephoto("/camera.jpg")
    return jsonify({"photo": picture}), 200

if __name__ == '__main__':
    sensor = threading.Thread(target=sensor_state)
    sensor.start()
    app.run(host='0.0.0.0', port=8080)
    stop = False
    sensor.join()
