import React, { useState, useEffect, useRef } from 'react';
import './PhotoGallery.css';

function PhotoGallery({ serverUrl }) {
    const [photos, setPhotos] = useState([]); // Fotos en el servidor
    const [previewPhotos, setPreviewPhotos] = useState([]); // Fotos cargadas localmente
    const [selectedPhoto, setSelectedPhoto] = useState(null); // Foto seleccionada para filtro
    const [filter, setFilter] = useState(''); // Filtro seleccionado
    const [slideshowActive, setSlideshowActive] = useState(false); // Estado del slideshow
    const [currentSlide, setCurrentSlide] = useState(0); // Índice de la imagen en el slideshow
    const [isFullscreen, setIsFullscreen] = useState(false); // Estado de pantalla completa

    const slideshowRef = useRef(null); // Referencia para el slideshow

    // Cargar imágenes desde el servidor al iniciar
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

    // Manejar la carga de nuevas imágenes para previsualización
    const handleUpload = (e) => {
        const files = Array.from(e.target.files);
        const previews = files.map((file) => ({
            file,
            url: URL.createObjectURL(file),
        }));
        setPreviewPhotos(previews); // Mostrar las imágenes cargadas localmente
    };

    // Aplicar filtro y enviar la imagen al servidor
    const applyFilter = async () => {
        if (!selectedPhoto || !filter) {
            alert('Selecciona una imagen y un filtro antes de aplicar');
            return;
        }

        const formData = new FormData();
        formData.append('image', selectedPhoto.file);
        formData.append('filter', filter);

        try {
            const response = await fetch(`${serverUrl}/apply-filter`, {
                method: 'POST',
                body: formData,
            });

            if (response.ok) {
                const data = await response.json();
                const processedUrl = `${serverUrl}/images/${data.filename}`;

                // Actualizar la galería con la nueva imagen procesada
                setPhotos((prev) => [...prev, processedUrl]);
                setPreviewPhotos([]); // Limpiar las previews locales
                setSelectedPhoto(null); // Limpiar la selección
                setFilter(''); // Limpiar el filtro
            } else {
                console.error('Error al aplicar el filtro');
            }
        } catch (error) {
            console.error('Error al aplicar el filtro:', error);
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

                {/* Controles para filtros */}
                {selectedPhoto && (
                    <div className="filter-controls">
                        <h3>Selecciona un filtro</h3>
                        <select value={filter} onChange={(e) => setFilter(e.target.value)}>
                            <option value="">--Seleccionar filtro--</option>
                            <option value="grayscale">Grayscale</option>
                            <option value="invert">Invert</option>
                        </select>
                        <button onClick={applyFilter}>Aplicar Filtro</button>
                    </div>
                )}

                {/* Galería de imágenes procesadas */}
                <div className="gallery">
                    {photos.length > 0 ? (
                        photos.map((photo, index) => (
                            <img
                                key={index}
                                src={photo}
                                alt={`Imagen ${index}`}
                                className="gallery-image"
                            />
                        ))
                    ) : (
                        <p>No hay imágenes procesadas para mostrar</p>
                    )}
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
        </div>
    );
}

export default PhotoGallery;
