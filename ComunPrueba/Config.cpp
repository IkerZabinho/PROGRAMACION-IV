#include "Clases.h"
#include <iostream>
#include <cstdio>  // Para FILE, fopen, fprintf, fclose
#include <cstring> // Para strtok, strcmp, strcpy

using namespace std;
using namespace GestionONG;

// 1. CARGAR CONFIGURACIÓN
int Config::cargar_configuracion(const char *filename, Config *conf) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) return 0;

    char linea[256];
    while (fgets(linea, sizeof(linea), file)) {
        // Eliminar el salto de línea al final si existe para que no ensucie los datos
        linea[strcspn(linea, "\r\n")] = 0;

        if (linea[0] == '#' || linea[0] == '\0') continue;

        char *clave = strtok(linea, "=");
        char *valor = strtok(NULL, ""); // Lee el resto de la línea después del '='

        if (clave && valor) {
            // Quitamos posibles espacios en blanco usando asignación directa a std::string
            if (strcmp(clave, "ADMIN_USER") == 0) conf->admin_user = valor;
            else if (strcmp(clave, "ADMIN_PASS") == 0) conf->admin_pass = valor;
            else if (strcmp(clave, "DB_PATH") == 0) conf->db_path = valor;
            else if (strcmp(clave, "REPORT_NAME") == 0) conf->report_name = valor;
        }
    }

    fclose(file);
    return 1;
}
// 2. GENERAR REPORTE RESUMEN
void Config::generarReporteResumen(sqlite3 *db, const char *nombreArchivo) {
    FILE *f = fopen(nombreArchivo, "w");
    if (f == NULL) return;

    fprintf(f, "=== RESUMEN DE LA ONG ===\n");
    
    // 1. Usuarios
    fprintf(f, "\n[USUARIOS]\n");
    sqlite3_exec(db, "SELECT nombre_usuario, tipo FROM Usuarios;", 
                 Config::callback_escribir_fichero, f, 0);

    // 2. Donaciones de Dinero
    fprintf(f, "\n[DONACIONES DINERO]\n");
    sqlite3_exec(db, "SELECT id_dinero, cantidad FROM Dinero;", 
                 Config::callback_escribir_fichero, f, 0);

    // 3. Ropa
    fprintf(f, "\n[DONACIONES  ROPA]\n");
    sqlite3_exec(db, "SELECT id_ropa, kilos FROM Ropa;", 
                 Config::callback_escribir_fichero, f, 0);
                 
    // 4. Comida
    fprintf(f, "\n[DONACIONES  COMIDA]\n");
    sqlite3_exec(db, "SELECT id_comida, tipo_comida, kilos FROM Comida;", 
                 Config::callback_escribir_fichero, f, 0);

    fclose(f);
}

// 3. CALLBACK AUXILIAR (Obligatorio usar el prefijo Config::)
int Config::callback_escribir_fichero(void *data, int argc, char **argv, char **azColName) {
    FILE *f = (FILE *)data;
    for (int i = 0; i < argc; i++) {
        fprintf(f, "%s: %s | ", azColName[i], argv[i] ? argv[i] : "N/A");
    }
    fprintf(f, "\n");
    return 0;
}