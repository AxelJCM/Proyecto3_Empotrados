import React from 'react';
import MediaTabs from './components/MediaTabs';
import './App.css';

const App = () => {
    const serverUrl = 'http://192.168.18.47:8080'; // Reemplaza con la IP correcta

    return (
        <div>
            <h1>Media Application</h1>
            <MediaTabs serverUrl={serverUrl} />
        </div>
    );
};

export default App;
