// src/components/PhotoGallery.js
import React, { useState } from 'react';
import './PhotoGallery.css';

function PhotoGallery() {
    const [photos, setPhotos] = useState([]);
    const [selectedPhoto, setSelectedPhoto] = useState(null);
    const [filter, setFilter] = useState('none');
    const [filteredPhoto, setFilteredPhoto] = useState(null);

    const handleUpload = (e) => {
        const files = Array.from(e.target.files);
        const newPhotos = files.map(file => URL.createObjectURL(file));
        setPhotos(prevPhotos => [...prevPhotos, ...newPhotos]);
    };

    const handlePhotoClick = (photo) => {
        setSelectedPhoto(photo);
    };

    const handleApplyFilter = async () => {
        if (!selectedPhoto || filter === 'none') return;

        const formData = new FormData();
        formData.append('filter', filter);
        const response = await fetch(selectedPhoto);
        const blob = await response.blob();
        formData.append('image', blob, 'image.jpg');

        const serverResponse = await fetch('http://localhost:5000/apply-filter', {
            method: 'POST',
            body: formData,
        });

        if (serverResponse.ok) {
            const filteredBlob = await serverResponse.blob();
            const filteredURL = URL.createObjectURL(filteredBlob);
            setFilteredPhoto(filteredURL);
        } else {
            console.error('Error applying filter on server');
        }
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
                    />
                ))}
            </div>
            <div className="filters">
                <select onChange={(e) => setFilter(e.target.value)}>
                    <option value="none">None</option>
                    <option value="grayscale">Grayscale</option>
                    <option value="sepia">Sepia</option>
                    <option value="invert">Invert</option>
                </select>
                <button onClick={handleApplyFilter}>Apply Filter</button>
            </div>
            {filteredPhoto && (
                <div className="output">
                    <h3>Filtered Output</h3>
                    <img src={filteredPhoto} alt="Filtered" />
                </div>
            )}
        </div>
    );
}

export default PhotoGallery;
