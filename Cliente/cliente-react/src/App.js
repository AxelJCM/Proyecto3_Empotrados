import React from 'react';

function App() {
    return (
        <div>
            <h1>Live Video Feed</h1>
            <VideoFeed />
        </div>
    );
}

function VideoFeed() {
    return (
        <div>
            <h2>Video Stream</h2>
            <img
                src="http://192.168.18.47:8080/video-feed"
                alt="Video Feed"
                style={{ width: '640px', height: '480px', border: '2px solid black' }}
            />
        </div>
    );
}

export default App;