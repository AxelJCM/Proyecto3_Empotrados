import React, { useState, useRef, useEffect } from 'react';
import './VideoFilter.css';

function VideoFilter({ serverUrl }) {
    const [filter, setFilter] = useState('none');
    const canvasRef = useRef(null);

    const handleFilterChange = async (e) => {
        const selectedFilter = e.target.value;
        setFilter(selectedFilter);

        await fetch(`${serverUrl}/set-filter`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ filter: selectedFilter }),
        });
    };

    useEffect(() => {
        const fetchVideoFrame = async () => {
            const response = await fetch(`${serverUrl}/video-feed`);
            const blob = await response.blob();
            const img = new Image();
            img.src = URL.createObjectURL(blob);
            img.onload = () => {
                const canvas = canvasRef.current;
                const ctx = canvas.getContext('2d');
                ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
            };
        };

        const interval = setInterval(fetchVideoFrame, 100);
        return () => clearInterval(interval);
    }, [serverUrl]);

    return (
        <div className="video-filter">
            <h3>Apply Filter to Video</h3>
            <select onChange={handleFilterChange} value={filter}>
                <option value="none">None</option>
                <option value="grayscale">Grayscale</option>
                <option value="sepia">Sepia</option>
                <option value="invert">Invert</option>
            </select>
            <canvas ref={canvasRef} width="640" height="480" />
        </div>
    );
}

export default VideoFilter;
