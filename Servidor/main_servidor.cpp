// main_servidor.cpp
#include <iostream>
#include <winsock2.h>
#include "../sqlite3.h"
#include "../Comun/protocolo.h" 
#include "GestionLogs.h"
#include "ConfigServidor.h"
#include "MotorBaseDatos.h" 

#pragma comment(lib, "ws2_32.lib") // Enlaza la librería de sockets en Windows

using namespace std;

int main() {
    // 1. Cargar configuración (Requerimiento 5)
    ConfigServidor config;
    
    registrarLog("=== INICIANDO SERVIDOR ONG ===");
    registrarLog("Cargando configuracion...");

    // 2. Inicializar Base de Datos SQLite usando la ruta del .conf (¡Cero Hardcoding!)
    sqlite3* db;
    int rc = sqlite3_open(config.getDbPath().c_str(), &db);
    if (rc != SQLITE_OK) {
        registrarLog("ERROR CRITICO: No se pudo abrir la BD en: " + config.getDbPath());
        return 1;
    }
    registrarLog("Base de datos conectada con exito en la ruta: " + config.getDbPath());

    // 3. Inicializar Sockets de Windows (Winsock)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        registrarLog("ERROR: Fallo en WSAStartup (red).");
        sqlite3_close(db);
        return 1;
    }

    // 4. Crear Socket de Escucha
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        registrarLog("ERROR: No se pudo crear el socket de escucha.");
        WSACleanup();
        sqlite3_close(db);
        return 1;
    }

    // 5. Vincular el Puerto (Bind)
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(config.getPuerto());

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        registrarLog("ERROR: Fallo en BIND en el puerto " + to_string(config.getPuerto()));
        closesocket(serverSocket);
        WSACleanup();
        sqlite3_close(db);
        return 1;
    }

    // 6. Escuchar Peticiones (Listen)
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        registrarLog("ERROR: Fallo en LISTEN.");
        closesocket(serverSocket);
        WSACleanup();
        sqlite3_close(db);
        return 1;
    }

    registrarLog("Servidor en linea. Escuchando peticiones en el puerto: " + to_string(config.getPuerto()));

    // Bucle principal para recibir conexiones individuales
    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        
        // Espera a que un cliente se conecte
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET) {
            registrarLog("Advertencia: Error al aceptar conexion de un cliente.");
            continue;
        }

        registrarLog("Cliente conectado al socket.");

        PaqueteRed paqueteRecibido;
        // Recibimos el paquete estructurado que definimos en el Paso 1
        int bytesRecibidos = recv(clientSocket, (char*)&paqueteRecibido, sizeof(PaqueteRed), 0);
        
        if (bytesRecibidos > 0) {
            PaqueteRed paqueteRespuesta;
            memset(&paqueteRespuesta, 0, sizeof(PaqueteRed));

            // Aquí procesaremos las operaciones (Login, Registros...)
            switch (paqueteRecibido.tipoOperacion) {
                
                case OP_LOGIN: {
                    registrarLog("Procesando Login Real para: " + string(paqueteRecibido.perfil.usuario));
                    
                    // Llamamos a la función real del módulo MotorBaseDatos
                    bool exito = autenticarUsuarioSQL(db, paqueteRecibido, paqueteRespuesta);
                    
                    if (exito) {
                        registrarLog("LOGIN EXITOSO: " + string(paqueteRecibido.perfil.usuario) + " (ID: " + to_string(paqueteRespuesta.idUsuario) + ")");
                    } else {
                        registrarLog("LOGIN FALLIDO: " + string(paqueteRecibido.perfil.usuario) + " -> " + string(paqueteRespuesta.mensajeRespuesta));
                    }
                    break;
                }
                // ==========================

                default:
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "Operacion desconocida o no implementada.");
                    break;
            }

            // Enviamos la respuesta de vuelta por el socket
            send(clientSocket, (char*)&paqueteRespuesta, sizeof(PaqueteRed), 0);
        }

        // Cerramos la conexion con este cliente antes de pasar al siguiente
        closesocket(clientSocket);
        registrarLog("Cliente desconectado del socket.");
    }

    // Cierre limpio de recursos
    closesocket(serverSocket);
    WSACleanup();
    sqlite3_close(db);
    return 0;
}