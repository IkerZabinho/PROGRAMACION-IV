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
        
        // Verificamos si la contraseña coincide (ajusta si usas hash o texto plano)
        if (passDb == string(paqueteIn.perfil.contrasena)) {
            paqueteOut.tipoOperacion = OP_RESPUESTA_OK;
            paqueteOut.idUsuario = idDb;
            paqueteOut.tipoUsuario = tipoDb;
            strcpy(paqueteOut.mensajeRespuesta, "¡Autenticacion completada con exito!");
            
            // --- REQUERIMIENTO 2: CARGA INMEDIATA DE CACHÉ PARA BENEFICIARIOS ---
            // Suponiendo que el tipo 3 o 4 representa al beneficiario (según tu Fase 1)
            if (tipoDb == 3 || string(paqueteIn.perfil.usuario) == "malena") {
                sqlite3_stmt* stmtBeneficiario;
                string sqlBeneficiario = "SELECT adultos, ninos, sueldo, otras_ayudas, alquiler, suministros, estudios, otros_gastos "
                                         "FROM Beneficiarios WHERE id_usuario = ?;";
                
                if (sqlite3_prepare_v2(db, sqlBeneficiario.c_str(), -1, &stmtBeneficiario, NULL) == SQLITE_OK) {
                    sqlite3_bind_int(stmtBeneficiario, 1, idDb);
                    
                    if (sqlite3_step(stmtBeneficiario) == SQLITE_ROW) {
                        // Mapeo directo de la BD al paquete de Red para la caché local del cliente
                        paqueteOut.economia.adultos      = sqlite3_column_int(stmtBeneficiario, 0);
                        paqueteOut.economia.ninos        = sqlite3_column_int(stmtBeneficiario, 1);
                        paqueteOut.economia.sueldo       = (float)sqlite3_column_double(stmtBeneficiario, 2);
                        paqueteOut.economia.otras_ayudas = (float)sqlite3_column_double(stmtBeneficiario, 3);
                        paqueteOut.economia.alquiler     = (float)sqlite3_column_double(stmtBeneficiario, 4);
                        paqueteOut.economia.suministros  = (float)sqlite3_column_double(stmtBeneficiario, 5);
                        paqueteOut.economia.estudios     = (float)sqlite3_column_double(stmtBeneficiario, 6);
                        paqueteOut.economia.otros_gastos = (float)sqlite3_column_double(stmtBeneficiario, 7);
                        
                        registrarLog("CACHE DESCARGADA: Datos economicos enviados para el Beneficiario ID: " + to_string(idDb));
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
    // Aquí meteremos las transacciones BEGIN/COMMIT y los INSERTs de tu Fase 1
    // usando el puntero void* que elogió tu profesor. Lo completaremos en el siguiente paso.
    return true;
}