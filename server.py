#!/usr/bin/env python3
# ==============================================================================
# Práctica 4: Servidor HTTP para Generación de Imágenes Táctiles de Alta Resolución
# ==============================================================================

import os
import numpy as np
import matplotlib.pyplot as plt
from flask import Flask, request, jsonify

# Configuración del servidor Flask
app = Flask(__name__)
PORT = 5000
CARPETA_IMAGENES = "tactile_images"

# Asegurar la existencia de la carpeta de almacenamiento para las imágenes PNG
os.makedirs(CARPETA_IMAGENES, exist_ok=True)

print("=========================================================")
print("       INICIANDO SERVIDOR INDUSTRIAL RECEPTOR DE PRESIÓN  ")
print("=========================================================")

@app.route('/upload', methods=['POST'])
def recibir_matriz_tactil():
    """
    Ruta HTTP POST que procesa la matriz de presión de 128x128 recibida en formato JSON,
    valida su estructura y renderiza un mapa térmico en alta resolución (PNG).
    """
    try:
        # 1. Recuperar payload de la petición HTTP
        datos = request.get_json()
        if not datos or 'id' not in datos or 'matrix' not in datos:
            return jsonify({"status": "error", "message": "Formato JSON incompleto."}), 400

        id_captura = datos['id']
        matriz_lista = datos['matrix']
        matriz_np = np.array(matriz_lista)

        # 2. Validación estructural de la matriz escalada (debe ser 128x128)
        if matriz_np.shape != (128, 128):
            print(f"[SERVIDOR ERROR] Captura ID {id_captura} rechazada. Dimensiones {matriz_np.shape} inválidas.")
            return jsonify({"status": "error", "message": "La dimensión de la matriz debe ser de 128x128."}), 400

        # 3. Renderizar el mapa de presión usando Matplotlib (Estilo heatmap industrial)
        plt.figure(figsize=(6, 6))
        
        # 'inferno' o 'viridis' son perfectos para representar mapas de presión táctiles
        plt.imshow(matriz_np, cmap='inferno', interpolation='nearest', vmin=0, vmax=100)
        plt.title(f"Mapa de Presión Gripper - Captura {id_captura:02d}", fontsize=12, fontweight='bold')
        plt.colorbar(label='Presión Relativa (%)')
        plt.axis('off')  # Omitir los ejes para una representación visual limpia de la pinza

        # Guardar en alta definición en la carpeta de destino
        ruta_archivo = os.path.join(CARPETA_IMAGENES, f"capture_{id_captura:02d}.png")
        plt.savefig(ruta_archivo, dpi=120, bbox_inches='tight')
        plt.close()

        print(f"[SERVIDOR] Captura ID {id_captura:02d} recibida con éxito. Imagen guardada.")
        return jsonify({"status": "success", "message": f"Imagen {id_captura:02d} renderizada con éxito."}), 200

    except Exception as e:
        print(f"[SERVIDOR ERROR] Excepción inesperada durante el procesamiento: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    # Arrancar el servidor en la dirección local estándar
    app.run(host='127.0.0.1', port=PORT, debug=False)