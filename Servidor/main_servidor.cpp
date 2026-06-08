// main_servidor.cpp
#include <iostream>
#include <winsock2.h>
#include "../ComunPrueba/sqlite3.h"
#include "../Comun/protocolo.h"
#include "GestionLogs.h"
#include "ConfigServidor.h"
#include "MotorBaseDatos.h"
#include "../ComunPrueba/Clases.h"
#include "interfaz.h"
#include <ctime>

#pragma comment(lib, "ws2_32.lib") // Enlaza la librería de sockets en Windows

using namespace std;


int tieneChoqueDeFechas(sqlite3* db, int id_voluntario, int id_evento_nuevo);
int estaEventoLleno(sqlite3* db, int id_e);
void verTalleresProximos(sqlite3* db, PaqueteRed& paqueteOut);
void verProximoRepartoRopa(sqlite3* db, PaqueteRed& paqueteIn, PaqueteRed& paqueteOut);
void verProximoRepartoComida(sqlite3* db, PaqueteRed& paqueteOut);
int actualizarDatosBeneficiario(sqlite3 *db, int id_beneficiario, GestionONG::Beneficiario b);


int callbackListarUsuarios(void* data, int argc, char** argv, char** azColName) {
    std::string* lista = static_cast<std::string*>(data);
    
    std::string id = argv[0] ? argv[0] : "NULL";
    std::string nombre = argv[1] ? argv[1] : "NULL";
    std::string apellidos = argv[2] ? argv[2] : "NULL";
    std::string tipoNum = argv[3] ? argv[3] : "0";
    
    std::string tipoTexto = "Desconocido";
    if (tipoNum == "1") tipoTexto = "Voluntario";
    else if (tipoNum == "2") tipoTexto = "Donante";
    else if (tipoNum == "3") tipoTexto = "Beneficiario";
    else if (tipoNum == "4") tipoTexto = "Administrador";

    *lista += "[ID: " + id + "] " + nombre + " " + apellidos + " (" + tipoTexto + ")\n";
    return 0;
}

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
            }
            
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
// ============================================================================
            // COMPROBAR Y REEMPLAZAR SOLO ESTOS CASES DENTRO DE TU SWITCH PRINCIPAL
            // ============================================================================
            case OP_CREAR_EVENTO: 
            {
                registrarLog("LOG ADMIN: Solicitud OP_CREAR_EVENTO.");
                char consulta[1024];
                char fecha_ini_str[30];
                char fecha_fin_str[30];

                snprintf(fecha_ini_str, sizeof(fecha_ini_str), "%04d-%02d-%02d %02d:00", 
                         paqueteRecibido.admin.f_inicio.anyo, paqueteRecibido.admin.f_inicio.mes, 
                         paqueteRecibido.admin.f_inicio.dia, paqueteRecibido.admin.f_inicio.hora);

                snprintf(fecha_fin_str, sizeof(fecha_fin_str), "%04d-%02d-%02d %02d:00", 
                         paqueteRecibido.admin.f_final.anyo, paqueteRecibido.admin.f_final.mes, 
                         paqueteRecibido.admin.f_final.dia, paqueteRecibido.admin.f_final.hora);
                
                snprintf(consulta, sizeof(consulta), 
                         "INSERT INTO Evento (material, descripcion, fecha_ini, fecha_fin, tipo, lim_voluntarios) "
                         "VALUES ('%s', '%s', '%s', '%s', '%s', %d);",
                         paqueteRecibido.admin.nombre_taller_o_material, 
                         paqueteRecibido.admin.descripcion, fecha_ini_str, fecha_fin_str, 
                         paqueteRecibido.perfil.nombre, paqueteRecibido.admin.cupo_o_limite);

                int rc = sqlite3_exec(db, consulta, 0, 0, 0);
                if (rc == SQLITE_OK) {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[Servidor] Éxito: Evento creado correctamente.");
                } else {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    snprintf(paqueteRespuesta.mensajeRespuesta, sizeof(paqueteRespuesta.mensajeRespuesta), "[Servidor] Error SQL: %s", sqlite3_errmsg(db));
                }
                break;
            }

            case OP_BORRAR_EVENTO: 
            {
                registrarLog("LOG ADMIN: Solicitud OP_BORRAR_EVENTO.");
                char consulta[256];
                snprintf(consulta, sizeof(consulta), "DELETE FROM Evento WHERE id_evento = %d;", paqueteRecibido.idEvento);
                
                int rc = sqlite3_exec(db, consulta, 0, 0, 0);
                if (rc == SQLITE_OK && sqlite3_changes(db) > 0) {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[Servidor] Éxito: Evento eliminado.");
                } else {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[Servidor] Error: ID de evento no encontrado o inexistente.");
                }
                break;
            }
            case OP_LISTAR_EVENTOS:
            {
                const char *sql_list = "SELECT id_evento, descripcion, fecha_ini FROM Evento;";
                sqlite3_stmt *stmt;
                string lista = "\n--- EVENTOS DISPONIBLES ---\nID\t| Descripcion\t| Fecha Inicio\n---------------------------------------\n";

                if (sqlite3_prepare_v2(db, sql_list, -1, &stmt, 0) == SQLITE_OK) {
                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        int id = sqlite3_column_int(stmt, 0);
                        string nombre = (const char*)sqlite3_column_text(stmt, 1);
                        string tipo = (const char*)sqlite3_column_text(stmt, 2);
                        
                        lista += to_string(id) + "\t| " + nombre + "\t| " + tipo + "\n";
                    }
                    sqlite3_finalize(stmt);
                } else {
                    lista = "[!] Error al acceder a la base de datos de eventos.";
                }

                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                strncpy(paqueteRespuesta.mensajeRespuesta, lista.c_str(), sizeof(paqueteRespuesta.mensajeRespuesta) - 1);
                break;
            }


            case OP_LISTAR_USUARIOS: 
            {
                registrarLog("LOG ADMIN: Solicitud OP_LISTAR_USUARIOS.");
                std::string bufferUsuarios = "";
                char* errorMsg = 0;
                const char* consulta = "SELECT id_usuario, nombre, apellidos, tipo FROM Usuarios;";
                
                int rc = sqlite3_exec(db, consulta, callbackListarUsuarios, &bufferUsuarios, &errorMsg);
                if (rc == SQLITE_OK) {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    if (bufferUsuarios.empty()) {
                        strcpy(paqueteRespuesta.mensajeRespuesta, "No hay usuarios registrados en el sistema.");
                    } else {
                        strncpy(paqueteRespuesta.mensajeRespuesta, bufferUsuarios.c_str(), sizeof(paqueteRespuesta.mensajeRespuesta) - 1);
                    }
                } else {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[Servidor] Error crítico al consultar la tabla Usuarios.");
                }
                break;
            }

            case OP_BAJA_USUARIO: 
            {
                registrarLog("LOG ADMIN: Solicitud OP_BAJA_USUARIO.");
                char consulta[256];
                snprintf(consulta, sizeof(consulta), "DELETE FROM Usuarios WHERE id_usuario = %d;", paqueteRecibido.idUsuario);
                
                int rc = sqlite3_exec(db, consulta, 0, 0, 0);
                if (rc == SQLITE_OK && sqlite3_changes(db) > 0) {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[Servidor] Éxito: Usuario eliminado del sistema.");
                } else {
                    paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                    strcpy(paqueteRespuesta.mensajeRespuesta, "[Servidor] Error: El ID de usuario especificado no existe.");
                }
                break;
            }

           case OP_REGISTRAR_ROPA: 
{
    registrarLog("LOG ADMIN: Solicitud OP_REGISTRAR_ROPA.");
    char consulta[512];
    
    snprintf(consulta, sizeof(consulta), 
            "INSERT INTO RecogidaRopa (id_beneficiario, id_evento, fecha_recogida) "
            "VALUES (%d, %d, (SELECT fecha_fin FROM Evento WHERE id_evento = %d));", 
            paqueteRecibido.idUsuario, 
            paqueteRecibido.idEvento,
            paqueteRecibido.idEvento); 
                
    int rc = sqlite3_exec(db, consulta, 0, 0, 0);
    if (rc == SQLITE_OK) {
        paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
        strncpy(paqueteRespuesta.mensajeRespuesta, 
                "\n[Servidor] Éxito: Registro insertado en RecogidaRopa con la fecha del evento.\n", 
                sizeof(paqueteRespuesta.mensajeRespuesta) - 1);
    } else {
        paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
        snprintf(paqueteRespuesta.mensajeRespuesta, sizeof(paqueteRespuesta.mensajeRespuesta), 
                "\n[Servidor] Error SQL: %s\n", sqlite3_errmsg(db));
    }
    break;
}
            case OP_LISTAR_BENEFICIARIOS:
            {
                const char *sql = "SELECT id_usuario, nombre, apellidos FROM Usuarios WHERE tipo = 3;";
                sqlite3_stmt *stmt;
                string lista = "";

                if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        int id = sqlite3_column_int(stmt, 0);
                        const char* nom = (const char*)sqlite3_column_text(stmt, 1);
                        const char* ape = (const char*)sqlite3_column_text(stmt, 2);
                        
                        lista += "ID: " + to_string(id) + " - " + (nom ? nom : "") + " " + (ape ? ape : "") + "\n";
                    }
                    sqlite3_finalize(stmt);
                } else {
                    lista = "[!] Error al cargar beneficiarios.";
                }

                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                strncpy(paqueteRespuesta.mensajeRespuesta, lista.c_str(), sizeof(paqueteRespuesta.mensajeRespuesta) - 1);
                break;
            }
            case OP_LISTAR_EVENTOS_ROPA:
            {
                const char *sql = "SELECT id_evento, descripcion, fecha_fin FROM Evento WHERE tipo = 0;";
                sqlite3_stmt *stmt;
                string lista = "";

                if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        int id = sqlite3_column_int(stmt, 0);
                        const char* desc = (const char*)sqlite3_column_text(stmt, 1);
                        const char* fecha_fin = (const char*)sqlite3_column_text(stmt, 2);
                        
                        lista += "ID: " + to_string(id) + " - " + (desc ? desc : "[Sin descripción]") 
                            + " (Fin: " + (fecha_fin ? fecha_fin : "No asignada") + ")\n";
                    }
                    sqlite3_finalize(stmt);
                    
                    if (lista.empty()) {
                        lista = "[No hay eventos de reparto de ropa futuros (tipo=0) disponibles]\n";
                    }
                } else {
                    lista = "[!] Error al acceder a la tabla de eventos filtrados (tipo=0).";
                }

                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                strncpy(paqueteRespuesta.mensajeRespuesta, lista.c_str(), sizeof(paqueteRespuesta.mensajeRespuesta) - 1);
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

           //  BLOQUE SWITCH CORREGIDO EN EL SERVIDOR (main_servidor.cpp)
case OP_CONSULTAR_EVENTOS:
    {
        // 🌟 SI QUIEN SOLICITA ES UN BENEFICIARIO (Rol 3)
        if (paqueteRecibido.tipoUsuario == 3) 
        {
            // ✔️ CORREGIDO: Cambiado paqueteIn por paqueteRecibido
            registrarLog("Beneficiario ID " + to_string(paqueteRecibido.idUsuario) + " consulta horarios de ayuda.");
            
            // 1. Obtenemos el texto de la comida
            PaqueteRed ayudaComida;
            memset(&ayudaComida, 0, sizeof(PaqueteRed));
            verProximoRepartoComida(db, ayudaComida);
            
            // 2. Obtenemos el texto de la ropa
            PaqueteRed ayudaRopa;
            memset(&ayudaRopa, 0, sizeof(PaqueteRed));
            // ✔️ CORREGIDO: Cambiado paqueteIn por paqueteRecibido
            verProximoRepartoRopa(db, paqueteRecibido, ayudaRopa);
            
            // 3. Fusionamos los mensajes en el paquete oficial de respuesta (paqueteRespuesta)
            paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
            string total = ayudaComida.mensajeRespuesta;
            total += ayudaRopa.mensajeRespuesta;
            
            strncpy(paqueteRespuesta.mensajeRespuesta, total.c_str(), sizeof(paqueteRespuesta.mensajeRespuesta) - 1);
            
            // Enviamos usando clientSocket
            send(clientSocket, (char*)&paqueteRespuesta, sizeof(PaqueteRed), 0);
        }
        else 
        {
            // 🌟 LOGICA ORIGINAL PARA EL VOLUNTARIO (Historial de eventos)
            registrarLog("Voluntario ID " + to_string(paqueteRecibido.idUsuario) + " solicita historial de eventos.");
            
            sqlite3_stmt *stmt;
            const char *sql_hist =
                "SELECT E.id_evento, E.descripcion, E.fecha_ini, E.tipo FROM Evento E "
                "JOIN Participaciones P ON E.id_evento = P.id_evento "
                "WHERE P.id_voluntario = ? AND date(E.fecha_ini) < date('now');";

            string bufferResultados = "";
            char lineaFila[256];

            if (sqlite3_prepare_v2(db, sql_hist, -1, &stmt, 0) == SQLITE_OK)
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
                    const char *txtTipo = (tipo_int == 1) ? "Recogida" : "Reparto";

                    sprintf(lineaFila, "%-4d | %-10s | %-12s | %s\n", id, txtTipo, fecha, desc ? desc : "");
                    bufferResultados += lineaFila;
                }

                if (!encontrados) {
                    bufferResultados += "[INFO] No tienes eventos pasados registrados.\n";
                }
                sqlite3_finalize(stmt);

                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                strncpy(paqueteRespuesta.mensajeRespuesta, bufferResultados.c_str(), sizeof(paqueteRespuesta.mensajeRespuesta) - 1);
            }
            else
            {
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                sprintf(paqueteRespuesta.mensajeRespuesta, "Error base datos historial: %s", sqlite3_errmsg(db));
            }

            // Enviamos de vuelta al voluntario usando clientSocket
            send(clientSocket, (char*)&paqueteRespuesta, sizeof(PaqueteRed), 0);
        }
        break;
    }

                // ... Dentro del switch (paqueteRecibido.tipoOperacion) en main_servidor.cpp

        // ====================================================================
        // CASO 1: REGISTRAR DONACIÓN DE DINERO
        // ====================================================================
        case OP_DONACION_DINERO: {
            registrarLog("Peticion recibida: Registrar donacion de dinero del Usuario ID: " + to_string(paqueteRecibido.idUsuario));
            
            int idDonanteReal = -1;
            const char* sqlBusqueda = "SELECT id_donante FROM Donantes WHERE id_usuario = ?;";
            sqlite3_stmt* stmtBusqueda;
            if (sqlite3_prepare_v2(db, sqlBusqueda, -1, &stmtBusqueda, 0) == SQLITE_OK) {
                sqlite3_bind_int(stmtBusqueda, 1, paqueteRecibido.idUsuario);
                if (sqlite3_step(stmtBusqueda) == SQLITE_ROW) {
                    idDonanteReal = sqlite3_column_int(stmtBusqueda, 0);
                }
                sqlite3_finalize(stmtBusqueda);
            }

            if (idDonanteReal == -1) {
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] No se encontro perfil de Donante para este usuario.");
                break;
            }

            float monto = paqueteRecibido.cantidadDonada;
            GestionONG::Dinero miDinero(0, idDonanteReal, monto);

            // Vinculamos usando el ID de donante real en la función de inserción
            int rc = GestionONG::Donacion::insertarDonacionDinero(db, miDinero, idDonanteReal);

            if (rc == SQLITE_DONE || rc == SQLITE_OK) {
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                sprintf(paqueteRespuesta.mensajeRespuesta, "\n[OK] Servidor: Donacion de %.2f EUR registrada correctamente.\n", monto);
            } else {
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] No se pudo registrar la donacion de dinero.");
            }
            break;
        }

        // ====================================================================
        // CASO 2: REGISTRAR DONACIÓN DE COMIDA
        // ====================================================================
        case OP_DONACION_COMIDA: {
            registrarLog("Peticion recibida: Registrar donacion de comida del Usuario ID: " + to_string(paqueteRecibido.idUsuario));
            
            int idDonanteReal = -1;
            const char* sqlBusqueda = "SELECT id_donante FROM Donantes WHERE id_usuario = ?;";
            sqlite3_stmt* stmtBusqueda;
            if (sqlite3_prepare_v2(db, sqlBusqueda, -1, &stmtBusqueda, 0) == SQLITE_OK) {
                sqlite3_bind_int(stmtBusqueda, 1, paqueteRecibido.idUsuario);
                if (sqlite3_step(stmtBusqueda) == SQLITE_ROW) {
                    idDonanteReal = sqlite3_column_int(stmtBusqueda, 0);
                }
                sqlite3_finalize(stmtBusqueda);
            }

            if (idDonanteReal == -1) {
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] No se encontro perfil de Donante para este usuario.");
                break;
            }

            int selectComida = paqueteRecibido.idEvento; 
            float kilos = paqueteRecibido.cantidadDonada;

            // 🌟 CORRECCIÓN: Calcular la fecha real de hoy en formato YYYY-MM-DD en texto plano
            time_t t = time(0);
            struct tm * now = localtime(&t);
            char fechaActual[11];
            strftime(fechaActual, sizeof(fechaActual), "%Y-%m-%d", now);

            // 🌟 Ahora pasamos la variable fechaActual en lugar de las comillas fijas
            GestionONG::Donacion baseDonacion(0, idDonanteReal, static_cast<GestionONG::TipoDonacion>(1), fechaActual); 
            GestionONG::Comida miComida(0, static_cast<GestionONG::TipoComida>(selectComida), kilos, idDonanteReal);

            int rc = GestionONG::Donacion::insertarDonacionComidaDB(db, baseDonacion, miComida);

            if (rc == SQLITE_DONE || rc == SQLITE_OK) {
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                sprintf(paqueteRespuesta.mensajeRespuesta, "\n[OK] Servidor: Recibidos %.2f kg de comida.\n", kilos);
            } else {
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] No se pudo procesar la donacion de comida.");
            }
            break;
        }

        // ====================================================================
        // CASO 3: REGISTRAR DONACIÓN DE ROPA
        // ====================================================================
        case OP_DONACION_ROPA: {
            registrarLog("Peticion recibida: Registrar donacion de ropa del Usuario ID: " + to_string(paqueteRecibido.idUsuario));
            
            int idDonanteReal = -1;
            const char* sqlBusqueda = "SELECT id_donante FROM Donantes WHERE id_usuario = ?;";
            sqlite3_stmt* stmtBusqueda;
            if (sqlite3_prepare_v2(db, sqlBusqueda, -1, &stmtBusqueda, 0) == SQLITE_OK) {
                sqlite3_bind_int(stmtBusqueda, 1, paqueteRecibido.idUsuario);
                if (sqlite3_step(stmtBusqueda) == SQLITE_ROW) {
                    idDonanteReal = sqlite3_column_int(stmtBusqueda, 0);
                }
                sqlite3_finalize(stmtBusqueda);
            }

            if (idDonanteReal == -1) {
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] No se encontro perfil de Donante para este usuario.");
                break;
            }

            float kilos = paqueteRecibido.cantidadDonada;
            GestionONG::Ropa miRopa(0, idDonanteReal, kilos);

            int rc = GestionONG::Donacion::insertarDonacionRopa(db, miRopa, idDonanteReal);

            if (rc == SQLITE_DONE || rc == SQLITE_OK) {
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                sprintf(paqueteRespuesta.mensajeRespuesta, "\n[OK] Servidor: Almacenados %.2f kg de ropa.\n", kilos);
            } else {
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] Error al guardar la donacion de ropa.");
            }
            break;
        }
        
        // ====================================================================
        // CASO 4: CONSULTAR HISTORIAL COMPLETO DE DONACIONES
        // ====================================================================
        case OP_CONSULTAR_DONACIONES: {
            registrarLog("Peticion recibida: Historial completo de donaciones del Usuario ID: " + to_string(paqueteRecibido.idUsuario));
            
            // 1. Buscamos el id_donante real mapeado con el id_usuario de red
            int idDonanteReal = -1;
            const char* sqlBusqueda = "SELECT id_donante FROM Donantes WHERE id_usuario = ?;";
            sqlite3_stmt* stmtBusqueda;
            if (sqlite3_prepare_v2(db, sqlBusqueda, -1, &stmtBusqueda, 0) == SQLITE_OK) {
                sqlite3_bind_int(stmtBusqueda, 1, paqueteRecibido.idUsuario);
                if (sqlite3_step(stmtBusqueda) == SQLITE_ROW) {
                    idDonanteReal = sqlite3_column_int(stmtBusqueda, 0);
                }
                sqlite3_finalize(stmtBusqueda);
            }

            if (idDonanteReal == -1) {
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                strcpy(paqueteRespuesta.mensajeRespuesta, "[ERROR] No se encontro el perfil de donante.");
                registrarLog("ERROR HISTORIAL: id_donante no encontrado para Usuario ID: " + to_string(paqueteRecibido.idUsuario));
                break;
            }

            sqlite3_stmt *stmt;
            // 🌟 Esta consulta barre TODAS las donaciones del ID sin importar la fecha
            const char *sql =
                "SELECT d.tipo, r.kilos, c.tipo_comida, c.kilos, din.cantidad, d.fecha "
                "FROM Donaciones d "
                "LEFT JOIN Ropa r ON d.id_donacion = r.id_donacion "
                "LEFT JOIN Comida c ON d.id_donacion = c.id_donacion "
                "LEFT JOIN Dinero din ON d.id_donacion = din.id_donacion "
                "WHERE d.id_donante = ? ORDER BY d.id_donacion DESC;";

            string tabla = "";
            char fila[256];

            sprintf(fila, "\n%-12s | %-32s | %-20s\n", "TIPO", "DETALLES", "FECHA");
            tabla += fila;
            tabla += "--------------------------------------------------------------------\n";

            if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, idDonanteReal);

                int count = 0;
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    count++;
                    int tipo = sqlite3_column_int(stmt, 0);
                    double k_ropa = sqlite3_column_double(stmt, 1);
                    int t_comida = sqlite3_column_int(stmt, 2);
                    double k_comida = sqlite3_column_double(stmt, 3);
                    double importe = sqlite3_column_double(stmt, 4);
                    const char *fecha_raw = (const char *)sqlite3_column_text(stmt, 5);
                    string fecha = fecha_raw ? fecha_raw : "Sin fecha";

                    switch (tipo) {
                        case 1: // COMIDA 
                            {
                                const char* txtC;
                                if (t_comida == 1) txtC = "Carbohidratos";
                                else if (t_comida == 2) txtC = "Legumbres";
                                else if (t_comida == 3) txtC = "Conservas";
                                else if (t_comida == 4) txtC = "Lacteos";
                                else txtC = "Alimento";

                                snprintf(fila, sizeof(fila), "%-12s | %-13s - %.2f kg      | %s\n", 
                                        "COMIDA", txtC, k_comida, fecha.c_str());
                            }
                            break;

                        case 2: // ROPA
                            snprintf(fila, sizeof(fila), "%-12s | Ropa variada - %.2f kg      | %s\n", 
                                    "ROPA", k_ropa, fecha.c_str());
                            break;

                        case 3: // DINERO
                            snprintf(fila, sizeof(fila), "%-12s | Importe: %.2f EUR           | %s\n", 
                                    "DINERO", importe, fecha.c_str());
                            break;

                        default:
                            snprintf(fila, sizeof(fila), "%-12s | Sin detalles (Tipo: %d)     | %s\n", 
                                    "OTROS", tipo, fecha.c_str());
                            break;
                    }
                    tabla += fila;
                }

                if (count == 0) {
                    tabla += "No se han encontrado registros en tu historial de donaciones.\n";
                }
                tabla += "--------------------------------------------------------------------\n";
                sqlite3_finalize(stmt);
                
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                registrarLog("ÉXITO: Historial completo enviado (" + to_string(count) + " filas) al Donante ID: " + to_string(idDonanteReal));
            } else {
                sprintf(fila, "[ERROR] Error SQL: %s\n", sqlite3_errmsg(db));
                tabla += fila;
                paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
            }

            strncpy(paqueteRespuesta.mensajeRespuesta, tabla.c_str(), sizeof(paqueteRespuesta.mensajeRespuesta) - 1);
            paqueteRespuesta.mensajeRespuesta[sizeof(paqueteRespuesta.mensajeRespuesta) - 1] = '\0';
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
            // Dejas el case OP_VER_EVENTOS_DISPONIBLES intacto arriba... y añades:

case OP_VER_EVENTOS_DISPONIBLES:
    {
        // Comprobamos si el usuario que hace la petición es un Beneficiario (Rol 3 en tu enum TipoUsuario)
        if (paqueteRecibido.tipoUsuario == 3) 
        {
            registrarLog("Beneficiario ID " + to_string(paqueteRecibido.idUsuario) + " solicita ver talleres disponibles.");
            
            // 🌟 AQUÍ LLAMAS A LA FUNCIÓN PASÁNDOLE EL PAQUETE DE RESPUETA
            verTalleresProximos(db, paqueteRespuesta);
        } 
        else 
        {
            // Aquí se queda intacto tu código original para los Voluntarios (Módulo de voluntariado)
            registrarLog("Usuario ID " + to_string(paqueteRecibido.idUsuario) + " solicita ver eventos disponibles.");

            sqlite3_stmt *stmt;
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

            
case OP_ACTUALIZAR_PERFIL: // Debe llamarse igual que en el cliente
                    registrarLog("[BD] Petición de actualización económica para beneficiario ID: " + to_string(paqueteRecibido.idUsuario));
                    {
                        // Reconstruimos el objeto Beneficiario extrayendo los datos del paquete de red
                        GestionONG::Beneficiario bLocal;
                        bLocal.setIngresos(paqueteRecibido.economia.sueldo);
                        bLocal.setNumAdultos(paqueteRecibido.economia.adultos);
                        bLocal.setNumNinos(paqueteRecibido.economia.ninos);
                        bLocal.setGastos(paqueteRecibido.economia.otros_gastos);

                        // Invocamos la función de SQLite local del servidor
                        int exito = actualizarDatosBeneficiario(db, paqueteRecibido.idUsuario, bLocal);

                        // Respondemos al cliente el resultado
                        memset(&paqueteRespuesta, 0, sizeof(PaqueteRed));
                        if (exito) {
                            paqueteRespuesta.tipoOperacion = OP_RESPUESTA_OK;
                            strcpy(paqueteRespuesta.mensajeRespuesta, "Datos actualizados correctamente en el servidor.");
                            registrarLog("[OK] Datos guardados con éxito en la Base de Datos.");
                        } else {
                            paqueteRespuesta.tipoOperacion = OP_RESPUESTA_ERROR;
                            strcpy(paqueteRespuesta.mensajeRespuesta, "Error al escribir en la Base de Datos del servidor.");
                            registrarLog("[ERROR] Falló la escritura SQLite del beneficiario.");
                        }
                    }
                    break;

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
           if (paqueteRecibido.tipoOperacion != OP_CONSULTAR_EVENTOS) {
                send(clientSocket, (char *)&paqueteRespuesta, sizeof(PaqueteRed), 0);
            }
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
int tieneChoqueDeFechas(sqlite3 *db, int id_voluntario, int id_evento_nuevo){
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


    void verProximoRepartoComida(sqlite3* db, PaqueteRed& paqueteOut) {
    paqueteOut.tipoOperacion = OP_RESPUESTA_OK;
    
    string mensaje = "\n=========================================================\n";
    mensaje += "            HORARIOS DE REPARTO DE COMIDA (SEMANAL)       \n";
    mensaje += "=========================================================\n";
    mensaje += " > Cada Martes y Jueves de 09:00 a 13:30.\n";
    mensaje += " > Lugar: Almacen Central de la ONG (Calle Solidaridad n4).\n";
    mensaje += " * Recuerda traer tu tarjeta de beneficiario original.\n";
    mensaje += "=========================================================\n";

    snprintf(paqueteOut.mensajeRespuesta, sizeof(paqueteOut.mensajeRespuesta), "%s", mensaje.c_str());
}

void verProximoRepartoRopa(sqlite3* db, PaqueteRed& paqueteIn, PaqueteRed& paqueteOut) {
    sqlite3_stmt *stmt;
    // Buscamos cuántos adultos y niños tiene el beneficiario en la BD
    const char *sql = "SELECT num_adultos, num_nino FROM Beneficiario WHERE id_beneficiario = ?;";
    
    paqueteOut.tipoOperacion = OP_RESPUESTA_OK;
    int adultos = 0, ninos = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, paqueteIn.idUsuario);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            adultos = sqlite3_column_int(stmt, 0);
            ninos = sqlite3_column_int(stmt, 1);
        }
        sqlite3_finalize(stmt);
    } else {
        // Si no se encuentra o hay un error, usamos los datos que venían en la sesión por si acaso
        adultos = paqueteIn.economia.adultos;
        ninos = paqueteIn.economia.ninos;
    }

    // Baremo de ropa personalizado
    int camNinos = ninos * 3;
    int panNinos = ninos * 2;
    int sudNinos = ninos * 1;
    int camAdultos = adultos * 2;
    int panAdultos = adultos * 1;

    string mensaje = "\n=========================================================\n";
    mensaje += "            HORARIOS Y PREVISION DE ENTREGA DE ROPA       \n";
    mensaje += "=========================================================\n";
    mensaje += " > Primer Lunes de cada mes de 16:00 a 19:00.\n";
    mensaje += "---------------------------------------------------------\n";
    
    char fila[256];
    if (ninos > 0) {
        snprintf(fila, sizeof(fila), "  -> NINOS/AS (%d): %d camisetas, %d pantalones, %d sudaderas.\n", ninos, camNinos, panNinos, sudNinos);
        mensaje += fila;
    }
    if (adultos > 0) {
        snprintf(fila, sizeof(fila), "  -> ADULTOS  (%d): %d camisetas, %d pantalones.\n", adultos, camAdultos, panAdultos);
        mensaje += fila;
    }
    mensaje += "=========================================================\n";

    snprintf(paqueteOut.mensajeRespuesta, sizeof(paqueteOut.mensajeRespuesta), "%s", mensaje.c_str());
}

void verTalleresProximos(sqlite3* db, PaqueteRed& paqueteOut) {
    sqlite3_stmt* stmt;
    // Query para obtener los talleres programados
    const char* sql = "SELECT id_taller, nombre_taller, fecha, cupo_maximo FROM Taller;";
    
    // Inicializamos el paquete de salida por defecto con un mensaje de error
    paqueteOut.tipoOperacion = OP_RESPUESTA_ERROR;
    strcpy(paqueteOut.mensajeRespuesta, "[!] No hay talleres disponibles o ocurrió un error en el servidor.");

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        string tabla = "\n=========================================================\n";
        tabla += "                TALLERES Y EVENTOS DISPONIBLES            \n";
        tabla += "=========================================================\n";
        tabla += " ID   | Nombre del Taller              | Fecha      | Cupo \n";
        tabla += "---------------------------------------------------------\n";

        char fila[256];
        bool f_encontradas = false;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            f_encontradas = true;
            int id = sqlite3_column_int(stmt, 0);
            const char* nombre = (const char*)sqlite3_column_text(stmt, 1);
            const char* fecha = (const char*)sqlite3_column_text(stmt, 2);
            int cupo = sqlite3_column_int(stmt, 3);

            // Formateamos la fila alineada en columnas limpias
            snprintf(fila, sizeof(fila), " %-4d | %-30s | %-10s | %-4d \n", id, nombre, fecha, cupo);
            tabla += fila;
        }
        tabla += "=========================================================\n";
        sqlite3_finalize(stmt);

        if (f_encontradas) {
            paqueteOut.tipoOperacion = OP_RESPUESTA_OK;
            // Copiamos la tabla generada en el buffer del paquete de red de forma segura
            snprintf(paqueteOut.mensajeRespuesta, sizeof(paqueteOut.mensajeRespuesta), "%s", tabla.c_str());
        }
    } else {
        registrarLog("ERROR SQL: Fallo al preparar la consulta de verTalleresProximos.");
    }
}
    // ============================================================================
    // FUNCIONES AUXILIARES MATEMÁTICAS Y DE BASE DE DATOS
    // ============================================================================

    float calcularAyudaDinero(GestionONG::Beneficiario b) {
        float renta = b.getIngresos() - b.getGastos();
        return abs(int(renta)) + 50.0f; 
    }

    void mostrarAyudaComida(GestionONG::Beneficiario b) {
        float totalArrozPasta = (b.getNumAdultos() * 1.0f) + (b.getNumNinos() * 0.75f);
        float totalLegumbres = (b.getNumAdultos() + b.getNumNinos()) * 0.5f;
        float totalLeche = (b.getNumAdultos() * 2.0f) + (b.getNumNinos() * 4.0f);
        int totalConservas = (b.getNumAdultos() * 3) + (b.getNumNinos() * 2);

        printf("\n[ALIMENTACION SEMANAL]");  
        printf("\n > Arroz/Pasta:        %.2f kg", totalArrozPasta);
        printf("\n > Legumbres:          %.2f kg", totalLegumbres);
        printf("\n > Leche:              %.0f litros", totalLeche);
        printf("\n > Conservas:          %d latas\n", totalConservas);
    }

    void mostrarAyudaRopa(GestionONG::Beneficiario b) {
        int camNinos = b.getNumNinos() * 3;
        int panNinos = b.getNumNinos() * 2;
        int sudNinos = b.getNumNinos() * 1;
        int camAdultos = b.getNumAdultos() * 2;
        int panAdultos = b.getNumAdultos() * 1;

        printf("\n[VESTIMENTA SEMESTRAL]"); 
        if (b.getNumNinos() > 0) {
            printf("\n > NINOS/AS: %d camisetas, %d pantalones, %d sudaderas", camNinos, panNinos, sudNinos);
        }
        if (b.getNumAdultos() > 0) {
            printf("\n > ADULTOS: %d camisetas, %d pantalones", camAdultos, panAdultos);
        }
        printf("\n");
    }

    int actualizarDatosBeneficiario(sqlite3 *db, int id_beneficiario,GestionONG:: Beneficiario b) {
        sqlite3_stmt *stmt;
        const char *sql = "UPDATE Beneficiario SET ingresos = ?, gastos = ?, num_adultos = ?, num_nino = ? WHERE id_usuario = ?;";
        int rc = 0;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_double(stmt, 1, b.getIngresos());
            sqlite3_bind_double(stmt, 2, b.getGastos());
            sqlite3_bind_int(stmt, 3, b.getNumAdultos());
            sqlite3_bind_int(stmt, 4, b.getNumNinos());
            sqlite3_bind_int(stmt, 5, id_beneficiario);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                rc = 1;
            }
            sqlite3_finalize(stmt);
        }
        return rc;
    }
