// main_servidor.cpp
#include <iostream>
#include <winsock2.h>
#include "../ComunPrueba/sqlite3.h"
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

            case OP_LOGIN:
            {
                registrarLog("Procesando Login Real para: " + string(paqueteRecibido.perfil.usuario));

                // Llamamos a la función real del módulo MotorBaseDatos
                bool exito = autenticarUsuarioSQL(db, paqueteRecibido, paqueteRespuesta);

                if (exito)
                {
                    registrarLog("LOGIN EXITOSO: " + string(paqueteRecibido.perfil.usuario) + " (ID: " + to_string(paqueteRespuesta.idUsuario) + ")");
                }
                else
                {
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

                // ... Dentro del switch (paqueteRecibido.tipoOperacion) en main_servidor.cpp

            case OP_DONACION_DINERO:
            {
                registrarLog("Procesando Donacion Economica. ID Donante: " + std::to_string(paqueteRecibido.idUsuario));

                // Usamos el campo nativo de donación del paquete de red
                float monto = paqueteRecibido.cantidadDonada;

                // Constructor de tu clase en Clases.h: Dinero(id, idDonacion, cantidad)
                GestionONG::Dinero miDinero(0, 0, monto);

                // Invocamos el método estático de la clase Donacion
                int resultado = GestionONG::Donacion::insertarDonacionDinero(db, miDinero, paqueteRecibido.idUsuario);

                if (resultado == 0 || resultado == SQLITE_OK) // Dependiendo de qué retorne tu método
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[EXITO] Donacion monetaria procesada con exito.\n");
                    registrarLog("DONACION OK: Registrados " + to_string(monto) + "€ para el usuario " + to_string(paqueteRecibido.idUsuario));
                }
                else
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] Error interno al procesar la donacion en la BD.");
                    registrarLog("ERROR: Fallo al insertar donacion de dinero en SQLite.");
                }
                break;
            }
            case OP_CONSULTAR_MIS_EVENTOS:
            {
                registrarLog("Usuario ID " + to_string(paqueteRecibido.idUsuario) + " solicita ver sus eventos.");

                sqlite3_stmt *stmt;
                // Tu consulta SQL exacta tal cual la tenías
                const char *sql_list =
                    "SELECT E.id_evento, E.descripcion, E.fecha_ini, E.tipo, E.material "
                    "FROM Evento E "
                    "JOIN Participaciones P ON E.id_evento = P.id_evento "
                    "WHERE P.id_voluntario = ? AND datetime(E.fecha_ini) >= datetime('now', 'localtime') "
                    "ORDER BY E.fecha_ini ASC;";

                string buffer = "";
                char lineaFila[256];

                if (sqlite3_prepare_v2(db, sql_list, -1, &stmt, 0) == SQLITE_OK)
                {
                    sqlite3_bind_int(stmt, 1, paqueteRecibido.idUsuario);

                    while (sqlite3_step(stmt) == SQLITE_ROW)
                    {
                        int id = sqlite3_column_int(stmt, 0);
                        const char *desc = (const char *)sqlite3_column_text(stmt, 1);
                        const char *fecha = (const char *)sqlite3_column_text(stmt, 2);
                        int tipo_int = sqlite3_column_int(stmt, 3);
                        int mat_int = sqlite3_column_int(stmt, 4);

                        // Tus traducciones idénticas de la Fase 1
                        const char *txtTipo = (tipo_int == 0) ? "Recogida" : "Reparto";
                        const char *txtMat = (mat_int == 0) ? "Ropa" : "Comida";

                        // Guardamos la fila formateada en un buffer de texto
                        sprintf(lineaFila, "%-5d | %-12s | %-10s | %-18s | %s\n", id, txtTipo, txtMat, fecha, desc ? desc : "");
                        buffer += lineaFila;
                    }
                    sqlite3_finalize(stmt);

                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    strncpy(paqueteRespuesta.mensajeRespuesta, buffer.c_str(), sizeof(paqueteRespuesta.mensajeRespuesta) - 1);
                }
                else
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    sprintf(paqueteRespuesta.mensajeRespuesta, "Error en el servidor: %s", sqlite3_errmsg(db));
                }
                break;
            }

            case OP_DESAPUNTAR_EVENTO:
            {
                registrarLog("Usuario ID " + to_string(paqueteRecibido.idUsuario) + " solicita desapuntarse del evento " + to_string(paqueteRecibido.idEvento) + ".");

                char sql_del[200];
                // Tu DELETE exacto adaptado
                sprintf(sql_del, "DELETE FROM Participaciones WHERE id_voluntario = %d AND id_evento = %d;",
                        paqueteRecibido.idUsuario, paqueteRecibido.idEvento);

                char *error = 0;
                if (sqlite3_exec(db, sql_del, 0, 0, &error) == SQLITE_OK)
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    if (sqlite3_changes(db) > 0)
                    {
                        strcpy(paqueteRespuesta.mensajeRespuesta, "\n[OK] Te has desapuntado con éxito.\n");
                    }
                    else
                    {
                        strcpy(paqueteRespuesta.mensajeRespuesta, "\n[!] No estabas inscrito en ese evento.\n");
                    }
                }
                else
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    sprintf(paqueteRespuesta.mensajeRespuesta, "\nError en Servidor: %s\n", error);
                    sqlite3_free(error);
                }
                break;
            }
            case OP_VER_EVENTOS_DISPONIBLES:
            {
                registrarLog("Usuario ID " + to_string(paqueteRecibido.idUsuario) + " solicita ver eventos disponibles.");

                sqlite3_stmt *stmt;
                // Tu consulta SQL exacta
                const char *sql_list =
                    "SELECT id_evento, descripcion, fecha_ini, tipo, material FROM Evento "
                    "WHERE date(fecha_ini) >= date('now') "
                    "AND id_evento NOT IN (SELECT id_evento FROM Participaciones WHERE id_voluntario = ?);";

                string bufferResultados = "";
                char lineaFila[256];

                if (sqlite3_prepare_v2(db, sql_list, -1, &stmt, 0) == SQLITE_OK)
                {
                    sqlite3_bind_int(stmt, 1, paqueteRecibido.idUsuario);

                    int encontrados = 0;
                    while (sqlite3_step(stmt) == SQLITE_ROW)
                    {
                        encontrados = 1;
                        int id = sqlite3_column_int(stmt, 0);
                        const char *desc = (const char *)sqlite3_column_text(stmt, 1);
                        const char *fecha = (const char *)sqlite3_column_text(stmt, 2);
                        int tipo_int = sqlite3_column_int(stmt, 3);
                        int mat_int = sqlite3_column_int(stmt, 4);

                        const char *txtTipo = (tipo_int == 0) ? "Recogida" : "Reparto";
                        const char *txtMat = (mat_int == 0) ? "Ropa" : "Comida";

                        sprintf(lineaFila, "%-4d | %-10s | %-10s | %-18s | %s\n", id, txtTipo, txtMat, fecha, desc ? desc : "");
                        bufferResultados += lineaFila;
                    }

                    if (!encontrados)
                    {
                        bufferResultados += "[INFO] No hay eventos nuevos disponibles para ti en este momento.\n";
                    }
                    sqlite3_finalize(stmt);

                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    strncpy(paqueteRespuesta.mensajeRespuesta, bufferResultados.c_str(), sizeof(paqueteRespuesta.mensajeRespuesta) - 1);
                }
                else
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    sprintf(paqueteRespuesta.mensajeRespuesta, "Error al consultar eventos en servidor: %s", sqlite3_errmsg(db));
                }
                break;
            }

            case OP_INSCRIBIR_EN_EVENTO:
            {
                int id_vol = paqueteRecibido.idUsuario;
                int id_ev = paqueteRecibido.idEvento;
                registrarLog("Procesando inscripción de Usuario ID " + to_string(id_vol) + " en Evento ID " + to_string(id_ev));

                // 1. Comprobar Choque de Fechas (Función de tu servidor)
                if (tieneChoqueDeFechas(db, id_vol, id_ev))
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "\n¡ERROR! Ya tienes otro compromiso registrado para ese mismo día.");
                    registrarLog("Inscripción denegada: Choque de fechas.");
                    break;
                }

                // 2. Comprobar Cupo (Función de tu servidor)
                if (estaEventoLleno(db, id_ev))
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "\n¡ERROR! El evento ya tiene suficientes voluntarios.");
                    registrarLog("Inscripción denegada: Cupo lleno.");
                    break;
                }

                // 3. Inserción final
                sqlite3_stmt *stmt;
                const char *sql_ins = "INSERT INTO Participaciones (id_voluntario, id_evento) VALUES (?, ?);";

                if (sqlite3_prepare_v2(db, sql_ins, -1, &stmt, 0) == SQLITE_OK)
                {
                    sqlite3_bind_int(stmt, 1, id_vol);
                    sqlite3_bind_int(stmt, 2, id_ev);

                    if (sqlite3_step(stmt) == SQLITE_DONE)
                    {
                        paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                        strcpy(paqueteRespuesta.mensajeRespuesta, "\n[OK] ¡Inscripción realizada con éxito! Gracias por tu colaboración.");
                        registrarLog("INSCRIPCIÓN EXITOSA: Voluntario " + to_string(id_vol) + " -> Evento " + to_string(id_ev));
                    }
                    else
                    {
                        paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                        sprintf(paqueteRespuesta.mensajeRespuesta, "\n[ERROR] No se pudo completar la inscripción: %s", sqlite3_errmsg(db));
                    }
                    sqlite3_finalize(stmt);
                }
                else
                {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    sprintf(paqueteRespuesta.mensajeRespuesta, "\n[ERROR] Error de preparación en servidor: %s", sqlite3_errmsg(db));
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
int tieneChoqueDeFechas(sqlite3 *db, int id_voluntario, int id_evento_nuevo)
{
    sqlite3_stmt *stmt;
    char fecha_objetivo[20] = "";
    int choque = 0;

    // A. Obtener la fecha del evento al que se quiere apuntar
    const char *sql_f = "SELECT date(fecha_ini) FROM Evento WHERE id_evento = ?;";
    if (sqlite3_prepare_v2(db, sql_f, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id_evento_nuevo);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            strcpy(fecha_objetivo, (const char *)sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    // B. Buscar si el voluntario ya tiene algo ese día (Evento o Taller)
    // Usamos UNION para mirar en las dos tablas de relación a la vez
    const char *sql_check =
        "SELECT COUNT(*) FROM ("
        "  SELECT date(e.fecha_ini) as fecha FROM Participaciones p "
        "  JOIN Evento e ON p.id_evento = e.id_evento WHERE p.id_voluntario = ? "
        "  UNION ALL "
        "  SELECT date(t.fecha) as fecha FROM Impartir i "
        "  JOIN Taller t ON i.id_taller = t.id_taller WHERE i.id_voluntario = ?"
        ") WHERE fecha = ?;";

    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id_voluntario);
        sqlite3_bind_int(stmt, 2, id_voluntario);
        sqlite3_bind_text(stmt, 3, fecha_objetivo, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            choque = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    return (choque > 0);
}
int estaEventoLleno(sqlite3 *db, int id_e)
{
    sqlite3_stmt *stmt;
    int lleno = 0;

    // La consulta obtiene el cupo máximo y cuántas participaciones hay ya registradas
    const char *sql = "SELECT E.lim_voluntarios, (SELECT COUNT(*) FROM Participaciones WHERE id_evento = ?) "
                      "FROM Evento E WHERE E.id_evento = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id_e);
        sqlite3_bind_int(stmt, 2, id_e);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int cupo_maximo = sqlite3_column_int(stmt, 0);
            int ocupados = sqlite3_column_int(stmt, 1);

            if (ocupados >= cupo_maximo)
            {
                lleno = 1; // El evento está lleno
            }
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("[!] Error al comprobar el cupo del evento: %s\n", sqlite3_errmsg(db));
    }

    return lleno;
}