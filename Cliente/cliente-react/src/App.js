import React, { useState, useEffect, useCallback } from 'react';
import './App.css';
import { HOSTNAME } from './Constant';
import Tabs from './Tabs';  
import Login from './Login'; 

function App() {
  const [isAuthenticated, setIsAuthenticated] = useState(false);
  const [lightStatus, setLightStatus] = useState({});
  const [doorStatus, setDoorStatus] = useState({});
  const [motionSensor, setMotionSensor] = useState(null);
  const [photo, setPhoto] = useState(null);
  const [sensorPhoto, setSensorPhoto] = useState(null);
  const [token, setToken] = useState(localStorage.getItem('token') || '');

  // Función para obtener el estado de luces, puertas y sensor de movimiento
  const fetchStatus = useCallback(async () => {
    try {
      const response = await fetch(`${HOSTNAME}/status`, {
        headers: {
          Authorization: `Bearer ${token}`
        }
      });
      const data = await response.json();
      console.log('Fetched status data:', data);
      setLightStatus(data.lights || {});  
      setDoorStatus(data.doors || {});    
      setMotionSensor(data.motion || 'No motion');
    } catch (error) {
      console.error('Error al obtener el estado:', error);
    }
  }, [token]);

  const fetchDoorStatus = useCallback(async () => {
    try {
      const response = await fetch(`${HOSTNAME}/doors`, {
        method: 'GET',
        headers: {
          'Authorization': `Bearer ${token}`
        }
      });

      const data = await response.json();
      console.log('Door status fetched:', data);  // Depuración: Verificar datos recibidos
        
    } catch (error) {
      console.error('Error al obtener el estado:', error);
    }
  }, [token]);
  
  // Función para obtener la última foto del sensor de movimiento
  const fetchSensorPhoto = useCallback(async () => {
    try {
      const response = await fetch(`${HOSTNAME}/motion-sensor`, {
        method: 'POST',
        headers: {
          Authorization: `Bearer ${token}`
        }
      });
      if (response.ok) {
        const data = await response.json();
        setSensorPhoto(`data:image/jpeg;base64,${data.photo}`);
      }
    } catch (error) {
      console.error('Error al obtener la última foto del sensor:', error);
    }
  }, [token]);

  // Actualiza el estado de las puertas, sensor y la foto del sensor cada intervalo
  useEffect(() => {
    if (token) {
      setIsAuthenticated(true);
      fetchStatus();  // Obtiene el estado inicial de puertas y luces
      fetchSensorPhoto();  // Obtiene la foto inicial del sensor
      fetchDoorStatus();

      // Configura el intervalo para actualizar el estado de puertas y la foto del sensor periódicamente
      const interval = setInterval(() => {
        fetchStatus();  // Actualiza las puertas y luces
        fetchSensorPhoto();  // Actualiza la foto del sensor
        fetchDoorStatus();
      }, 1000);

      return () => clearInterval(interval);  // Limpia el intervalo cuando el componente se desmonta
    }
  }, [token, fetchStatus, fetchSensorPhoto]);

  const handleLogin = async (username, password) => {
    try {
      const response = await fetch(`${HOSTNAME}/login`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({ username, password })
      });
      const data = await response.json();
      if (data.token) {
        localStorage.setItem('token', data.token);
        setToken(data.token);
        setIsAuthenticated(true);
      } else {
        alert('Error en el login');
      }
    } catch (error) {
      console.error('Error en el login:', error);
    }
  };

  const handleLogout = () => {
    localStorage.removeItem('token');
    setToken('');
    setIsAuthenticated(false);
  };

  const handleTakePhoto = async () => {
    try {
      const response = await fetch(`${HOSTNAME}/take-photo`, {
        method: 'POST',
        headers: {
          Authorization: `Bearer ${token}`
        }
      });
      if (response.ok) {
        const data = await response.json();
        setPhoto(`data:image/jpeg;base64,${data.photo}`);
      }
    } catch (error) {
      console.error('Error al tomar la foto:', error);
    }
  };

  const toggleLight = async (room) => {
    const newState = lightStatus[room] === 'on' ? 'off' : 'on';
    setLightStatus({ ...lightStatus, [room]: newState });

    try {
      const response = await fetch(`${HOSTNAME}/lights/${room}`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          Authorization: `Bearer ${token}`
        },
        body: JSON.stringify({ state: newState })
      });
      if (!response.ok && response.status === 401) {
        handleLogout(); 
      }
    } catch (error) {
      console.error('Error al cambiar el estado de la luz:', error);
    }
  };

  if (!isAuthenticated) {
    return <Login handleLogin={handleLogin} />;
  }

  const lightsTab = (
    <section className="tab-content">
      <h2>Luces</h2>
      <div className="lights-grid">
        {Object.keys(lightStatus || {}).map((room) => (
          <div key={room} className="light-card">
            <span>{room.charAt(0).toUpperCase() + room.slice(1)}</span>
            <button
              className={`light-button ${lightStatus[room] === 'on' ? 'on' : 'off'}`}
              onClick={() => toggleLight(room)}
            >
              {lightStatus[room] === 'on' ? '💡 Encendida' : '💤 Apagada'}
            </button>
          </div>
        ))}
      </div>
    </section>
  );

  const doorsTab = (
    <section className="tab-content">
      <h2>Puertas</h2>
      <div className="doors-grid">
        {Object.keys(doorStatus || {}).map((door) => (
          <div key={door} className="door-card">
            <span>{door.charAt(0).toUpperCase() + door.slice(1)}</span>
            <span className="door-icon">
              {doorStatus[door] === 'closed' ? '🔒 Cerrada' : '🔓 Abierta'}
            </span>
          </div>
        ))}
      </div>
    </section>
  );

  const sensorAndCameraTab = (
    <section className="tab-content">
      <h2>Sensor y Cámara</h2>
      <div className="sensor-camera">
        <div className="sensor-section">
          <h3>Sensor de Movimiento</h3>
          <p>{motionSensor}</p>
          {/* Aquí ya no es necesario el botón para el sensor */}
          {sensorPhoto && <img src={sensorPhoto} alt="Última foto del sensor" />}
        </div>
        <div className="camera-section">
          <h3>Cámara</h3>
          <button onClick={handleTakePhoto} className="camera-button">Tomar Foto</button>
          {photo && <img src={photo} alt="Foto de la cámara" />}
        </div>
      </div>
    </section>
  );

  return (
    <div className="house-control">
      <h1>Home Manager</h1>
      <button onClick={handleLogout} className="logout-button">Cerrar sesión</button>
      <Tabs
        tabs={{
          'luces': lightsTab,
          'puertas': doorsTab,
          'sensor y cámara': sensorAndCameraTab
        }}
      />
    </div>
  );
}

export default App;
