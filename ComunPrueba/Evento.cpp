#include "Clases.h"
#include <iostream>
#include <cstdio> // Para sprintf

using namespace std;
using namespace GestionONG;

// Constructor de Evento (Solo si no lo habías definido en otro .cpp)
Evento::Evento(int id, Fecha ini, Fecha fin, TipoEvento t, string desc, int lim, Material mat)
    : id_evento(id), fecha_inicio(ini), fecha_fin(fin), tipoEvento(t), 
      descripcion(desc), lim_voluntarios(lim), material(mat) {}

// MÉTODO DE INSERCIÓN EN BD
int Evento::insertarEvento(sqlite3 *db) {
    char sql[1000];
    char *error = 0;
    
    // 1. Preparamos las fechas accediendo directamente a los atributos de 'this'
    char f_ini[20], f_fin[20];
    sprintf(f_ini, "%04d-%02d-%02d %02d:%02d", 
            this->fecha_inicio.anyo, this->fecha_inicio.mes, this->fecha_inicio.dia, 
            this->fecha_inicio.hora, this->fecha_inicio.minutos);
            
    sprintf(f_fin, "%04d-%02d-%02d %02d:%02d", 
            this->fecha_fin.anyo, this->fecha_fin.mes, this->fecha_fin.dia, 
            this->fecha_fin.hora, this->fecha_fin.minutos);

    // 2. Construimos el SQL. Usamos .c_str() para el std::string descripcion
    sprintf(sql, "INSERT INTO Evento (descripcion, fecha_ini, fecha_fin, lim_voluntarios, material, tipo) "
                 "VALUES ('%s', '%s', '%s', %d, '%d', '%d');",
            this->descripcion.c_str(), 
            f_ini, 
            f_fin, 
            this->lim_voluntarios, 
            (int)this->material, 
            (int)this->tipoEvento);

    // 3. Ejecutamos
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        cout << "\n[!] Error SQL al insertar evento: " << error << endl;
        sqlite3_free(error);
        return -1;
    }

    return 0; // Todo OK
}


// ============================================================================
// COMPROBACIONES Y CONSULTAS DE EVENTOS
// ============================================================================

int Evento::tieneChoqueDeFechas(sqlite3 *db, int id_voluntario, int id_evento_nuevo) {
    sqlite3_stmt *stmt;
    string fecha_objetivo = "";
    int choque = 0;

    // A. Obtener la fecha del evento al que se quiere apuntar
    const char *sql_f = "SELECT date(fecha_ini) FROM Evento WHERE id_evento = ?;";
    if (sqlite3_prepare_v2(db, sql_f, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_evento_nuevo);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            fecha_objetivo = (const char*)sqlite3_column_text(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // B. Buscar si el voluntario ya tiene algo ese día
    const char *sql_check = 
        "SELECT COUNT(*) FROM ("
        "  SELECT date(e.fecha_ini) as fecha FROM Participaciones p "
        "  JOIN Evento e ON p.id_evento = e.id_evento WHERE p.id_voluntario = ? "
        "  UNION ALL "
        "  SELECT date(t.fecha) as fecha FROM Impartir i "
        "  JOIN Taller t ON i.id_taller = t.id_taller WHERE i.id_voluntario = ?"
        ") WHERE fecha = ?;";

    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_voluntario);
        sqlite3_bind_int(stmt, 2, id_voluntario);
        sqlite3_bind_text(stmt, 3, fecha_objetivo.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            choque = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    return (choque > 0); 
}

int Evento::estaEventoLleno(sqlite3 *db, int id_e) {
    sqlite3_stmt *stmt;
    int lleno = 0;

    const char *sql = "SELECT E.lim_voluntarios, (SELECT COUNT(*) FROM Participaciones WHERE id_evento = ?) "
                      "FROM Evento E WHERE E.id_evento = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_e);
        sqlite3_bind_int(stmt, 2, id_e);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int cupo_maximo = sqlite3_column_int(stmt, 0);
            int ocupados = sqlite3_column_int(stmt, 1);

            if (ocupados >= cupo_maximo) {
                lleno = 1; 
            }
        }
        sqlite3_finalize(stmt);
    } else {
        cout << "[!] Error al comprobar el cupo del evento: " << sqlite3_errmsg(db) << endl;
    }

    return lleno;
}