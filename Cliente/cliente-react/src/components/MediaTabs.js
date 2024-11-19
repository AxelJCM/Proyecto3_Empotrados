import React, { useState } from 'react';
import VideoStream from './VideoStream';
import PhotoGallery from './PhotoGallery';
import './MediaTabs.css';

function MediaTabs({ serverUrl }) {
    const [activeTab, setActiveTab] = useState('video');

    return (
        <div className="media-tabs">
            <div className="tabs-header">
                <button onClick={() => setActiveTab('video')} className={`tab ${activeTab === 'video' ? 'active' : ''}`}>
                    Video Stream
                </button>
                <button onClick={() => setActiveTab('gallery')} className={`tab ${activeTab === 'gallery' ? 'active' : ''}`}>
                    Photo Gallery
                </button>
            </div>
            <div className="tabs-content">
                {activeTab === 'video' && <VideoStream serverUrl={serverUrl} />}
                {activeTab === 'gallery' && <PhotoGallery serverUrl={serverUrl} />}
            </div>
        </div>
    );
}

export default MediaTabs;

