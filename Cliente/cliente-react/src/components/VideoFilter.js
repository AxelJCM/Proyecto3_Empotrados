import React, { useEffect, useState, useRef } from 'react';
import './VideoFilter.css';

function VideoFilter({ serverUrl }) {
    const [filters, setFilters] = useState([]);
    const canvasRef = useRef(null);

    const handleFilterChange = (e) => {
        const selectedFilter = e.target.value;
        if (filters.includes(selectedFilter)) {
            setFilters(filters.filter((f) => f !== selectedFilter));
        } else {
            setFilters([...filters, selectedFilter]);
        }

        // Enviar los filtros seleccionados al servidor
        fetch(`${serverUrl}/set-filter`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ filter: filters.join('') }),
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
                URL.revokeObjectURL(img.src);
            };
        };

        const interval = setInterval(fetchVideoFrame, 100);
        return () => clearInterval(interval);
    }, [serverUrl]);

    return (
        <div className="video-filter">
            <h3>Apply Filters to Video Feed</h3>
            <div className="filters">
                <label>
                    <input type="checkbox" value="b" onChange={handleFilterChange} />
                    Blur
                </label>
                <label>
                    <input type="checkbox" value="g" onChange={handleFilterChange} />
                    Grayscale
                </label>
                <label>
                    <input type="checkbox" value="r" onChange={handleFilterChange} />
                    Reflect
                </label>
                <label>
                    <input type="checkbox" value="s" onChange={handleFilterChange} />
                    Sepia
                </label>
                <label>
                    <input type="checkbox" value="e" onChange={handleFilterChange} />
                    Edges
                </label>
                <label>
                    <input type="checkbox" value="p" onChange={handleFilterChange} />
                    Pixelate
                </label>
                <label>
                    <input type="checkbox" value="z" onChange={handleFilterChange} />
                    Sharpen
                </label>
            </div>
            <canvas ref={canvasRef} width="640" height="480" style={{ width: '100%', height: '100%' }} />
        </div>
    );
}

export default VideoFilter;
