// src/components/PhotoGallery.js
import React, { useState } from 'react';
import './PhotoGallery.css';

function PhotoGallery() {
    const [photos, setPhotos] = useState([]);
    const [selectedPhoto, setSelectedPhoto] = useState(null);
    const [filter, setFilter] = useState('none');

    const handleUpload = (e) => {
        const files = Array.from(e.target.files);
        const newPhotos = files.map(file => URL.createObjectURL(file));
        setPhotos(prevPhotos => [...prevPhotos, ...newPhotos]);
    };

    const handlePhotoClick = (photo) => {
        setSelectedPhoto(photo);
    };

    return (
        <div className="photo-gallery">
            <input type="file" multiple accept="image/*" onChange={handleUpload} />
            <h2>Gallery</h2>
            <div className="carousel">
                {photos.map((photo, index) => (
                    <img
                        key={index}
                        src={photo}
                        alt={`uploaded ${index}`}
                        onClick={() => handlePhotoClick(photo)}
                        style={{ filter: selectedPhoto === photo ? filter : 'none' }}
                    />
                ))}
            </div>
            <div className="filters">
                <select onChange={(e) => setFilter(e.target.value)}>
                    <option value="none">None</option>
                    <option value="grayscale(100%)">Grayscale</option>
                    <option value="sepia(100%)">Sepia</option>
                    <option value="invert(100%)">Invert</option>
                </select>
            </div>
            {selectedPhoto && (
                <div className="output">
                    <h3>Filtered Output</h3>
                    <img src={selectedPhoto} alt="Selected" style={{ filter }} />
                </div>
            )}
        </div>
    );
}

export default PhotoGallery;
