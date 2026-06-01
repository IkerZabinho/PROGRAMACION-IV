// Cliente/RedCliente.cpp
#include "RedCliente.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

PaqueteRed enviarPeticionServidor(PaqueteRed paqueteAEnviar) {
    PaqueteRed paqueteRespuesta;
    // Inicializamos la respuesta con un error por si algo falla en la conexión
    memset(&paqueteRespuesta, 0, sizeof(PaqueteRed));
    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
    strcpy(paqueteRespuesta.mensajeRespuesta, "[Error] No se pudo conectar con el servidor.");

    // 1. Inicializar Winsock (Requisito obligatorio en Windows)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "[Red] Error al inicializar Winsock.\n";
        return paqueteRespuesta;
    }

    // 2. Crear el Socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        cout << "[Red] Error al crear el socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return paqueteRespuesta;
    }

    // 3. Configurar la dirección del Servidor (Localhost / Puerto 8888)
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888); // El puerto en el que escucha tu servidor
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP local (tu propia máquina)

    // 4. Conectarse al servidor
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "[Red] No se encontró el servidor encendido.\n";
        closesocket(sock);
        WSACleanup();
        return paqueteRespuesta;
    }

    // 5. ENVIAR el paquete con los datos (ej: el login)
    int bytesEnviados = send(sock, (char*)&paqueteAEnviar, sizeof(PaqueteRed), 0);
    if (bytesEnviados == SOCKET_ERROR) {
        cout << "[Red] Error al enviar datos.\n";
        closesocket(sock);
        WSACleanup();
        return paqueteRespuesta;
    }

    // 6. RECIBIR la respuesta del servidor
    int bytesRecibidos = recv(sock, (char*)&paqueteRespuesta, sizeof(PaqueteRed), 0);
    if (bytesRecibidos <= 0) {
        cout << "[Red] El servidor no respondió o cerró la conexión.\n";
        paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
        strcpy(paqueteRespuesta.mensajeRespuesta, "[Error] Sin respuesta del servidor.");
    }

    // 7. Limpiar y cerrar el socket
    closesocket(sock);
    WSACleanup();

    return paqueteRespuesta;
}