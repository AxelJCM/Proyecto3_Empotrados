import React, { useState } from 'react';
import VideoFilter from './VideoFilter';
import PhotoGallery from './PhotoGallery';
import CameraCapture from './CameraCapture';
import './MediaTabs.css';

function MediaTabs({ serverUrl }) {
    const [activeTab, setActiveTab] = useState('gallery');

    return (
        <div className="media-tabs">
            <div className="tabs-header">
                <button onClick={() => setActiveTab('gallery')} className={`tab ${activeTab === 'gallery' ? 'active' : ''}`}>
                    Gallery
                </button>
                <button onClick={() => setActiveTab('video')} className={`tab ${activeTab === 'video' ? 'active' : ''}`}>
                    Video
                </button>
            </div>
            <div className="tabs-content">
                {activeTab === 'gallery' && (
                    <div>
                        <CameraCapture />
                        <PhotoGallery serverUrl={serverUrl} />
                    </div>
                )}
                {activeTab === 'video' && <VideoFilter serverUrl={serverUrl} />}
            </div>
        </div>
    );
}

export default MediaTabs;
