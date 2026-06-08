// MotorBaseDatos.cpp
#include "MotorBaseDatos.h"
#include "GestionLogs.h"
#include <cstring>
#include <string>

using namespace std;

bool autenticarUsuarioSQL(sqlite3* db, PaqueteRed& paqueteIn, PaqueteRed& paqueteOut) {
    sqlite3_stmt* stmt;
    // Buscamos el ID, la contraseña y el tipo (rol) en tu tabla de Usuarios
    string sql = "SELECT id_usuario, contrasena, tipo FROM Usuarios WHERE nombre_usuario = ?;";
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        registrarLog("ERROR SQL: No se pudo preparar la consulta de login.");
        strcpy(paqueteOut.mensajeRespuesta, "[ERROR] Error interno en el servidor.");
        return false;
    }
    
    // Enlazamos el usuario enviado por el cliente
    sqlite3_bind_text(stmt, 1, paqueteIn.perfil.usuario, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int idDb = sqlite3_column_int(stmt, 0);
        string passDb = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int tipoDb = sqlite3_column_int(stmt, 2);
        
        // Verificamos si la contraseña coincide
        if (passDb == string(paqueteIn.perfil.contrasena)) {
            paqueteOut.tipoOperacion = OP_RESPUESTA_OK;
            paqueteOut.idUsuario = idDb;
            paqueteOut.tipoUsuario = tipoDb;
            strcpy(paqueteOut.mensajeRespuesta, "¡Autenticacion completada con exito!");
            
            // --- REQUERIMIENTO 2: CARGA INMEDIATA DE CACHÉ PARA BENEFICIARIOS ---
            // --- REQUERIMIENTO 2: CARGA INMEDIATA DE CACHÉ PARA BENEFICIARIOS ---
            if (tipoDb == 3) { // Beneficiario
                // Pedimos las columnas esenciales que guarda tu base de datos
                string sqlBeneficiario = "SELECT ingresos, gastos, num_adultos, num_nino FROM Beneficiario WHERE id_usuario = ?;";
                sqlite3_stmt* stmtBeneficiario;
                
                if (sqlite3_prepare_v2(db, sqlBeneficiario.c_str(), -1, &stmtBeneficiario, NULL) == SQLITE_OK) {
                    sqlite3_bind_int(stmtBeneficiario, 1, idDb);
                    if (sqlite3_step(stmtBeneficiario) == SQLITE_ROW) {
                        // MAPEO TOTALMENTE SINCRONIZADO:
                        // 'ingresos' de la BD va a 'sueldo' y 'gastos' de la BD va a 'otros_gastos'
                        paqueteOut.economia.sueldo       = (float)sqlite3_column_double(stmtBeneficiario, 0); 
                        paqueteOut.economia.otros_gastos = (float)sqlite3_column_double(stmtBeneficiario, 1); 
                        paqueteOut.economia.adultos      = sqlite3_column_int(stmtBeneficiario, 2);          
                        paqueteOut.economia.ninos        = sqlite3_column_int(stmtBeneficiario, 3);          
                        
                        registrarLog("CACHE COMPLETADA: Datos reales cargados para Beneficiario ID: " + to_string(idDb));
                    } else {
                        registrarLog("ADVERTENCIA: El usuario existe en Usuarios pero no tiene fila en la tabla Beneficiario.");
                    }
                    sqlite3_finalize(stmtBeneficiario);
                }
            }
            
            sqlite3_finalize(stmt);
            return true;
        } else {
            paqueteOut.tipoOperacion = OP_RESPUESTA_ERROR;
            strcpy(paqueteOut.mensajeRespuesta, "[ERROR] Contrasena incorrecta.");
        }
    } else {
        paqueteOut.tipoOperacion = OP_RESPUESTA_ERROR;
        strcpy(paqueteOut.mensajeRespuesta, "[ERROR] El usuario no existe.");
    }
    
    sqlite3_finalize(stmt);
    return false;
}

bool registrarUsuarioSQL(sqlite3* db, PaqueteRed& paqueteIn, void* datosPerfilEspecifico) {
    sqlite3_stmt* stmtUser = NULL;
    sqlite3_stmt* stmtBen = NULL;
    int rc;

    // Forzamos el tipo de usuario correcto según la operación por si el cliente no lo rellenó bien
    int rolUsuario = paqueteIn.tipoUsuario;
    if (paqueteIn.tipoOperacion == OP_REGISTRO_VOLUNTARIO)   rolUsuario = 1;
    if (paqueteIn.tipoOperacion == OP_REGISTRO_DONANTE)      rolUsuario = 2;
    if (paqueteIn.tipoOperacion == OP_REGISTRO_BENEFICIARIO) rolUsuario = 3;

    // 1. INICIAMOS TRANSACCIÓN SEGURA
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        registrarLog("ERROR: No se pudo iniciar la transaccion de registro.");
        return false;
    }

    // 2. INSERTAR EN LA TABLA GENERAL DE USUARIOS
    string sqlUser = "INSERT INTO Usuarios (nombre_usuario, contrasena, tipo) VALUES (?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sqlUser.c_str(), -1, &stmtUser, NULL);
    if (rc != SQLITE_OK) {
        registrarLog("ERROR SQL: Fallo al preparar INSERT en Usuarios.");
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        return false;
    }

    // Enlazamos los datos comunes
    sqlite3_bind_text(stmtUser, 1, paqueteIn.perfil.usuario, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmtUser, 2, paqueteIn.perfil.contrasena, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmtUser, 3, rolUsuario); 

    rc = sqlite3_step(stmtUser);
    if (rc != SQLITE_DONE) {
        registrarLog("ERROR SQL: No se pudo insertar el usuario (posible usuario duplicado en la base de datos).");
        sqlite3_finalize(stmtUser);
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        return false;
    }

    // Recuperamos el ID autoincremental que SQLite acaba de asignar
    sqlite3_int64 nuevoIdUsuario = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmtUser);

    // 3. SI ES BENEFICIARIO (ROL 3), INSERTAMOS OBLIGATORIAMENTE EN LA TABLA BENEFICIARIO
    if (rolUsuario == 3) {
        string sqlBen = "INSERT INTO Beneficiario (id_beneficiario, ingresos, gastos, num_adultos, num_nino) VALUES (?, ?, ?, ?, ?);";
        rc = sqlite3_prepare_v2(db, sqlBen.c_str(), -1, &stmtBen, NULL);
        if (rc != SQLITE_OK) {
            registrarLog("ERROR SQL: Fallo al preparar INSERT en Beneficiario.");
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            return false;
        }

        // Mapeamos las columnas numéricas usando las variables reales del cliente
        sqlite3_bind_int64(stmtBen, 1, nuevoIdUsuario);
        sqlite3_bind_double(stmtBen, 2, (double)paqueteIn.economia.sueldo);
        sqlite3_bind_double(stmtBen, 3, (double)paqueteIn.economia.otros_gastos); 
        sqlite3_bind_int(stmtBen, 4, paqueteIn.economia.adultos);
        sqlite3_bind_int(stmtBen, 5, paqueteIn.economia.ninos);

        rc = sqlite3_step(stmtBen);
        if (rc != SQLITE_DONE) {
            registrarLog("ERROR SQL: Fallo al insertar datos economicos en Beneficiario.");
            sqlite3_finalize(stmtBen);
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            return false;
        }
        sqlite3_finalize(stmtBen);
    }

    // 4. CONFIRMAMOS CAMBIOS
    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    registrarLog("REGISTRO COMPLETADO CON ÉXITO: Generado Usuario ID: " + to_string(nuevoIdUsuario));
    return true;
}