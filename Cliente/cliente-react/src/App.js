import React from 'react';
import MediaTabs from './components/MediaTabs';
import './App.css';

const App = () => {
    const serverUrl = 'http://192.168.18.47:5000'; // Gallery
    const serverUrl2 = 'http://192.168.18.47:8080'; // Stream

    return (
        <div>
            <h1>Media Application</h1>
            <MediaTabs serverUrl={serverUrl} serverUrl2={serverUrl2}/>
        </div>
    );
};

export default App;
