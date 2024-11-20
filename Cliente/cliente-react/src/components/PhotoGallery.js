import React, { useState, useEffect, useRef } from 'react';
import './PhotoGallery.css';

function PhotoGallery({ serverUrl }) {
    const [photos, setPhotos] = useState([]); // Fotos cargadas desde el servidor
    const [previewPhotos, setPreviewPhotos] = useState([]); // Previsualizaciones locales
    const [selectedPhoto, setSelectedPhoto] = useState(null); // Foto seleccionada para aplicar filtros
    const [filters, setFilters] = useState([]); // Lista de filtros seleccionados
    const [filteredPhoto, setFilteredPhoto] = useState(null); // Foto procesada
    const [slideshowActive, setSlideshowActive] = useState(false); // Estado del slideshow
    const [currentSlide, setCurrentSlide] = useState(0); // Índice de la imagen en el slideshow
    const [isFullscreen, setIsFullscreen] = useState(false); // Estado de pantalla completa

    const slideshowRef = useRef(null); // Referencia para el slideshow

    // Cargar imágenes desde el servidor
    useEffect(() => {
        const fetchPhotos = async () => {
            try {
                const response = await fetch(`${serverUrl}/list-images`);
                if (response.ok) {
                    const imageList = await response.json();
                    setPhotos(imageList.map((filename) => `${serverUrl}/images/${filename}`));
                } else {
                    console.error('Error al obtener las imágenes del servidor');
                }
            } catch (error) {
                console.error('Error al conectar con el servidor:', error);
            }
        };
        fetchPhotos();
    }, [serverUrl]);

    // Manejar carga de imágenes desde el cliente
    const handleUpload = (e) => {
        const files = Array.from(e.target.files);
        const previews = files.map((file) => ({
            file,
            url: URL.createObjectURL(file),
        }));
        setPreviewPhotos(previews);
    };

    // Seleccionar una foto para aplicar filtros
    const handlePhotoClick = (photo) => {
        setSelectedPhoto(photo);
    };

    // Manejar la selección de filtros
    const handleFilterChange = (e) => {
        const selectedFilter = e.target.value;
        if (filters.includes(selectedFilter)) {
            setFilters(filters.filter((f) => f !== selectedFilter));
        } else {
            setFilters([...filters, selectedFilter]);
        }
    };

    // Enviar imagen seleccionada con múltiples filtros al servidor
    const handleApplyFilters = async () => {
        if (!selectedPhoto || filters.length === 0) {
            alert('Selecciona una imagen y al menos un filtro antes de aplicar');
            return;
        }

        const filterString = filters.join(','); // Convertir filtros en un string separado por comas
        const formData = new FormData();

        if (selectedPhoto.file) {
            // Imagen desde previsualización local
            formData.append('image', selectedPhoto.file);
        } else {
            // Imagen desde el servidor
            const response = await fetch(selectedPhoto);
            const blob = await response.blob();
            formData.append('image', blob, 'uploaded_image.jpg');
        }

        formData.append('filters', filterString); // Añadir filtros al FormData

        try {
            const response = await fetch(`${serverUrl}/apply-filters`, {
                method: 'POST',
                body: formData,
            });

            if (response.ok) {
                const filteredBlob = await response.blob();
                const filteredURL = URL.createObjectURL(filteredBlob);
                setFilteredPhoto(filteredURL); // Mostrar la imagen procesada
            } else {
                console.error('Error al aplicar filtros en el servidor');
            }
        } catch (error) {
            console.error('Error al conectar con el servidor:', error);
        }
    };

    // Manejar el inicio del slideshow
    const startSlideshow = () => {
        if (photos.length === 0) {
            alert('No hay imágenes para mostrar en el slideshow.');
            return;
        }
        setSlideshowActive(true);
        setCurrentSlide(0);
    };

    // Manejar el ciclo del slideshow
    useEffect(() => {
        let interval;
        if (slideshowActive) {
            interval = setInterval(() => {
                setCurrentSlide((prevSlide) => (prevSlide + 1) % photos.length);
            }, 2000); // Cambiar de imagen cada 2 segundos
        }
        return () => clearInterval(interval); // Limpiar intervalo al detener el slideshow
    }, [slideshowActive, photos]);

    // Detener el slideshow
    const stopSlideshow = () => {
        setSlideshowActive(false);
    };

    // Activar fullscreen para el slideshow
    const toggleFullscreen = () => {
        if (slideshowRef.current) {
            if (!document.fullscreenElement) {
                slideshowRef.current.requestFullscreen().catch((err) => {
                    console.error(`Error al entrar en fullscreen: ${err.message}`);
                });
                setIsFullscreen(true);
            } else {
                document.exitFullscreen().catch((err) => {
                    console.error(`Error al salir de fullscreen: ${err.message}`);
                });
                setIsFullscreen(false);
            }
        }
    };

    return (
        <div className="photo-gallery-container">
            {/* Panel de galería */}
            <div className="gallery-panel">
                <h2>Galería de Imágenes</h2>

                {/* Input para cargar imágenes */}
                <input type="file" multiple accept="image/*" onChange={handleUpload} />

                {/* Mostrar previews locales */}
                {previewPhotos.length > 0 && (
                    <div className="preview-gallery">
                        <h3>Previsualización</h3>
                        {previewPhotos.map((photo, index) => (
                            <img
                                key={index}
                                src={photo.url}
                                alt={`Preview ${index}`}
                                onClick={() => setSelectedPhoto(photo)}
                                className={selectedPhoto === photo ? 'selected' : ''}
                            />
                        ))}
                    </div>
                )}

                {/* Seleccionar filtros */}
                <div className="filters">
                    <h3>Seleccionar Filtros</h3>
                    <div>
                        <label>
                            <input type="checkbox" value="-b" onChange={handleFilterChange} />
                            Blur
                        </label>
                        <label>
                            <input type="checkbox" value="-g" onChange={handleFilterChange} />
                            Grayscale
                        </label>
                        <label>
                            <input type="checkbox" value="-r" onChange={handleFilterChange} />
                            Reflect
                        </label>
                        <label>
                            <input type="checkbox" value="-s" onChange={handleFilterChange} />
                            Sepia
                        </label>
                        <label>
                            <input type="checkbox" value="-e" onChange={handleFilterChange} />
                            Edges
                        </label>
                        <label>
                            <input type="checkbox" value="-p" onChange={handleFilterChange} />
                            Pixelate
                        </label>
                        <label>
                            <input type="checkbox" value="-z" onChange={handleFilterChange} />
                            Sharpen
                        </label>
                    </div>
                    <button onClick={handleApplyFilters}>Aplicar Filtros</button>
                </div>

                {/* Galería de imágenes procesadas */}
                <div className="gallery">
                    {photos.map((photo, index) => (
                        <img
                            key={index}
                            src={photo}
                            alt={`Imagen ${index}`}
                            className="gallery-image"
                        />
                    ))}
                </div>
            </div>

            {/* Panel de slideshow */}
            <div className="slideshow-panel" ref={slideshowRef}>
                <h2>Slideshow</h2>
                <div className="slideshow-controls">
                    <button onClick={startSlideshow} disabled={slideshowActive}>Iniciar Slideshow</button>
                    <button onClick={stopSlideshow} disabled={!slideshowActive}>Detener Slideshow</button>
                    <button onClick={toggleFullscreen}>Pantalla Completa</button>
                </div>

                {slideshowActive && photos.length > 0 && (
                    <div className="slideshow">
                        <img
                            src={photos[currentSlide]}
                            alt={`Slideshow ${currentSlide}`}
                            className={`slideshow-image ${isFullscreen ? 'fullscreen' : ''}`}
                        />
                    </div>
                )}
            </div>

            {/* Imagen procesada */}
            {filteredPhoto && (
                <div className="output">
                    <h3>Imagen Procesada</h3>
                    <img src={filteredPhoto} alt="Filtered" />
                </div>
            )}
        </div>
    );
}

export default PhotoGallery;
