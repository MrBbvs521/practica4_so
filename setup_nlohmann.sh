#!/usr/bin/env bash
# ==============================================================================
# Práctica 4: Descarga y verificación local de la librería nlohmann/json
# ==============================================================================

set -euo pipefail

LIBRARY_FILE="json.hpp"
DOWNLOAD_URL="https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
TEST_CPP="test_nlohmann.cpp"
TEST_BIN="./test_nlohmann"

echo "====================================================================="
echo "        DESCARGANDO LIBRERÍA PORTABLE NLOHMANN/JSON (UBU)            "
echo "====================================================================="

# 1. COMPROBAR SI YA EXISTE LA LIBRERÍA
if [[ -f "$LIBRARY_FILE" ]]; then
    echo "[INFO] El archivo '$LIBRARY_FILE' ya existe en este directorio."
    read -p "¿Deseas volver a descargarlo e instalarlo de todos modos? (s/n): " RESPUESTA
    if [[ "$RESPUESTA" != "s" && "$RESPUESTA" != "S" ]]; then
        echo "[OK] Operación cancelada por el usuario."
        exit 0
    fi
fi

# 2. SELECCIONAR CLIENTE DE DESCARGA (curl o wget)
echo "[1/3] Descargando la cabecera limpia 'json.hpp' desde el repositorio oficial..."
if command -v curl >/dev/null 2>&1; then
    curl -Lo "$LIBRARY_FILE" "$DOWNLOAD_URL"
elif command -v wget >/dev/null 2>&1; then
    wget -O "$LIBRARY_FILE" "$DOWNLOAD_URL"
else
    echo "[ERROR] No se ha encontrado 'curl' ni 'wget' en tu sistema WSL." >&2
    echo "Instálalos usando: sudo apt install curl wget" >&2
    exit 1
fi

echo "[OK] Descarga completada con éxito."

# 3. VERIFICAR LA INTEGRIDAD GENERANDO UN PROGRAMA DE PRUEBA C++
echo "[2/3] Generando un programa temporal de prueba de C++..."

cat << 'EOF' > "$TEST_CPP"
#include <iostream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

int main() {
    // Crear un objeto JSON simple para testear la sintaxis
    json prueba;
    prueba["estatus"] = "correcto";
    prueba["mensaje"] = "Libreria nlohmann configurada e instalada con exito";
    prueba["curso"] = "2025-2026";
    
    cout << "\n[TEST COMPILACIÓN] " << prueba["mensaje"] << " ✅" << endl;
    cout << "[TEST COMPILACIÓN] Estatus del Kernel: " << prueba["estatus"] << endl;
    return 0;
}
EOF

# 4. COMPILAR Y EJECUTAR EL TEST
echo "[3/3] Compilando el programa de prueba..."
if g++ -std=c++17 "$TEST_CPP" -o test_nlohmann 2>/dev/null; then
    # Ejecutar la prueba
    "$TEST_BIN"
    # Limpieza de archivos temporales de testeo
    rm -f "$TEST_CPP" "$TEST_BIN"
    echo "====================================================================="
    echo " ¡Librería lista! Ya tienes '$LIBRARY_FILE' en tu carpeta local.  "
    echo " Ahora puedes compilar usando: #include \"json.hpp\"                 "
    echo "====================================================================="
else
    echo "[ERROR] Falló la compilación de prueba. Verifica tu compilador g++." >&2
    rm -f "$TEST_CPP"
    exit 1
fi