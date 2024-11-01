// src/components/VideoFilter.js
import React, { useEffect, useRef, useState } from 'react';
import './VideoFilter.css';

function VideoFilter() {
    const videoRef = useRef(null);
    const [filter, setFilter] = useState('none');

    useEffect(() => {
        const startVideo = async () => {
            try {
                const stream = await navigator.mediaDevices.getUserMedia({ video: true });
                videoRef.current.srcObject = stream;
            } catch (err) {
                console.error("Error accessing webcam: ", err);
            }
        };
        startVideo();
    }, []);

    return (
        <div className="video-filter">
            <h3>Apply Filter</h3>
            <select onChange={(e) => setFilter(e.target.value)}>
                <option value="none">None</option>
                <option value="grayscale(100%)">Grayscale</option>
                <option value="sepia(100%)">Sepia</option>
                <option value="invert(100%)">Invert</option>
            </select>
            <video ref={videoRef} style={{ filter }} autoPlay />
        </div>
    );
}

export default VideoFilter;
