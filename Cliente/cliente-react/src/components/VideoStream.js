import React, { useState } from 'react';
import './VideoStream.css';

function VideoStream({ serverUrl }) {
    const [streaming, setStreaming] = useState(false);

    const startStream = async () => {
        await fetch(`${serverUrl}/start`, { method: 'POST' });
        setStreaming(true);
    };

    const stopStream = async () => {
        await fetch(`${serverUrl}/stop`, { method: 'POST' });
        setStreaming(false);
    };

    return (
        <div className="video-stream">
            <h3>Video Stream</h3>
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
}

export default VideoStream;
