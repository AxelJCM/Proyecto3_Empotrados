// src/components/MediaTabs.js
import React, { useState } from 'react';
import VideoFilter from './VideoFilter';
import PhotoGallery from './PhotoGallery';
import CameraCapture from './CameraCapture'; // Importa CameraCapture
import './MediaTabs.css';

function MediaTabs() {
    const [activeTab, setActiveTab] = useState('photoCamera'); // Cambia el valor inicial a 'photoCamera'

    return (
        <div className="media-tabs">
            <button onClick={() => setActiveTab('photoCamera')}>Photo & Camera</button> {/* Un botón para Photo & Camera */}
            <button onClick={() => setActiveTab('video')}>Video</button>
            <div className="tab-content">
                {activeTab === 'photoCamera' && (
                    <div>
                        <CameraCapture /> {/* Incluye CameraCapture */}
                        <PhotoGallery /> {/* Incluye PhotoGallery */}
                    </div>
                )}
                {activeTab === 'video' && <VideoFilter />} {/* Muestra VideoFilter en su pestaña */}
            </div>
        </div>
    );
}

export default MediaTabs;
