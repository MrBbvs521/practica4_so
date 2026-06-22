#!/usr/bin/env bash
# ==============================================================================
# Práctica 4: Automatizador de Despliegue de Procesamiento e Interpolación
# ==============================================================================

set -euo pipefail

CPP_SRC="main.cpp"
CPP_BIN="./main"
PY_SERVER="server.py"
PORT=5000

echo "====================================================================="
echo "       INICIANDO INFRAESTRUCTURA DE PROCESAMIENTO TÁCTIL (UBU)       "
echo "====================================================================="

# 1. VERIFICAR DEPENDENCIAS CRUCIALES DE SISTEMA OPERATIVO
echo "[Bash 1/6] Verificando e instalando dependencias requeridas..."
sudo apt update -qq
sudo apt install -y -qq g++ libcurl4-openssl-dev nlohmann-json3-dev python3-pip python3-flask python3-matplotlib python3-numpy

# 2. COMPILACIÓN DEL PROCESADOR C++
if [[ ! -f "$CPP_SRC" ]]; then
    echo "[ERROR BASH] No se encuentra el archivo fuente $CPP_SRC" >&2
    exit 1
fi

echo "[Bash 2/6] Compilando el cliente C++ con estándar C++17 y librerías dinámicas..."
g++ -std=c++17 "$CPP_SRC" -lcurl -o main
echo "[OK] Compilación completada con éxito."

# 3. CONTROL DE PUERTOS DE CONEXIÓN RESIDUALES (Failsafe de Red)
if lsof -i :$PORT >/dev/null 2>&1; then
    echo "[Bash] Detectado puerto $PORT ocupado. Liberando puerto del sistema..."
    fuser -k $PORT/tcp 2>/dev/null || true
    sleep 1
fi

# 4. LANZAR EL SERVIDOR DE RENDERIZACIÓN FLASK EN SEGUNDO PLANO
if [[ ! -f "$PY_SERVER" ]]; then
    echo "[ERROR BASH] No se encuentra el script del servidor $PY_SERVER" >&2
    exit 1
fi

echo "[Bash 3/6] Iniciando el Servidor Python en segundo plano..."
python3 "$PY_SERVER" &
SERVER_PID=$!
echo "[INFO] Servidor Flask en background con PID: $SERVER_PID"

# Esperar unos segundos para permitir que Flask levante el puerto de red
sleep 3

# 5. EJECUTAR EL PROCESADOR EN C++ (Ingesta y procesamiento de los 50 JSONs)
echo "[Bash 4/6] Ejecutando procesamiento en C++..."
if ! $CPP_BIN; then
    echo "[ERROR BASH] El procesador en C++ falló durante la ejecución." >&2
    kill -SIGINT "$SERVER_PID" 2>/dev/null || true
    exit 1
fi

# 6. PARADA DE SERVICIO ORDENADA (Gestión de señales de SO)
echo "[Bash 5/6] Procesamiento finalizado. Deteniendo servidor de forma ordenada..."
if kill -0 "$SERVER_PID" 2>/dev/null; then
    kill -SIGINT "$SERVER_PID" 2>/dev/null
    sleep 1
    
    # SIGKILL como último recurso si el hilo de Flask no responde
    if kill -0 "$SERVER_PID" 2>/dev/null; then
        echo "[BASH ALERTA] Servidor bloqueado. Forzando cierre con SIGKILL..."
        kill -SIGKILL "$SERVER_PID" 2>/dev/null
    fi
fi

# Recolectar el proceso en el Kernel para evitar estados zombies
wait "$SERVER_PID" 2>/dev/null || true

echo "[Bash 6/6] Despliegue completado con éxito. Entorno de red limpio"