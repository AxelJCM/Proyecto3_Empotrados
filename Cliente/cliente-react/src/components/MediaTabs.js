// src/components/MediaTabs.js
import React, { useState } from 'react';
import VideoFilter from './VideoFilter';
import './MediaTabs.css';

function MediaTabs() {
    const [activeTab, setActiveTab] = useState('video');

    return (
        <div className="media-tabs">
            <button onClick={() => setActiveTab('photo')}>Photo</button>
            <button onClick={() => setActiveTab('video')}>Video</button>
            <div className="tab-content">
                {activeTab === 'photo' ? <h2>Photo Display (WIP)</h2> : <VideoFilter />}
            </div>
        </div>
    );
}

export default MediaTabs;

