# Práctica 4: Procesamiento e Interpolación de Imágenes Táctiles para un Gripper Robótico en C++ y Python

Este repositorio contiene la implementación completa del sistema de alta resolución para un sensor táctil matricial de $16\times16$ instalado en el gripper de un robot industrial, desarrollado para la asignatura de Sistemas Operativos del Grado en Tecnologías Digitales para la Empresa (Curso 2025-2026) en la Universidad de Burgos.

# Descripción del Proyecto

En entornos industriales de robótica de precisión, los sensores táctiles son clave para interactuar con piezas frágiles o irregulares. Sin embargo, los sensores de bajo coste ofrecen bajas resoluciones de lectura (como rejillas de $16\times16$ sensores de presión).

Esta solución aplica computación matemática distribuida e ingeniería de software para superar esta barrera de hardware:

Mapeo y Carga de Datos (C++): Lee de manera persistente y valida el archivo tactile_captures_50.json que contiene 50 capturas de presión del gripper de 16x16 sensores.

Algoritmo de Interpolación Bilineal (C++): Escala cada matriz de baja resolución a una matriz interpolada matemática de $128\times128$ puntos utilizando fórmulas de ponderación espacial de cuatro vecinos, incrementando el nivel de detalle de manera sustancial.

Transmisión HTTP y Sockets de Capa de Aplicación: Mediante peticiones concurrentes de red, el programa C++ serializa los datos con la librería nlohmann/json y los envía usando la llamada HTTP POST a través de la librería dinámica libcurl a un servidor local.

Generación de Mapas Térmicos Visuales (Python): Un servidor ligero implementado en Flask recibe cada matriz de presión y, mediante las librerías científicas NumPy y Matplotlib, renderiza mapas de calor estilizados (inferno) que salvan la presión táctil del gripper en formato de imagen de alta definición PNG (plots/).

Control y Monitorización (Bash): Orquesta la inicialización e instalación de dependencias, gestiona la asignación de puertos, lanza los hilos del servidor en segundo plano (&), evalúa de manera asíncrona mediante señales (kill -0) y destruye los procesos del Kernel al finalizar la tarea para asegurar la estabilidad física del equipo.

# Requisitos Previos (Prerrequisitos)

El entorno requiere la instalación de compiladores, utilidades de red y herramientas de análisis científico. Para instalar todas las librerías en tu entorno Linux (Ubuntu/WSL), puedes ejecutar directamente nuestro script de automatización en Bash o usar manualmente este comando:

sudo apt update && sudo apt install -y g++ libcurl4-openssl-dev nlohmann-json3-dev python3-pip python3-flask python3-matplotlib python3-numpy


# Instrucciones de Ejecución

Opción Automatizada (Recomendada)

Para asegurar el correcto orden de arranque del cliente-servidor y liberar puertos automáticamente, ejecuta el script en Bash:

Concede permisos de ejecución al automatizador:

chmod +x run_practica4.sh


Lanza el procesamiento completo de las 50 imágenes:

bash run_practica4.sh


El script instalará las dependencias necesarias de C++ y Python, compilará el código C++ con el estándar moderno C++17, levantará el servidor Flask en segundo plano, procesará las 50 capturas mostrándolas en tiempo real y detendrá el servidor web de forma segura.

# Opción Manual Paso a Paso

En una terminal de Linux, inicia la parte del servidor gráfico:

python3 server.py


Abre otra terminal en paralelo, compila y arranca el procesador de imágenes en C++:

g++ -std=c++17 main.cpp -lcurl -o main
./main


# Salida Esperada en Pantalla

Tras lanzar el script bash run_practica4.sh, verás la comunicación interactiva de red en tiempo real:

=====================================================================
       INICIANDO INFRAESTRUCTURA DE PROCESAMIENTO TÁCTIL (UBU)       
=====================================================================
[Bash 1/6] Verificando e instalando dependencias requeridas...
[Bash 2/6] Compilando el cliente C++ con estándar C++17 y librerías dinámicas...
[OK] Compilación completada con éxito.
[Bash 3/6] Iniciando el Servidor Python en segundo plano...
[INFO] Servidor Flask en background con PID: 21305
=========================================================
       INICIANDO SERVIDOR INDUSTRIAL RECEPTOR DE PRESIÓN  
=========================================================
 * Serving Flask app 'server'
[Bash 4/6] Ejecutando procesamiento en C++...
[C++] Iniciando el Procesador de Imágenes Táctiles...
[C++] Archivo JSON cargado. Detectadas 50 capturas táctiles.
[C++] Procesando captura ID: 0 [16x16 -> 128x128]. Enviando...
[SERVIDOR] Captura ID 00 recibida con éxito. Imagen guardada.
[C++] Procesando captura ID: 1 [16x16 -> 128x128]. Enviando...
[SERVIDOR] Captura ID 01 recibida con éxito. Imagen guardada.
...
[C++] Procesando captura ID: 49 [16x16 -> 128x128]. Enviando...
[SERVIDOR] Captura ID 49 recibida con éxito. Imagen guardada.

========================================================
      PROCESAMIENTO FINALIZADO DE FORMA EXITOSA
  * Capturas leídas : 50
  * Enviadas OK     : 50 / 50
========================================================

[Bash 5/6] Procesamiento finalizado. Deteniendo servidor de forma ordenada...
[Bash 6/6] Despliegue completado con éxito. Entorno de red limpio 


Las imágenes renderizadas de los mapas de presión se guardarán automáticamente en la carpeta tactile_images/ creada de forma dinámica.

Grado en Tecnologías Digitales para la Empresa - Universidad de Burgos.