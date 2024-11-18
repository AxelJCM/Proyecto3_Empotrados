// src/components/VideoFilter.js
import React, { useEffect, useState, useRef } from 'react';
import './VideoFilter.css';

function VideoFilter() {
    const [filter, setFilter] = useState('none');
    const canvasRef = useRef(null);
    const [intervalId, setIntervalId] = useState(null);
    const [isLoading, setIsLoading] = useState(true);

    // Function to set the filter on the server
    const handleFilterChange = async (e) => {
        const selectedFilter = e.target.value;
        setFilter(selectedFilter);

        try {
            await fetch('http://localhost:5000/set-filter', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ filter: selectedFilter }),
            });
        } catch (error) {
            console.error('Failed to update filter:', error);
        }
    };

    // Function to fetch video frames from the server
    const fetchVideoFrame = async () => {
        try {
            const response = await fetch('http://localhost:5000/video-feed', {
                method: 'GET',
                headers: {
                    'Cache-Control': 'no-cache',
                },
            });

            if (response.ok) {
                const blob = await response.blob();
                const img = new Image();
                img.src = URL.createObjectURL(blob);
                img.onload = () => {
                    setIsLoading(false); // Video feed has started successfully
                    const canvas = canvasRef.current;
                    const ctx = canvas.getContext('2d');
                    ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
                    URL.revokeObjectURL(img.src);
                };
            }
        } catch (error) {
            console.error('Error fetching video frame:', error);
        }
    };

    useEffect(() => {
        // Start fetching frames periodically
        const id = setInterval(fetchVideoFrame, 100);
        setIntervalId(id);

        // Cleanup on unmount
        return () => {
            if (intervalId) {
                clearInterval(intervalId);
            }
        };
    }, []);

    return (
        <div className="video-filter">
            <h3>Apply Filter to Video</h3>
            <select onChange={handleFilterChange} value={filter}>
                <option value="none">None</option>
                <option value="grayscale">Grayscale</option>
                <option value="sepia">Sepia</option>
                <option value="invert">Invert</option>
            </select>
            <div className="video-container" style={{ position: 'relative', width: '100%', height: '500px', marginTop: '20px' }}>
                {isLoading && (
                    <div
                        style={{
                            position: 'absolute',
                            top: '50%',
                            left: '50%',
                            transform: 'translate(-50%, -50%)',
                            color: '#ffffff',
                            fontSize: '20px',
                            zIndex: 1,
                        }}
                    >
                        Loading Video Feed...
                    </div>
                )}
                <canvas
                    ref={canvasRef}
                    width="640"
                    height="480"
                    style={{
                        width: '100%',
                        height: '100%',
                        borderRadius: '8px',
                        backgroundColor: '#333', // Use a clear background color
                    }}
                />
            </div>
        </div>
    );
}

export default VideoFilter;
