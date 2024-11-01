// src/components/DigitalFrame.js
import React, { useState, useEffect, useCallback } from 'react';
import '../App.css';
import { HOSTNAME } from '../Constant';

function DigitalFrame({ onLogout }) {
  const [photo, setPhoto] = useState(null);
  const [videos, setVideos] = useState([]);
  const [activeFilter, setActiveFilter] = useState('none');
  const [activeTab, setActiveTab] = useState('media');
  const token = localStorage.getItem('token');

  const fetchMedia = useCallback(async () => {
    try {
      const response = await fetch(`${HOSTNAME}/media`, {
        headers: { Authorization: `Bearer ${token}` }
      });
      const data = await response.json();
      setPhoto(data.photo);
      setVideos(data.videos);
    } catch (error) {
      console.error('Error al obtener el contenido multimedia:', error);
    }
  }, [token]);

  const applyFilter = async (filter) => {
    try {
      const response = await fetch(`${HOSTNAME}/apply-filter`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          Authorization: `Bearer ${token}`
        },
        body: JSON.stringify({ filter })
      });
      if (response.ok) setActiveFilter(filter);
    } catch (error) {
      console.error('Error al aplicar el filtro:', error);
    }
  };

  useEffect(() => {
    if (token) fetchMedia();
  }, [token, fetchMedia]);

  const handleTabChange = (tab) => setActiveTab(tab);

  return (
    <div className="digital-frame">
      <h2>Marco Digital Interactivo</h2>
      <button onClick={onLogout}>Cerrar sesión</button>

      <div className="tabs">
        {['media', 'filters'].map((tab) => (
          <button
            key={tab}
            onClick={() => handleTabChange(tab)}
            className={activeTab === tab ? 'active' : ''}
          >
            {tab.charAt(0).toUpperCase() + tab.slice(1)}
          </button>
        ))}
      </div>

      {activeTab === 'media' && (
        <div className="media-tab">
          <h3>Fotos y Videos</h3>
          {photo && <img src={`data:image/jpeg;base64,${photo}`} alt="Foto" />}
          <div className="videos-list">
            {videos.map((video, index) => (
              <video key={index} src={video} controls />
            ))}
          </div>
        </div>
      )}

      {activeTab === 'filters' && (
        <div className="filters-tab">
          <h3>Filtros</h3>
          {['None', 'GrayScale', 'Sepia', 'Invert'].map((filter) => (
            <button
              key={filter}
              onClick={() => applyFilter(filter)}
              className={`filter-button ${activeFilter === filter ? 'active' : ''}`}
            >
              {filter}
            </button>
          ))}
        </div>
      )}
    </div>
  );
}

export default DigitalFrame;
