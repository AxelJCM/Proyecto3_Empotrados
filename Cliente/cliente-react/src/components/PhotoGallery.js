import React, { useState } from 'react';
import './PhotoGallery.css';

function PhotoGallery({ serverUrl }) {
    const [photos, setPhotos] = useState([]); // Lista de fotos cargadas
    const [selectedPhoto, setSelectedPhoto] = useState(null); // Foto seleccionada para aplicar filtros
    const [filters, setFilters] = useState([]); // Lista de filtros seleccionados
    const [filteredPhoto, setFilteredPhoto] = useState(null); // Foto procesada por el servidor

    // Manejar carga de imágenes desde el usuario
    const handleUpload = (e) => {
        const files = Array.from(e.target.files);
        const newPhotos = files.map((file) => URL.createObjectURL(file));
        setPhotos((prevPhotos) => [...prevPhotos, ...newPhotos]);
    };

    // Seleccionar una foto para aplicar filtros
    const handlePhotoClick = (photo) => {
        setSelectedPhoto(photo);
    };

    // Manejar selección de filtros
    const handleFilterChange = (e) => {
        const selectedFilter = e.target.value;
        if (filters.includes(selectedFilter)) {
            setFilters(filters.filter((f) => f !== selectedFilter));
        } else {
            setFilters([...filters, selectedFilter]);
        }
    };

    // Enviar imagen seleccionada y filtros al servidor
    const handleApplyFilters = async () => {
        if (!selectedPhoto || filters.length === 0) {
            console.error("No se seleccionaron filtros o imágenes.");
            return;
        }

        const filterString = filters.join(''); // Combinar filtros seleccionados en un solo string
        const formData = new FormData();
        formData.append('filter', filterString); // Añadir filtros al FormData

        // Obtener el blob de la imagen seleccionada
        const response = await fetch(selectedPhoto);
        const blob = await response.blob();

        formData.append('image', blob, 'image.bmp'); // Añadir la imagen al FormData

        try {
            const serverResponse = await fetch(`${serverUrl}/apply-filter`, {
                method: 'POST',
                body: formData, // Enviar FormData al servidor
            });

            if (serverResponse.ok) {
                const filteredBlob = await serverResponse.blob();
                const filteredURL = URL.createObjectURL(filteredBlob);
                setFilteredPhoto(filteredURL);
            } else {
                console.error('Error al aplicar filtros en el servidor');
            }
        } catch (error) {
            console.error('Error en la solicitud al servidor:', error);
        }
    };

    return (
        <div className="photo-gallery">
            <h2>Photo Gallery</h2>
            <input type="file" multiple accept="image/*" onChange={handleUpload} />
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
