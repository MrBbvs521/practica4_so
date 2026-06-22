// ==============================================================================
// Práctica 4: Procesamiento, Interpolación Bilineal y Envío HTTP de Datos Táctiles
// ==============================================================================

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Constantes globales de configuración
const string ARCHIVO_JSON = "tactile_captures_50.json";
const string URL_SERVIDOR = "http://127.0.0.1:5000/upload";
const int FILAS_ORIGINAL = 16;
const int COLS_ORIGINAL = 16;
const int FILAS_INTERPOLADO = 128;
const int COLS_INTERPOLADO = 128;

// Estructura para almacenar una captura táctil individual
struct CapturaTactil {
    int id;
    vector<vector<double>> matriz;
};

// ==============================================================================
// 1. FUNCIÓN: Interpolación Bilineal (Mapear de 16x16 a 128x128)
// ==============================================================================
vector<vector<double>> interpolarMatrizBilineal(const vector<vector<double>>& original) {
    vector<vector<double>> interpolada(FILAS_INTERPOLADO, vector<double>(COLS_INTERPOLADO, 0.0));

    // Factor de escalado dinámico entre las dimensiones
    double escala_x = static_cast<double>(FILAS_ORIGINAL - 1) / (FILAS_INTERPOLADO - 1);
    double escala_y = static_cast<double>(COLS_ORIGINAL - 1) / (COLS_INTERPOLADO - 1);

    for (int i = 0; i < FILAS_INTERPOLADO; ++i) {
        for (int j = 0; j < COLS_INTERPOLADO; ++j) {
            // Mapear coordenadas del lienzo destino (128x128) al original (16x16)
            double x = i * escala_x;
            double y = j * escala_y;

            // Encontrar los 4 índices de los píxeles vecinos más cercanos en la matriz original
            int x1 = static_cast<int>(floor(x));
            int y1 = static_cast<int>(floor(y));
            int x2 = min(x1 + 1, FILAS_ORIGINAL - 1);
            int y2 = min(y1 + 1, COLS_ORIGINAL - 1);

            // Calcular las diferencias ponderadas (pesos) para la interpolación
            double dx = x - x1;
            double dy = y - y1;

            // Obtener los valores de presión de los cuatro vecinos
            double q11 = original[x1][y1];
            double q21 = original[x2][y1];
            double q12 = original[x1][y2];
            double q22 = original[x2][y2];

            // Ecuación matemática de la interpolación bilineal
            double valor_interpolado = (1.0 - dx) * (1.0 - dy) * q11 +
                                       dx * (1.0 - dy) * q21 +
                                       (1.0 - dx) * dy * q12 +
                                       dx * dy * q22;

            interpolada[i][j] = valor_interpolado;
        }
    }
    return interpolada;
}

// ==============================================================================
// 2. FUNCIÓN: Enviar la matriz de alta resolución vía HTTP POST usando libcurl
// ==============================================================================
bool enviarMatrizPorHTTP(const CapturaTactil& captura) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "[ERROR C++] No se pudo inicializar la librería libcurl." << endl;
        return false;
    }

    // Estructurar el objeto JSON a enviar
    json payload;
    payload["id"] = captura.id;
    payload["matrix"] = captura.matriz;

    string datos_serializados = payload.dump();

    // Configurar cabeceras de la petición HTTP indicando JSON
    struct curl_slist* cabeceras = nullptr;
    cabeceras = curl_slist_append(cabeceras, "Content-Type: application/json");

    // Configurar opciones de la llamada HTTP en Curl
    curl_easy_setopt(curl, CURLOPT_URL, URL_SERVIDOR.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, cabeceras);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, datos_serializados.c_str());

    // Ejecutar de forma síncrona la petición HTTP POST
    CURLcode resultado = curl_easy_perform(curl);
    
    long codigo_respuesta = 0;
    if (resultado == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigo_respuesta);
    }

    // Liberar memoria asignada a los sockets y estructuras de Curl
    curl_slist_free_all(cabeceras);
    curl_easy_cleanup(curl);

    if (resultado != CURLE_OK) {
        cerr << "[ERROR HTTP] Fallo al enviar captura " << captura.id 
             << ". Motivo: " << curl_easy_strerror(resultado) << endl;
        return false;
    }

    if (codigo_respuesta != 200) {
        cerr << "[ERROR HTTP] El servidor respondió con código de estado " << codigo_respuesta 
             << " para la captura " << captura.id << endl;
        return false;
    }

    return true;
}

// ==============================================================================
// PROGRAMA PRINCIPAL
// ==============================================================================
int main() {
    cout << "[C++] Iniciando el Procesador de Imágenes Táctiles..." << endl;

    // 1. LECTURA DEL ARCHIVO JSON CON LAS CAPTURAS
    ifstream archivo_entrada(ARCHIVO_JSON);
    if (!archivo_entrada.is_open()) {
        cerr << "[ERROR C++] No se pudo abrir el archivo de entrada: " << ARCHIVO_JSON << endl;
        return 1;
    }

    json datos_json;
    try {
        archivo_entrada >> datos_json;
    } catch (const json::parse_error& e) {
        cerr << "[ERROR C++] El archivo JSON contiene errores de sintaxis: " << e.what() << endl;
        archivo_entrada.close();
        return 1;
    }
    archivo_entrada.close();

    // Validar metadatos del sensor táctil
    if (!datos_json.contains("captures") || !datos_json["captures"].is_array()) {
        cerr << "[ERROR C++] La estructura del JSON no contiene una lista válida de capturas." << endl;
        return 1;
    }

    auto lista_capturas = datos_json["captures"];
    cout << "[C++] Archivo JSON cargado. Detectadas " << lista_capturas.size() << " capturas táctiles." << endl;

    int enviadas_con_exito = 0;

    // 2. BUCLE PRINCIPAL: Procesamiento e interpolación secuencial de las 50 muestras
    for (const auto& elemento : lista_capturas) {
        int id_captura = elemento["id"];
        vector<vector<double>> matriz_original = elemento["matrix"];

        // Validación estricta de las dimensiones originales (debe ser 16x16)
        if (matriz_original.size() != FILAS_ORIGINAL || matriz_original[0].size() != COLS_ORIGINAL) {
            cerr << "[ALERTA] La captura con ID " << id_captura 
                 << " tiene un tamaño incorrecto de " << matriz_original.size() 
                 << "x" << matriz_original[0].size() << ". Omitiendo..." << endl;
            continue;
        }

        // Aplicar interpolación bilineal para escalar la imagen táctil a 128x128
        vector<vector<double>> matriz_alta_resolucion = interpolarMatrizBilineal(matriz_original);

        // Empaquetar y enviar vía HTTP POST al servidor Python
        CapturaTactil captura_procesada = { id_captura, matriz_alta_resolucion };
        
        cout << "[C++] Procesando captura ID: " << id_captura << " [16x16 -> 128x128]. Enviando..." << endl;
        if (enviarMatrizPorHTTP(captura_procesada)) {
            enviadas_con_exito++;
        }
    }

    cout << "\n========================================================" << endl;
    cout << "      PROCESAMIENTO FINALIZADO DE FORMA EXITOSA" << endl;
    cout << "  * Capturas leídas : " << lista_capturas.size() << endl;
    cout << "  * Enviadas OK     : " << enviadas_con_exito << " / " << lista_capturas.size() << endl;
    cout << "========================================================" << endl;

    return 0;
}