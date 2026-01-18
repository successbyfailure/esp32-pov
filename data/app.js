// Estado global
let selectedImage = '';
let currentEffect = 'none';

// Inicialización
document.addEventListener('DOMContentLoaded', () => {
    loadStatus();
    loadImages();
    loadConfig();

    // Actualizar estado cada 2 segundos
    setInterval(loadStatus, 2000);
});

// Cargar estado del sistema
async function loadStatus() {
    try {
        const response = await fetch('/api/status');
        const data = await response.json();

        document.getElementById('pov-state').textContent = data.state;
        document.getElementById('current-image').textContent = data.image || 'Ninguna';
        document.getElementById('current-column').textContent = data.column || 0;
        document.getElementById('total-columns').textContent = data.totalColumns || 0;
        document.getElementById('speed-value').textContent = data.speed;
        document.getElementById('speed-slider').value = data.speed;
        document.getElementById('brightness-value').textContent = data.brightness;
        document.getElementById('brightness-slider').value = data.brightness;
        document.getElementById('loop-checkbox').checked = data.loopMode;
        document.getElementById('orientation-select').value = data.orientation;
        document.getElementById('wifi-ssid').textContent = data.wifiSSID || 'No conectado';
        document.getElementById('ip').textContent = data.wifiIP || '-';
        document.getElementById('space').textContent = Math.round(data.freeSpace / 1024);

    } catch (error) {
        console.error('Error cargando estado:', error);
    }
}

// Cargar lista de imágenes
async function loadImages() {
    try {
        const response = await fetch('/api/images');
        const data = await response.json();

        const container = document.getElementById('images-list');

        if (data.images.length === 0) {
            container.innerHTML = '<p class="loading">No hay imágenes. Sube una imagen para comenzar.</p>';
            return;
        }

        container.innerHTML = '';

        data.images.forEach(img => {
            const card = document.createElement('div');
            card.className = 'image-card';

            card.innerHTML = `
                <h3>${img.name}</h3>
                <div class="image-info">Dimensiones: ${img.width}x${img.height}</div>
                <div class="image-info">Tamaño: ${Math.round(img.size / 1024)} KB</div>
                <div class="image-info">Formato: ${img.format}</div>
                <div class="image-actions">
                    <button class="btn btn-primary" onclick="selectImage('${img.name}')">Seleccionar</button>
                    <button class="btn btn-danger" onclick="deleteImage('${img.name}')">Eliminar</button>
                </div>
            `;

            container.appendChild(card);
        });

    } catch (error) {
        console.error('Error cargando imágenes:', error);
    }
}

// Cargar configuración
async function loadConfig() {
    try {
        const response = await fetch('/api/config');
        const data = await response.json();

        document.getElementById('device-name').value = data.deviceName || '';
        document.getElementById('num-leds').value = data.numLeds || 144;
        document.getElementById('wifi-ssid-input').value = data.wifiSSID || '';
        document.getElementById('mqtt-enabled').checked = data.mqttEnabled || false;
        document.getElementById('mqtt-broker').value = data.mqttBroker || '';
        document.getElementById('mqtt-port').value = data.mqttPort || 1883;

    } catch (error) {
        console.error('Error cargando configuración:', error);
    }
}

// Seleccionar imagen
function selectImage(imageName) {
    selectedImage = imageName;
    alert('Imagen seleccionada: ' + imageName + '\nHaz clic en Play para iniciar el efecto POV');
}

// Play POV
async function playPOV() {
    if (!selectedImage) {
        alert('Por favor, selecciona una imagen primero');
        return;
    }

    try {
        const formData = new FormData();
        formData.append('image', selectedImage);

        const response = await fetch('/api/play', {
            method: 'POST',
            body: formData
        });

        const data = await response.json();

        if (data.success) {
            console.log('POV iniciado');
        } else {
            alert('Error al iniciar POV: ' + data.error);
        }

    } catch (error) {
        console.error('Error:', error);
        alert('Error al iniciar POV');
    }
}

// Pausar POV
async function pausePOV() {
    try {
        const response = await fetch('/api/pause', { method: 'POST' });
        const data = await response.json();
        console.log('POV pausado');
    } catch (error) {
        console.error('Error:', error);
    }
}

// Detener POV
async function stopPOV() {
    try {
        const response = await fetch('/api/stop', { method: 'POST' });
        const data = await response.json();
        console.log('POV detenido');
    } catch (error) {
        console.error('Error:', error);
    }
}

// Actualizar velocidad
async function updateSpeed(value) {
    document.getElementById('speed-value').textContent = value;

    try {
        const formData = new FormData();
        formData.append('speed', value);

        await fetch('/api/settings', {
            method: 'POST',
            body: formData
        });
    } catch (error) {
        console.error('Error:', error);
    }
}

// Actualizar brillo
async function updateBrightness(value) {
    document.getElementById('brightness-value').textContent = value;

    try {
        const formData = new FormData();
        formData.append('brightness', value);

        await fetch('/api/settings', {
            method: 'POST',
            body: formData
        });
    } catch (error) {
        console.error('Error:', error);
    }
}

// Actualizar modo loop
async function updateLoop(checked) {
    try {
        const formData = new FormData();
        formData.append('loop', checked ? 'true' : 'false');

        await fetch('/api/settings', {
            method: 'POST',
            body: formData
        });
    } catch (error) {
        console.error('Error:', error);
    }
}

// Actualizar orientación
async function updateOrientation(value) {
    try {
        const formData = new FormData();
        formData.append('orientation', value);

        await fetch('/api/settings', {
            method: 'POST',
            body: formData
        });

        console.log('Orientación actualizada a:', value);
    } catch (error) {
        console.error('Error:', error);
    }
}

// Eliminar imagen
async function deleteImage(imageName) {
    if (!confirm('¿Estás seguro de que quieres eliminar ' + imageName + '?')) {
        return;
    }

    try {
        const formData = new FormData();
        formData.append('image', imageName);

        const response = await fetch('/api/image/delete', {
            method: 'POST',
            body: formData
        });

        const data = await response.json();

        if (data.success) {
            alert('Imagen eliminada correctamente');
            loadImages();
        } else {
            alert('Error al eliminar imagen: ' + data.error);
        }

    } catch (error) {
        console.error('Error:', error);
        alert('Error al eliminar imagen');
    }
}

// Manejo de upload
function handleFileSelect(event) {
    const file = event.target.files[0];
    if (file) {
        uploadFile(file);
    }
}

function handleDragOver(event) {
    event.preventDefault();
    event.stopPropagation();
}

function handleDrop(event) {
    event.preventDefault();
    event.stopPropagation();

    const file = event.dataTransfer.files[0];
    if (file) {
        uploadFile(file);
    }
}

async function uploadFile(file) {
    // Validar tamaño
    if (file.size > 100 * 1024) {
        alert('El archivo es muy grande. Tamaño máximo: 100 KB');
        return;
    }

    // Validar extensión
    const validExtensions = ['.bmp', '.rgb', '.565'];
    const fileName = file.name.toLowerCase();
    const isValid = validExtensions.some(ext => fileName.endsWith(ext));

    if (!isValid) {
        alert('Formato de archivo no soportado. Usa BMP, RGB o 565');
        return;
    }

    const formData = new FormData();
    formData.append('file', file);

    const progressDiv = document.getElementById('upload-progress');
    const progressFill = document.getElementById('progress-fill');
    const statusText = document.getElementById('upload-status');

    progressDiv.style.display = 'block';
    progressFill.style.width = '0%';
    statusText.textContent = 'Subiendo...';

    try {
        const response = await fetch('/api/upload', {
            method: 'POST',
            body: formData
        });

        progressFill.style.width = '100%';

        const data = await response.json();

        if (data.success) {
            statusText.textContent = 'Archivo subido correctamente';
            setTimeout(() => {
                progressDiv.style.display = 'none';
                loadImages();
            }, 2000);
        } else {
            statusText.textContent = 'Error al subir archivo';
        }

    } catch (error) {
        console.error('Error:', error);
        statusText.textContent = 'Error al subir archivo';
    }
}

// Efectos
async function setEffect(effectName) {
    currentEffect = effectName;

    const formData = new FormData();
    formData.append('effect', effectName);

    // Si es color sólido o chase, incluir color
    if (effectName === 'solid' || effectName === 'chase') {
        const colorPicker = document.getElementById('color-picker');
        const color = colorPicker.value;

        // Convertir hex a RGB
        const r = parseInt(color.substr(1, 2), 16);
        const g = parseInt(color.substr(3, 2), 16);
        const b = parseInt(color.substr(5, 2), 16);

        formData.append('r', r);
        formData.append('g', g);
        formData.append('b', b);

        document.getElementById('color-picker-container').style.display = 'block';
    } else {
        document.getElementById('color-picker-container').style.display = 'none';
    }

    try {
        const response = await fetch('/api/effect', {
            method: 'POST',
            body: formData
        });

        const data = await response.json();
        console.log('Efecto aplicado:', effectName);

    } catch (error) {
        console.error('Error:', error);
    }
}

// Guardar configuración
async function saveConfig() {
    const formData = new FormData();
    formData.append('deviceName', document.getElementById('device-name').value);
    formData.append('numLeds', document.getElementById('num-leds').value);
    formData.append('wifiSSID', document.getElementById('wifi-ssid-input').value);
    formData.append('wifiPassword', document.getElementById('wifi-pass').value);
    formData.append('mqttEnabled', document.getElementById('mqtt-enabled').checked);
    formData.append('mqttBroker', document.getElementById('mqtt-broker').value);
    formData.append('mqttPort', document.getElementById('mqtt-port').value);

    try {
        const response = await fetch('/api/config', {
            method: 'POST',
            body: formData
        });

        const data = await response.json();

        if (data.success) {
            alert('Configuración guardada. El dispositivo se reiniciará.');
        } else {
            alert('Error al guardar configuración');
        }

    } catch (error) {
        console.error('Error:', error);
        alert('Error al guardar configuración');
    }
}
