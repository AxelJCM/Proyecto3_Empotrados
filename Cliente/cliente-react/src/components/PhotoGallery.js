import React, { useState } from 'react';
import './PhotoGallery.css';

function PhotoGallery({ serverUrl }) {
    const [photos, setPhotos] = useState([]);
    const [selectedPhoto, setSelectedPhoto] = useState(null);
    const [filters, setFilters] = useState([]);
    const [filteredPhoto, setFilteredPhoto] = useState(null);

    const handleUpload = (e) => {
        const files = Array.from(e.target.files);
        const newPhotos = files.map((file) => URL.createObjectURL(file));
        setPhotos((prevPhotos) => [...prevPhotos, ...newPhotos]);
    };

    const handlePhotoClick = (photo) => {
        setSelectedPhoto(photo);
    };

    const handleFilterChange = (e) => {
        const selectedFilter = e.target.value;
        if (filters.includes(selectedFilter)) {
            setFilters(filters.filter((f) => f !== selectedFilter));
        } else {
            setFilters([...filters, selectedFilter]);
        }
    };

    const handleApplyFilters = async () => {
        if (!selectedPhoto || filters.length === 0) return;

        const filterString = filters.join('');
        const formData = new FormData();
        formData.append('filter', filterString);

        const response = await fetch(selectedPhoto);
        const blob = await response.blob();
        formData.append('image', blob, 'image.bmp');

        const serverResponse = await fetch(`${serverUrl}/apply-filter`, {
            method: 'POST',
            body: formData,
        });

        if (serverResponse.ok) {
            const filteredBlob = await serverResponse.blob();
            const filteredURL = URL.createObjectURL(filteredBlob);
            setFilteredPhoto(filteredURL);
        } else {
            console.error('Error applying filters on server');
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
                        className={selectedPhoto === photo ? 'selected' : ''}
                    />
                ))}
            </div>
            <div className="filters">
                <h3>Select Filters</h3>
                <div>
                    <label>
                        <input
                            type="checkbox"
                            value="b"
                            onChange={handleFilterChange}
                        />
                        Blur
                    </label>
                    <label>
                        <input
                            type="checkbox"
                            value="g"
                            onChange={handleFilterChange}
                        />
                        Grayscale
                    </label>
                    <label>
                        <input
                            type="checkbox"
                            value="r"
                            onChange={handleFilterChange}
                        />
                        Reflect
                    </label>
                    <label>
                        <input
                            type="checkbox"
                            value="s"
                            onChange={handleFilterChange}
                        />
                        Sepia
                    </label>
                    <label>
                        <input
                            type="checkbox"
                            value="e"
                            onChange={handleFilterChange}
                        />
                        Edges
                    </label>
                    <label>
                        <input
                            type="checkbox"
                            value="p"
                            onChange={handleFilterChange}
                        />
                        Pixelate
                    </label>
                    <label>
                        <input
                            type="checkbox"
                            value="z"
                            onChange={handleFilterChange}
                        />
                        Sharpen
                    </label>
                </div>
                <button onClick={handleApplyFilters}>Apply Filters</button>
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
