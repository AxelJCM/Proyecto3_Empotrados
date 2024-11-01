// src/App.js
import React, { useState } from 'react';
import './App.css';
import DigitalFrame from './components/DigitalFrame';
import AuthLogin from './components/AuthLogin';

function App() {
  const [isAuthenticated, setIsAuthenticated] = useState(false);

  const handleLogin = (username, password) => {
    const token = 'mock_token';
    localStorage.setItem('token', token);
    setIsAuthenticated(true);
  };

  const handleLogout = () => {
    localStorage.removeItem('token');
    setIsAuthenticated(false);
  };

  return (
    <div className="App">
      {isAuthenticated ? (
        <DigitalFrame onLogout={handleLogout} />
      ) : (
        <AuthLogin handleLogin={handleLogin} />
      )}
    </div>
  );
}

export default App;
