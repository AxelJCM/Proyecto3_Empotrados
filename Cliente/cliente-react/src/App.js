// src/App.js
import React, { useState } from 'react';
import Auth from './components/Auth';
import MediaTabs from './components/MediaTabs';
import './App.css';

function App() {
    const [token, setToken] = useState(null);

    const handleLogin = (newToken) => {
        setToken(newToken);
        console.log("Logged in with token:", newToken);
        // Store the token in localStorage or state
    };

    const handleLogout = () => {
        setToken(null);
        console.log("Logged out");
        // Remove token from localStorage if needed
    };

    return (
        <div className="App">
            {!token ? (
                <Auth onLogin={handleLogin} />
            ) : (
                <div>
                    <button onClick={handleLogout} className="logout-button">Logout</button>
                    <MediaTabs />
                </div>
            )}
        </div>
    );
}

export default App;
