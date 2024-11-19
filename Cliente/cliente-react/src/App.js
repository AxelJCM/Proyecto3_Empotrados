import React, { useState } from 'react';

const App = () => {
    const [streaming, setStreaming] = useState(false);

    // Server's IP and port
    const serverUrl = 'http://192.168.18.47:8080';

    const startStream = async () => {
        await fetch(`${serverUrl}/start`, { method: 'POST' });
        setStreaming(true);
    };

    const stopStream = async () => {
        await fetch(`${serverUrl}/stop`, { method: 'POST' });
        setStreaming(false);
    };

    return (
        <div>
            <h1>Video Stream</h1>
            <div>
                {streaming ? (
                    <img
                        src={`${serverUrl}/video-stream?ts=${new Date().getTime()}`}
                        alt="Video Stream"
                        style={{ maxWidth: '100%' }}
                    />
                ) : (
                    <p>Stream is stopped.</p>
                )}
            </div>
            <div>
                <button onClick={startStream} disabled={streaming}>
                    Start Stream
                </button>
                <button onClick={stopStream} disabled={!streaming}>
                    Stop Stream
                </button>
            </div>
        </div>
    );
};

export default App;