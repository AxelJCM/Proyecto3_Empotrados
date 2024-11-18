// src/components/VideoFilter.js
import React, { useState } from 'react';
import './VideoFilter.css';

function VideoFilter() {
    const [filter, setFilter] = useState('none');

    // Function to set the filter on the server
    const handleFilterChange = async (e) => {
        const selectedFilter = e.target.value;
        setFilter(selectedFilter);

        try {
            await fetch('http://localhost:5000/set-filter', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ filter: selectedFilter })
            });
        } catch (error) {
            console.error("Failed to update filter:", error);
        }
    };

    return (
        <div className="video-filter">
            <h3>Apply Filter to Video</h3>
            <select onChange={handleFilterChange} value={filter}>
                <option value="none">None</option>
                <option value="grayscale">Grayscale</option>
                <option value="sepia">Sepia</option>
                <option value="invert">Invert</option>
            </select>
            <div className="video-container">
                <img
                    src="http://localhost:5000/video-feed"
                    alt="Video feed from server"
                    style={{ width: '100%', borderRadius: '8px' }}
                />
            </div>
        </div>
    );
}

export default VideoFilter;
