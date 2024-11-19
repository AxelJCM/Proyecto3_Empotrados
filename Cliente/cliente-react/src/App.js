import React from 'react';
import MediaTabs from './components/MediaTabs';
import './App.css';

const App = () => {
    const serverUrl = 'http://192.168.18.47:8080'; // Asegúrate de usar la IP correcta del servidor

    return (
        <div>
            <h1>Media Application</h1>
            <MediaTabs serverUrl={serverUrl} />
        </div>
    );
};

export default App;
