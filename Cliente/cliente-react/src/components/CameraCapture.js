// src/components/CameraCapture.js
import React, { useRef, useEffect } from 'react';
import './CameraCapture.css';

function CameraCapture({ onCapture }) { // Agrega onCapture como prop
    const webcamRef = useRef(null);

    useEffect(() => {
        const startWebcam = async () => {
            try {
                const stream = await navigator.mediaDevices.getUserMedia({ video: true });
                webcamRef.current.srcObject = stream;
            } catch (err) {
                console.error("Error accessing webcam: ", err);
            }
        };
        startWebcam();

        return () => {
            if (webcamRef.current && webcamRef.current.srcObject) {
                const tracks = webcamRef.current.srcObject.getTracks();
                tracks.forEach(track => track.stop());
            }
        };
    }, []);

    const capturePhoto = () => {
        const canvas = document.createElement('canvas');
        canvas.width = webcamRef.current.videoWidth;
        canvas.height = webcamRef.current.videoHeight;
        const context = canvas.getContext('2d');
        context.drawImage(webcamRef.current, 0, 0, canvas.width, canvas.height);
        const photoData = canvas.toDataURL('image/jpeg'); // Convierte la imagen a base64
        onCapture(photoData); // Llama a onCapture con la foto capturada
    };

    return (
        <div className="camera-capture">
            <h3>Camera Capture</h3>
            <video ref={webcamRef} autoPlay style={{ width: '100%' }} />
            <button onClick={capturePhoto}>Take Photo</button> {/* Botón para tomar foto */}
        </div>
    );
}

export default CameraCapture;
