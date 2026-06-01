// main_servidor.cpp
#include <iostream>
#include <winsock2.h>
#include "../sqlite3.h"
#include "../Comun/protocolo.h"
#include "GestionLogs.h"
#include "ConfigServidor.h"
#include "MotorBaseDatos.h"
#include "../ComunPrueba/Clases.h"

#pragma comment(lib, "ws2_32.lib") // Enlaza la librería de sockets en Windows

using namespace std;

int main()
{
    // 1. Cargar configuración (Requerimiento 5)
    ConfigServidor config;

    registrarLog("=== INICIANDO SERVIDOR ONG ===");
    registrarLog("Cargando configuracion...");

    // 2. Inicializar Base de Datos SQLite usando la ruta del .conf (¡Cero Hardcoding!)
    sqlite3 *db;
    int rc = sqlite3_open(config.getDbPath().c_str(), &db);
    if (rc != SQLITE_OK)
    {
        registrarLog("ERROR CRITICO: No se pudo abrir la BD en: " + config.getDbPath());
        return 1;
    }
    registrarLog("Base de datos conectada con exito en la ruta: " + config.getDbPath());

    // 3. Inicializar Sockets de Windows (Winsock)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        registrarLog("ERROR: Fallo en WSAStartup (red).");
        sqlite3_close(db);
        return 1;
    }

    // 4. Crear Socket de Escucha
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
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

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        registrarLog("ERROR: Fallo en BIND en el puerto " + to_string(config.getPuerto()));
        closesocket(serverSocket);
        WSACleanup();
        sqlite3_close(db);
        return 1;
    }

    // 6. Escuchar Peticiones (Listen)
    if (listen(serverSocket, 1) == SOCKET_ERROR)
    {
        registrarLog("ERROR: Fallo en LISTEN.");
        closesocket(serverSocket);
        WSACleanup();
        sqlite3_close(db);
        return 1;
    }

    registrarLog("Servidor en linea. Escuchando peticiones en el puerto: " + to_string(config.getPuerto()));

    // Bucle principal para recibir conexiones individuales
    while (true)
    {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);

        // Espera a que un cliente se conecte
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET)
        {
            registrarLog("Advertencia: Error al aceptar conexion de un cliente.");
            continue;
        }

        registrarLog("Cliente conectado al socket.");

        PaqueteRed paqueteRecibido;
        // Recibimos el paquete estructurado que definimos en el Paso 1
        int bytesRecibidos = recv(clientSocket, (char *)&paqueteRecibido, sizeof(PaqueteRed), 0);

        if (bytesRecibidos > 0)
        {
            PaqueteRed paqueteRespuesta;
            memset(&paqueteRespuesta, 0, sizeof(PaqueteRed));

            // Aquí procesaremos las operaciones (Login, Registros...)
            switch (paqueteRecibido.tipoOperacion)
            {

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
                 };
            case OP_REGISTRO_BENEFICIARIO:
            {
                registrarLog("Procesando registro de Beneficiario: " + std::string(paqueteRecibido.perfil.usuario));

                // 1. Instanciamos el usuario base usando el namespace e inyectando los datos planos de red
                GestionONG::Usuario nuevoUsuario(
                    0, // ID provisional (SQLite lo sobreescribirá)
                    paqueteRecibido.perfil.nombre,
                    paqueteRecibido.perfil.apellidos,
                    paqueteRecibido.perfil.usuario,
                    paqueteRecibido.perfil.contrasena,
                    GestionONG::BENEFICIARIO);

                // 2. Calculamos los ingresos agregados a partir de tu estructura de red 'economia'
                float ingresosTotales = paqueteRecibido.economia.sueldo + paqueteRecibido.economia.otras_ayudas;
                float gastosTotales = paqueteRecibido.economia.alquiler + paqueteRecibido.economia.suministros +
                                      paqueteRecibido.economia.estudios + paqueteRecibido.economia.otros_gastos;

                // 3. Creamos el objeto de la clase hija 'Beneficiario' en la memoria del Servidor
                GestionONG::Beneficiario datosHijo(
                    0, // id_usuario (se enlazará automáticamente por transacciones)
                    paqueteRecibido.perfil.nombre,
                    paqueteRecibido.perfil.apellidos,
                    paqueteRecibido.perfil.usuario,
                    paqueteRecibido.perfil.contrasena,
                    0, // id_beneficiario autoincremental
                    paqueteRecibido.economia.adultos,
                    paqueteRecibido.economia.ninos,
                    ingresosTotales,
                    gastosTotales);

                // 4. Invocamos tu función polimórfica controladora con el puntero a datos específicos
                int idPerfilGen = GestionONG::Usuario::insertarUsuario(db, nuevoUsuario, &datosHijo);

                if (idPerfilGen != -1)
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "¡Beneficiario registrado de forma segura con transacciones!");
                    registrarLog("REGISTRO OK: Beneficiario guardado en cascada.");
                }
                else
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] Falló la inserción en la base de datos.");
                    registrarLog("REGISTRO FALLIDO: Error en transaccion controladora.");
                }
                break;
            }

            case OP_REGISTRO_VOLUNTARIO:
            {
                registrarLog("Procesando registro de Voluntario: " + std::string(paqueteRecibido.perfil.usuario));

                GestionONG::Usuario nuevoUsuario(
                    0, paqueteRecibido.perfil.nombre, paqueteRecibido.perfil.apellidos,
                    paqueteRecibido.perfil.usuario, paqueteRecibido.perfil.contrasena, GestionONG::VOLUNTARIO);

                GestionONG::Voluntario datosHijo(
                    0, paqueteRecibido.perfil.nombre, paqueteRecibido.perfil.apellidos,
                    paqueteRecibido.perfil.usuario, paqueteRecibido.perfil.contrasena, 0, "General");

                int idPerfilGen = GestionONG::Usuario::insertarUsuario(db, nuevoUsuario, &datosHijo);

                if (idPerfilGen != -1)
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "¡Voluntario registrado correctamente!");
                }
                else
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] No se pudo registrar el voluntario.");
                }
                break;
            }

            case OP_REGISTRO_DONANTE:
            {
                registrarLog("Procesando registro de Donante: " + std::string(paqueteRecibido.perfil.usuario));

                GestionONG::Usuario nuevoUsuario(
                    0, paqueteRecibido.perfil.nombre, paqueteRecibido.perfil.apellidos,
                    paqueteRecibido.perfil.usuario, paqueteRecibido.perfil.contrasena, GestionONG::DONANTE);

                // Mapeamos a tu clase Donante de Clases.h
                GestionONG::Donante datosHijo(
                    0, paqueteRecibido.perfil.nombre, paqueteRecibido.perfil.apellidos,
                    paqueteRecibido.perfil.usuario, paqueteRecibido.perfil.contrasena, 0);

                int idPerfilGen = GestionONG::Usuario::insertarUsuario(db, nuevoUsuario, &datosHijo);

                if (idPerfilGen != -1)
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "¡Donante registrado correctamente!");
                }
                else
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] No se pudo registrar el donante.");
                }
                break;
            }

            case OP_CONSULTAR_EVENTOS:
            {
                registrarLog("Usuario consulta remotamente las recogidas de ropa activas.");

                sqlite3_stmt *stmt;
                // tipo = 1 corresponde a RECOGIDA en tu enum TipoEvento
                std::string sql = "SELECT descripcion, fecha_ini, fecha_fin, lim_voluntarios FROM Evento "
                                  "WHERE tipoEvento = 1 AND date(fecha_ini) >= date('now') "
                                  "ORDER BY fecha_ini ASC LIMIT 5;";

                std::string bufferEventos = "\n--- PRÓXIMAS RECOGIDAS DE ROPA PROGRAMADAS ---\n";
                char linea[120];
                sprintf(linea, "%-25s | %-17s | %-12s\n", "DESCRIPCIÓN", "FECHA INICIO", "CUPO MAX");
                bufferEventos += linea;
                bufferEventos += "-------------------------------------------------------------\n";

                if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK)
                {
                    int hay_datos = 0;
                    while (sqlite3_step(stmt) == SQLITE_ROW)
                    {
                        hay_datos = 1;
                        const char *desc = (const char *)sqlite3_column_text(stmt, 0);
                        const char *ini = (const char *)sqlite3_column_text(stmt, 1);
                        int limite = sqlite3_column_int(stmt, 3);

                        sprintf(linea, "-> %-22s | %-12s | %d voluntarios\n",
                                desc ? desc : "Recogida", ini ? ini : "---", limite);
                        bufferEventos += linea;
                    }
                    sqlite3_finalize(stmt);

                    if (!hay_datos)
                    {
                        bufferEventos += "No hay campañas de recogida planificadas próximamente.\n";
                    }

                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    strncpy(paqueteRespuesta.mensajeRespuesta, bufferEventos.c_str(), sizeof(paqueteRespuesta.mensajeRespuesta) - 1);
                }
                else
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] Error en el motor de eventos de la BD.");
                }
                break;
            }

            case OP_DONACION_DINERO:
            {
                registrarLog("Procesando Donación Económica. ID Donante: " + std::to_string(paqueteRecibido.idUsuario));

                // Instanciamos el detalle de Dinero respetando tu constructor: Dinero(id, idD, cantidad)
                // Usamos paqueteRecibido.economia.sueldo como el campo para pasar la cantidad donada
                GestionONG::Dinero miDinero(0, 0, paqueteRecibido.economia.sueldo);

                // Llamamos al método estático exacto de tu clase Donacion
                int resultado = GestionONG::Donacion::insertarDonacionDinero(db, miDinero, paqueteRecibido.idUsuario);

                if (resultado == 0)
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "¡Donación monetaria procesada e inyectada con éxito!");
                    registrarLog("DONACIÓN OK: Guardada usando transacciones seguras.");
                }
                else
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] Error interno al procesar la donación.");
                }
                break;
            }

            default:
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] Operación de red no reconocida por el Servidor.");
                break;
            }
            // ==========================

            /*   default:
                  paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                  strcpy(paqueteRespuesta.mensajeRespuesta, "Operacion desconocida o no implementada.");
                  break;
          }
*/
            // Enviamos la respuesta de vuelta por el socket
            send(clientSocket, (char *)&paqueteRespuesta, sizeof(PaqueteRed), 0);
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