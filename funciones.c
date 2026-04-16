#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "funciones.h"
#include "estructuras.h"
#include "sqlite3.h"
#include <time.h>


int cargar_configuracion(const char *filename, Config *conf) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) return 0;

    char linea[256];
    while (fgets(linea, sizeof(linea), file)) {
        // Ignorar comentarios o líneas vacías
        if (linea[0] == '#' || linea[0] == '\n') continue;

        // Dividir la línea en CLAVE=VALOR
        char *clave = strtok(linea, "=");
        char *valor = strtok(NULL, "\n");

        if (clave && valor) {
            if (strcmp(clave, "ADMIN_USER") == 0) strcpy(conf->admin_user, valor);
            else if (strcmp(clave, "ADMIN_PASS") == 0) strcpy(conf->admin_pass, valor);
            else if (strcmp(clave, "DB_PATH") == 0) strcpy(conf->db_path, valor);
            else if (strcmp(clave, "REPORT_NAME") == 0) strcpy(conf->report_name, valor);
        }
    }

    fclose(file);
    return 1;
}
void generarReporteResumen(sqlite3 *db, const char *nombreArchivo) {
    FILE *f = fopen(nombreArchivo, "w");
    if (f == NULL) return;

    fprintf(f, "=== RESUMEN DE LA ONG ===\n");
    
    // 1. Usuarios
    fprintf(f, "\n[USUARIOS]\n");
    sqlite3_exec(db, "SELECT nombre_usuario, tipo FROM Usuarios;", 
                 callback_escribir_fichero, f, 0);

    // 2. Donaciones de Dinero
    fprintf(f, "\n[DONACIONES DINERO]\n");
    sqlite3_exec(db, "SELECT id_dinero, cantidad FROM Dinero;", 
                 callback_escribir_fichero, f, 0);

    // 3. Ropa
    fprintf(f, "\n[DONACIONES  ROPA]\n");
    sqlite3_exec(db, "SELECT id_ropa, kilos FROM Ropa;", 
                 callback_escribir_fichero, f, 0);
    // 4.Comida
    fprintf(f, "\n[DONACIONES  COMIDA]\n");
    sqlite3_exec(db, "SELECT id_comida, tipo_comida,kilos FROM Comida;", 
                 callback_escribir_fichero, f, 0);


    fclose(f);
}

// Esta es una función auxiliar para que SQLite escriba en el archivo
int callback_escribir_fichero(void *data, int argc, char **argv, char **azColName) {
    FILE *f = (FILE *)data;
    for (int i = 0; i < argc; i++) {
        fprintf(f, "%s: %s | ", azColName[i], argv[i] ? argv[i] : "N/A");
    }
    fprintf(f, "\n");
    return 0;
}

// [GRUPO 1: BASE DE DATOS]

// Inserta usuario génerico y devuelve su ID generado (-1 si falla)
// Inserta un usuario y su perfil específico. Devuelve el ID del perfil o -1.
int insertarUsuario(sqlite3 *db, Usuario u, void *datosEspecificos) {
    char sql[500];
    char *error = 0;
    int id_usuario_gen = -1;
    int id_perfil_especifico = -1;

    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    // 1. Insertar en Usuarios (Usamos 'tipo' que es la columna real de tu DB)
    sprintf(sql, "INSERT INTO Usuarios (nombre, apellidos, nombre_usuario, contrasena, tipo) "
                 "VALUES ('%s', '%s', '%s', '%s', %d);",
            u.nombre, u.apellidos, u.nombre_usuario, u.contrasena, u.tipoUsuario);

    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("[ERROR SQL Usuarios] %s\n", error);
        sqlite3_free(error);
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        return -1;
    }

    id_usuario_gen = (int)sqlite3_last_insert_rowid(db);

    // 2. Insertar en tablas específicas según el rol
    if (u.tipoUsuario == 2) { // BENEFICIARIO
        Beneficiario *b = (Beneficiario *)datosEspecificos;
        b->id_usuario = id_usuario_gen;
        id_perfil_especifico = insertarDatosBeneficiario(db, *b);
    } 
    else if (u.tipoUsuario == 0) { // VOLUNTARIO
        Voluntario *v = (Voluntario *)datosEspecificos;
        v->id_usuario = id_usuario_gen;
        id_perfil_especifico = insertarDatosVoluntario(db, *v);
    } 
    else if (u.tipoUsuario == 1) { // DONANTE (Ajusta el número si tu enum es distinto)
        Donante *d = (Donante *)datosEspecificos;
        d->id_usuario = id_usuario_gen;
        id_perfil_especifico = insertarDatosDonante(db, *d);
        /*// Insertamos en la tabla Donantes para que el perfil exista
        sprintf(sql, "INSERT INTO Donantes (id_usuario) VALUES (%d);", id_usuario_gen);
        if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
            printf("[ERROR SQL Donantes] %s\n", error);
            sqlite3_free(error);
            id_perfil_especifico = -1; // Esto disparará el ROLLBACK abajo
        }*/

    if (id_perfil_especifico == -1) {
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        return -1;
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    return id_perfil_especifico;
}}

// Eliminar usuario
int eliminarUsuarioDB(sqlite3 *db, int id) {
    char sql[200];
    char *error = 0;
    
    sprintf(sql, "DELETE FROM Usuarios WHERE id_usuario = %d;", id);
    
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("Error SQL: %s\n", error);
        sqlite3_free(error);
        return -1; // Fallo
    }
    return 0; // Éxito
}

// Listar usuarios
void listarUsuarios(sqlite3 *db) {
    sqlite3_stmt *stmt;
    char *sql = "SELECT id_usuario, nombre, apellidos, nombre_usuario FROM Usuarios;";
   
    printf("\n--- LISTA ACTUAL DE USUARIOS ---\n");
    printf("%-10s %-20s %-20s %-20s\n", "ID", "NOMBRE", "APELLIDOS", "USUARIO");
    printf("-------------------------------\n");


    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char *nombre = sqlite3_column_text(stmt, 1);
            const unsigned char *apellidos = sqlite3_column_text(stmt, 2);
            const unsigned char *nombre_usuario = sqlite3_column_text(stmt, 3);

            printf("%-10d %-20s %-20s %-20s\n", id, nombre, apellidos, nombre_usuario);
        }
    }
    sqlite3_finalize(stmt);
    printf("-------------------------------\n");
}


// Insertar beneficiarios
// Inserta los datos específicos de un beneficiario. 
int insertarDatosBeneficiario(sqlite3 *db, Beneficiario b)
{
    char sql[400];
    char *error = 0;

    // Usamos los campos de la estructura b
    sprintf(sql, "INSERT INTO Beneficiario (id_usuario, ingresos, gastos, num_adultos, num_nino) "
                 "VALUES (%d, %.2f, %.2f, %d, %d);",
            b.id_usuario, b.ingresos, b.gastos, b.num_adultos, b.num_ninos);

    if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) {
        return (int)sqlite3_last_insert_rowid(db); 
    } else {
        printf("[ERROR SQL] No se pudo insertar datos de beneficiario: %s\n", error);
        sqlite3_free(error);
        return -1;
    }
}

// Insertar voluntarios
// Inserta los datos específicos de un voluntario.
int insertarDatosVoluntario(sqlite3 *db, Voluntario v)
{
    char sql[300];
    char *error = 0;

    // Usamos los campos de la estructura v
    sprintf(sql, "INSERT INTO Voluntarios (id_usuario) VALUES (%d);",
            v.id_usuario, v.rol);

    if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) {
        return (int)sqlite3_last_insert_rowid(db); 
    } else {
        printf("[ERROR SQL] No se pudo insertar datos de voluntario: %s\n", error);
        sqlite3_free(error);
        return -1;
    }
}
// Insertar donantes
// Inserta los datos específicos de un donante. 
int insertarDatosDonante(sqlite3 *db, Donante d)
{
    char sql[400];
    char *error = 0;

    // Usamos los campos de la estructura b
    sprintf(sql, "INSERT INTO Donantes (id_usuario) "
                 "VALUES (%d);",
            d.id_usuario);

    if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) {
        return (int)sqlite3_last_insert_rowid(db); 
    } else {
        printf("[ERROR SQL] No se pudo insertar datos de beneficiario: %s\n", error);
        sqlite3_free(error);
        return -1;
    }
}
// Insertar Evento
int insertarEvento(sqlite3 *db, Evento e) {
    char sql[1000];
    char *error = 0;
    
    // 1. Preparamos las fechas en el formato que SQLite entiende (AAAA-MM-DD HH:MM)
    // Usamos los datos de tu estructura Fecha (e.fecha_inicio y e.fecha_fin)
    char f_ini[20], f_fin[20];
    sprintf(f_ini, "%04d-%02d-%02d %02d:%02d", 
            e.fecha_inicio.anyo, e.fecha_inicio.mes, e.fecha_inicio.dia, 
            e.fecha_inicio.hora, e.fecha_inicio.minutos);
            
    sprintf(f_fin, "%04d-%02d-%02d %02d:%02d", 
            e.fecha_fin.anyo, e.fecha_fin.mes, e.fecha_fin.dia, 
            e.fecha_fin.hora, e.fecha_fin.minutos);

    // 2. Construimos el SQL. 
    // IMPORTANTE: Incluimos 'material' y 'tipo' porque tu DB los pide.
    // Usamos %d para material y tipo ya que son ENUMS (enteros).
    sprintf(sql, "INSERT INTO Evento (descripcion, fecha_ini, fecha_fin, lim_voluntarios, material, tipo) "
                 "VALUES ('%s', '%s', '%s', %d, '%d', '%d');",
            e.descripcion, 
            f_ini, 
            f_fin, 
            e.lim_voluntarios, 
            (int)e.material, 
            (int)e.tipoEvento);

    // 3. Ejecutamos
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("\n[!] Error SQL al insertar: %s\n", error);
        sqlite3_free(error);
        return -1;
    }

    return 0; // Todo OK
}

// Insertar donación de ropa
int insertarDonacionRopa(sqlite3 *db, Ropa r, int id_donante) {
    char sql[400];
    char *error = 0;

    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    // 1. Insertamos en Donaciones (id_donacion es AUTOINCREMENT, no se pone)
    // El '2' es el tipo para ROPA
    sprintf(sql, "INSERT INTO Donaciones (id_donante, tipo, fecha) VALUES (%d, 2, date('now'));", 
            id_donante);
    
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("Error en Donaciones: %s\n", error);
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        sqlite3_free(error);
        return -1;
    }

    // 2. Recuperamos el ID generado
    int id_padre = (int)sqlite3_last_insert_rowid(db);

    // 3. Insertamos en la tabla Ropa
    sprintf(sql, "INSERT INTO Ropa (id_donacion, kilos) VALUES (%d, %.2f);", 
            id_padre, r.kilos);

    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("Error en Ropa: %s\n", error);
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        sqlite3_free(error);
        return -1;
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    return 0;
}
// Insertar donación dinero
int insertarDonacionDinero(sqlite3 *db, Dinero d, int id_donante) {
    char sql[400];
    char *error = 0;

    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    // El '3' es el tipo para DINERO
    sprintf(sql, "INSERT INTO Donaciones (id_donante, tipo, fecha) VALUES (%d, 3, date('now'));", 
            id_donante);
    
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        return -1;
    }

    int id_padre = (int)sqlite3_last_insert_rowid(db);

    sprintf(sql, "INSERT INTO Dinero (id_donacion, cantidad) VALUES (%d, %.2f);", 
            id_padre, d.cantidad);
    
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        return -1;
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    return 0;
}
// Insertar donación comida
int insertarDonacionComidaDB(sqlite3 *db, Donacion d, Comida c) {
    char sql[400];
    char *error = 0;

    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    // El '1' es el tipo para COMIDA
    sprintf(sql, "INSERT INTO Donaciones (id_donante, tipo, fecha) VALUES (%d, 1, date('now'));", 
            d.id_usuario);

    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("[!] Error Donacion: %s\n", error);
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        sqlite3_free(error);
        return -1;
    }

    int id_padre = (int)sqlite3_last_insert_rowid(db);

    sprintf(sql, "INSERT INTO Comida (id_donacion, tipo_comida, kilos) VALUES (%d, %d, %.2f);",
            id_padre, (int)c.tipo_comida, c.kilos);

    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("[!] Error Comida: %s\n", error);
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        sqlite3_free(error);
        return -1;
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    return 0;
}

//[GRUPO 2: LÓGICA/CONSULTAS]
//Comprobar Login
int comprobarLogin(sqlite3 *db, char *user, char *pass, Usuario *u_sesion) {
    sqlite3_stmt *stmt;
    // Seleccionamos todos los campos necesarios para rellenar la struct Usuario
    char *sql = "SELECT id_usuario, nombre, apellidos, nombre_usuario, tipo FROM Usuarios WHERE nombre_usuario = ? AND contrasena = ?;";
    int encontrado = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, pass, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            // Rellenamos la estructura campo a campo
            u_sesion->id_usuario = sqlite3_column_int(stmt, 0);
            
            // Copiamos los strings de la DB a los arrays fijos de la struct
            strncpy(u_sesion->nombre, (const char*)sqlite3_column_text(stmt, 1), sizeof(u_sesion->nombre) - 1);
            strncpy(u_sesion->apellidos, (const char*)sqlite3_column_text(stmt, 2), sizeof(u_sesion->apellidos) - 1);
            strncpy(u_sesion->nombre_usuario, (const char*)sqlite3_column_text(stmt, 3), sizeof(u_sesion->nombre_usuario) - 1);
            
            u_sesion->tipoUsuario = (TipoUsuario)sqlite3_column_int(stmt, 4);
            
            encontrado = 1;
        }
    }
    sqlite3_finalize(stmt);
    return encontrado;
}

// Devuelve 1 si el voluntario ya tiene otro evento ese día
int tieneChoqueDeFechas(sqlite3 *db, int id_voluntario, int id_evento_nuevo) {
    sqlite3_stmt *stmt;
    char fecha_objetivo[20] = "";
    int choque = 0;

    // A. Obtener la fecha del evento al que se quiere apuntar
    const char *sql_f = "SELECT date(fecha_ini) FROM Evento WHERE id_evento = ?;";
    if (sqlite3_prepare_v2(db, sql_f, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_evento_nuevo);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            strcpy(fecha_objetivo, (const char*)sqlite3_column_text(stmt, 0));
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

    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_voluntario);
        sqlite3_bind_int(stmt, 2, id_voluntario);
        sqlite3_bind_text(stmt, 3, fecha_objetivo, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            choque = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    return (choque > 0); 
}
// Devuelve 1 si el evento ha llegado a su límite
int estaEventoLleno(sqlite3 *db, int id_e) {
    sqlite3_stmt *stmt;
    int lleno = 0;

    // La consulta obtiene el cupo máximo y cuántas participaciones hay ya registradas
    const char *sql = "SELECT E.lim_voluntarios, (SELECT COUNT(*) FROM Participaciones WHERE id_evento = ?) "
                      "FROM Evento E WHERE E.id_evento = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_e);
        sqlite3_bind_int(stmt, 2, id_e);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int cupo_maximo = sqlite3_column_int(stmt, 0);
            int ocupados = sqlite3_column_int(stmt, 1);

            if (ocupados >= cupo_maximo) {
                lleno = 1; // El evento está lleno
            }
        }
        sqlite3_finalize(stmt);
    } else {
        printf("[!] Error al comprobar el cupo del evento: %s\n", sqlite3_errmsg(db));
    }

    return lleno;
}

// Evaluar la ayuda que necesita el beneficiario
void evaluarBeneficiario(Beneficiario b) {
    float renta = b.ingresos - b.gastos;
   
    // Cálculos de umbrales mensuales
    float gastoComidaMensual = (b.num_adultos * 150.0f) + (b.num_ninos * 140.0f);
    float gastoRopaMensual = (b.num_adultos * 5.50f) + (b.num_ninos * 9.0f);
    float umbralTotal = gastoComidaMensual + gastoRopaMensual;


    printf("\n===========================================");
    printf("\n      RESULTADO DEL ANÁLISIS SOCIAL");
    printf("\n===========================================");


    // Lógica de Escenarios
    if (renta > umbralTotal)
    {
        // --- AUTOSUFICIENTE ---
        printf("\nESTADO: Evaluación Finalizada -> Autosuficiente");
        printf("\nTras analizar tu renta disponible, el sistema indica que puedes cubrir");
        printf("\nlas necesidades básicas de alimentación y vestimenta por tu cuenta.");
        printf("\nPriorizamos nuestros recursos para casos en situación de mayor urgencia.");
        printf("\n-------------------------------------------");
        printf("\nSi tu situación económica cambia, puedes solicitar una nueva evaluación.");
    }
    else if (renta >= gastoComidaMensual && renta <= umbralTotal)
    {
        // --- ESCENARIO A ---
        printf("\nESTADO: Evaluación Finalizada -> Escenario A");
        printf("\nTras analizar tu renta disponible, consideramos que cubres la alimentación");
        printf("\nbásica, por lo tanto, recibirás apoyo específico en vestimenta.");
        printf("\n-------------------------------------------");
        mostrarAyudaRopa(b);
    }
    else if (renta > 0 && renta < gastoComidaMensual) {
        // --- ESCENARIO B ---
        printf("\nESTADO: Evaluación Finalizada -> Escenario B");
        printf("\nTras analizar tu renta disponible, el sistema indica que necesitas apoyo");
        printf("\ntanto en alimentación semanal como en vestimenta semestral.");
        printf("\n-------------------------------------------");
        mostrarAyudaComida(b);
        mostrarAyudaRopa(b);
    }
    else {
        // --- ESCENARIO C ---
        printf("\nESTADO: Evaluación Finalizada -> Escenario C");
        printf("\nTras analizar tu renta disponible, el sistema detecta una situación de");
        printf("\nemergencia. Recibirás ayuda económica, alimentación y vestimenta.");
        printf("\n-------------------------------------------");
        float dinero = calcularAyudaDinero(b);
        printf("\n > AYUDA ECONÓMICA: %.2f euros/mes", dinero);
        mostrarAyudaComida(b);
        mostrarAyudaRopa(b);
    }
    printf("\n===========================================\n");
}

// Función para actualizar los datos de beneficiario
int actualizarDatosBeneficiario(sqlite3 *db, int id_perfil, Beneficiario b) {
    char sql[400];
    char *error = 0;


    // Usamos UPDATE para modificar la fila que ya existe
    sprintf(sql,
        "UPDATE Beneficiario SET ingresos = %.2f, gastos = %.2f, "
        "num_adultos = %d, num_nino = %d WHERE id_beneficiario = %d;",
        b.ingresos, b.gastos, b.num_adultos, b.num_ninos, id_perfil);


    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("Error al actualizar base de datos: %s\n", error);
        sqlite3_free(error);
        return 0;
    }
    evaluarBeneficiario(b);
    return 1;
}



//[GRUPO 3: INTERFAZ]

// Guardar condiciones de beneficiario
Beneficiario guardarCondicionesBeneficiario() {
    Beneficiario b = {0}; // Inicializamos a 0
    int correcto;
    float sueldos, ayudas, alquiler, suministros, material_escolar, estudios, otros;

    printf("\n--- DETALLES ECONÓMICOS DEL BENEFICIARIO ---\n");
    printf("\n* Responde a la pregunta y pulsa enter para continuar.\n");

    // 1. INTEGRANTES
    do {
        printf("\n> INTEGRANTES DE LA FAMILIA\n");
        printf("Número de adultos en casa: ");
        scanf("%d", &b.num_adultos);
        printf("Número de niños/as en casa: ");
        scanf("%d", &b.num_ninos);
        
        printf("  > ¿Deseas cambiar algún dato de los integrantes? (1: Sí / 0: No): ");
        scanf("%d", &correcto);
        while (getchar() != '\n'); // Limpia buffer

        if (correcto == 1) printf("[!] Reintentando integrantes...\n");
    } while (correcto != 0);

    // 2. INGRESOS
    do {
        printf("\n> INGRESOS\n");
        printf("Sueldo mensual total: ");
        scanf("%f", &sueldos);
        printf("Otras ayudas/pensiones: ");
        scanf("%f", &ayudas);
        
        printf("  > ¿Deseas cambiar algún dato de los ingresos? (1: Sí / 0: No): ");
        scanf("%d", &correcto);
        while (getchar() != '\n');

        if (correcto == 1) printf("[!] Reintentando ingresos...\n");
    } while (correcto != 0);

    // 3. GASTOS
    do {
        printf("\n> GASTOS\n");
        printf("Alquiler o hipoteca: ");
        scanf("%f", &alquiler);
        printf("Luz, agua y gas: ");
        scanf("%f", &suministros);
        printf("Material escolar: ");
        scanf("%f", &material_escolar);
        printf("Gastos en estudios: ");
        scanf("%f", &estudios);
        printf("Otros gastos: ");
        scanf("%f", &otros);

        printf("  > ¿Deseas cambiar algún dato de los gastos? (1: Sí / 0: No): ");
        scanf("%d", &correcto);
        while (getchar() != '\n');

        if (correcto == 1) printf("[!] Reintentando gastos...\n");
    } while (correcto != 0);

    // CALCULAMOS TOTALES Y GUARDAMOS EN LA ESTRUCTURA
    b.ingresos = sueldos + ayudas;
    b.gastos = alquiler + suministros + material_escolar + estudios + otros;

    // Llamamos a tu lógica de evaluación
    evaluarBeneficiario(b);

    return b;
}

// RegistrarUsuario
void registrarUsuario(sqlite3 *db) {
    Usuario u;
    char buffer[100];
    int rolElegido;
    void *datosE = NULL;
    Beneficiario b = {0};
    Voluntario v = {0};
    Donante d={0};

    printf("\n--- REGISTRO DE NUEVO USUARIO ---\n");
    printf("Elige tu rol:\n1. Voluntario\n2. Donante\n3. Beneficiario\nRol (1-3): ");
    
    if (scanf("%d", &rolElegido) != 1 || rolElegido < 1 || rolElegido > 3) {
        printf("Rol no válido.\n");
        while (getchar() != '\n'); 
        return;
    }
    
    u.tipoUsuario = (TipoUsuario)(rolElegido - 1);

    // Captura con arrays fijos (usando strcpy)
    printf("Nombre: ");
    scanf("%s", u.nombre); // Se guarda directo en el array de la struct

    printf("Apellidos: ");
    while (getchar() != '\n'); 
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    strncpy(u.apellidos, buffer, sizeof(u.apellidos) - 1);

    printf("Nombre de usuario: ");
    scanf("%s", u.nombre_usuario);

    printf("Contraseña: ");
    scanf("%s", u.contrasena);

    // LÓGICA DE PERFILES (Lo que añadimos para que sea "Rico en estructuras")
    if (u.tipoUsuario == 2) {
        b = guardarCondicionesBeneficiario(); // Esta es la que ya arreglamos antes
        datosE = &b;
    } 
    else if (u.tipoUsuario == 0) {
        //printf("Elija su rol (Profesor, Repartidor...): ");
        //scanf("%s", v.rol);
        datosE = &v;
    }
    else if(u.tipoUsuario==1){
        datosE= &d;
    }
    // Llamamos a la función que gestiona la DB
// ... después de capturar los datos ...
    int id_perfil_especifico = insertarUsuario(db, u, datosE);

    if (id_perfil_especifico != -1) {
        printf("\n[OK] Registro completado con éxito.\n");
    } else {
        printf("\n[!] ERROR: No se pudo registrar. Puede que el usuario ya exista.\n");
    }

    printf("\nPresiona ENTER para volver al menú principal...");
    fflush(stdin); // Limpia teclado
    getchar(); 
    getchar();
    // NOTA: Ya no hay frees porque no hay strdups. ¡Mucho más limpio!
}

// Iniciar Sesion
void iniciarSesion(sqlite3 *db) {
    char user[50], pass[50];
    Usuario sesion; // Estructura de "sesion" para guardar todo lo que sabemos del usuario

    printf("\n--- INICIAR SESIÓN ---\n");
    printf("\nUsuario: ");
    scanf("%s", user);
    printf("Contraseña: ");
    scanf("%s", pass);

    // Ahora comprobarLogin RELLENA la estructura 'sesion'
    if (comprobarLogin(db, user, pass, &sesion)) {
        
        // Usamos sesion.id_usuario (el de la tabla Usuarios) para buscar el perfil específico
        int id_perfil = buscarIdEspecifico(db, sesion.id_usuario, sesion.tipoUsuario);
        
        if (id_perfil != -1) {
            // ¡Mira qué bien queda ahora saludar por el nombre!
            printf("\n¡Bienvenido, %s %s!\n", 
                   sesion.nombre, sesion.apellidos, id_perfil);
            
            if (sesion.tipoUsuario == ADMINISTRADOR) {
                menuAdministrador(db);
            } else {
                // Pasamos el tipo (enum) y el id_perfil (FK)
                menuPrincipal(db, (int)sesion.tipoUsuario, id_perfil);
            }
        } else {
            printf("Error: No se encontró un perfil asociado a este usuario.\n");
        }
    } else {
        printf("Error: Usuario o contraseña incorrectos.\n");
    }
}

// Crear Evento
void crearEvento(sqlite3 *db) {
    Evento e; // Nuestra estructura única
    int temp_material;

    printf("\n--- CREAR EVENTO ---\n");
    printf("Descripción: ");
    scanf(" %[^\n]", e.descripcion); // Guardamos directo en el array de la struct

    printf("Tipo de evento (0 = Ropa, 1 = Comida): ");
    scanf("%d", &temp_material);
    e.material = (Material)temp_material;

    printf("Límite de voluntarios: ");
    scanf("%d", &e.lim_voluntarios);

    // Mantenemos tus validaciones potentes de fecha
    do {
        printf("Fecha inicio (DD/MM/AAAA HH:MM): ");
        if (leer_y_validar_fecha("", &e.fecha_inicio)) {
            break;
        }
        printf(" Error: La fecha de inicio debe ser válida y futura.\n");
    } while (1);

    do {
        printf("Fecha final (DD/MM/AAAA HH:MM): ");
        if (leer_y_validar_fecha("", &e.fecha_fin)) {
            // Comparamos usando los campos de la struct
            if (comparar_fechas(e.fecha_inicio, e.fecha_fin) == 1) {
                break;
            }
            printf(" Error: La fecha final debe ser posterior a la de inicio.\n");
        } else {
            printf(" Error: Formato incorrecto o fecha pasada.\n");
        }
    } while (1);

    // Llamamos a la lógica de base de datos
    if (insertarEvento(db, e) == 0) {
        printf("Evento registrado correctamente en el sistema.\n");
    } else {
        printf("Hubo un problema al guardar el evento.\n");
    }
}

// Apuntarse a un evento
void apuntarseEvento(sqlite3 *db, int id_voluntario) {
    Participacion p;
    p.id_voluntario = id_voluntario;
    sqlite3_stmt *stmt;
    char *error = 0;

    printf("\n--- APUNTARSE A UN EVENTO ---\n");
    printf("\nEventos disponibles en los que aún no participas:\n");

    // 1. Mostrar disponibles usando SENTENCIAS PREPARADAS (Sustituye al callback)
    const char *sql_list = 
        "SELECT id_evento, descripcion, fecha_ini, tipo, material FROM Evento "
        "WHERE date(fecha_ini) >= date('now') "
        "AND id_evento NOT IN (SELECT id_evento FROM Participaciones WHERE id_voluntario = ?);";

    if (sqlite3_prepare_v2(db, sql_list, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_voluntario);

        printf("\n%-4s | %-10s | %-10s | %-18s | %s\n", "ID", "TIPO", "MATERIAL", "FECHA", "DESCRIPCIÓN");
        printf("------------------------------------------------------------------------------------\n");

        int encontrados = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            encontrados = 1;
            int id = sqlite3_column_int(stmt, 0);
            const char *desc = (const char*)sqlite3_column_text(stmt, 1);
            const char *fecha = (const char*)sqlite3_column_text(stmt, 2);
            int tipo_int = sqlite3_column_int(stmt, 3);
            int mat_int = sqlite3_column_int(stmt, 4);

            // Traducción de enums a texto (lo que antes hacía el callback)
            char *txtTipo = (tipo_int == 0) ? "Recogida" : "Reparto";
            char *txtMat = (mat_int == 0) ? "Ropa" : "Comida";

            printf("%-4d | %-10s | %-10s | %-18s | %s\n", id, txtTipo, txtMat, fecha, desc);
        }
        
        if (!encontrados) {
            printf("[INFO] No hay eventos nuevos disponibles para ti en este momento.\n");
        }
        sqlite3_finalize(stmt);
    } else {
        printf("Error al consultar eventos: %s\n", sqlite3_errmsg(db));
    }

    printf("------------------------------------------------------------------------------------\n");
    printf("Introduce el ID del evento (0 para cancelar): ");
    if (scanf("%d", &p.id_evento) != 1) {
        while (getchar() != '\n'); // Limpiar si meten letras
        return;
    }
    if (p.id_evento <= 0) return;

    // 2. Comprobar Choque de Fechas (Se queda igual)
    if (tieneChoqueDeFechas(db, p.id_voluntario, p.id_evento)) {
        printf("\n¡ERROR! Ya tienes otro compromiso registrado para ese mismo día.\n");
        return;
    }

    // 3. Comprobar Cupo (Se queda igual)
    if (estaEventoLleno(db, p.id_evento)) {
        printf("\n¡ERROR! El evento ya tiene suficientes voluntarios.\n");
        return;
    }

    // 4. Inserción final usando la estructura Participacion
    const char *sql_ins = "INSERT INTO Participaciones (id_voluntario, id_evento) VALUES (?, ?);";
    
    if (sqlite3_prepare_v2(db, sql_ins, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, p.id_voluntario);
        sqlite3_bind_int(stmt, 2, p.id_evento);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("\n[OK] ¡Inscripción realizada con éxito! Gracias por tu colaboración.\n");
        } else {
            printf("\n[ERROR] No se pudo completar la inscripción: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
    } else {
        printf("\n[ERROR] Error de preparación: %s\n", sqlite3_errmsg(db));
    }
}

// Leer y validar la fecha
int leer_y_validar_fecha(const char *mensaje, Fecha *f)
{
    printf("%s", mensaje);
    if (scanf("%d/%d/%d %d:%d", &f->dia, &f->mes, &f->anyo, &f->hora, &f->minutos) != 5)
    {
        while (getchar() != '\n')
            ;
        return 0;
    }
    while (getchar() != '\n')
        ;
    int dias_max;
    if (f->mes < 1 || f->mes > 12 || f->hora < 0 || f->hora > 23 || f->minutos < 0 || f->minutos > 59)
        return 0;

    if (f->mes == 2)
    {
        if (es_bisiesto(f->anyo))
        {
            dias_max = 29;
        }
        else
        {
            dias_max = 28;
        }
    }

    else if (f->mes == 4 || f->mes == 6 || f->mes == 9 || f->mes == 11)
        dias_max = 30;
    else
        dias_max = 31;

    if (f->dia < 1 || f->dia > dias_max)
        return 0;
    struct tm temp = {0};
    temp.tm_mday = f->dia;
    temp.tm_mon = f->mes - 1;      // Los meses en C van de 0 a 11
    temp.tm_year = f->anyo - 1900; // El año cuenta desde 1900
    temp.tm_hour = f->hora;
    temp.tm_min = f->minutos;
    temp.tm_sec = 0;
    temp.tm_isdst = -1; // Para que el sistema ajuste horario de verano solo

    time_t tiempo_usuario = mktime(&temp); // Convertimos la fecha del usuario a segundos
    time_t tiempo_ahora;                   // Creamos la "caja" para guardar el tiempo
    tiempo_ahora = time(NULL);
    if (tiempo_usuario <= tiempo_ahora)
    {
        printf("Error: La fecha debe ser posterior a la actual.\n");
        return 0; // Fecha ya pasada
    }

    return 1; // Fecha valida
}

// Donar Ropa
void donarRopa(sqlite3 *db, int id_usuario) {
    Ropa r;
    int respuesta;

    printf("\n--- REALIZAR DONACIÓN DE ROPA ---\n");
    printf("Cantidad en kilogramos: ");
    
    // Validación de entrada
    if (scanf("%f", &r.kilos) != 1 || r.kilos <= 0) {
        printf("[!] Error: Debes introducir un peso válido mayor que 0.\n");
        while (getchar() != '\n');
        return;
    }

    // Limpiamos buffer tras el scanf
    while (getchar() != '\n');

    printf("¿Confirmas donar %.2f kg de ropa?\n0. No\n1. Sí\nSelección: ", r.kilos);
    scanf("%d", &respuesta);
    while (getchar() != '\n');

    if (respuesta == 1) {
        // Llamamos a la función que usa las estructuras
        if (insertarDonacionRopa(db, r, id_usuario) == 0) {
            printf("\n[ÉXITO] Tu donación de %.2f kg ha sido registrada.\n", r.kilos);
            // Aquí llamarías a mostrarProximaRecogida si la tienes
        } else {
            printf("\n[!] No se pudo completar la donación en la base de datos.\n");
        }
    } else {
        printf("Operación cancelada.\n");
    }
}

// Donar dinero
void donarDinero(sqlite3 *db, int id_donante) {
    Dinero d; // Usamos tu estructura
    int confirmar;

    printf("\n--- DONACIÓN DE DINERO ---\n");
    printf("Cantidad: ");
    if (scanf("%f", &d.cantidad) != 1 || d.cantidad <= 0) {
        printf("Error en cantidad.\n");
        while(getchar()!='\n'); return;
    }

    printf("¿Confirmar %.2f€? (1:Si / 0:No): ", d.cantidad);
    scanf("%d", &confirmar);

    if (confirmar == 1) {
        // En una sola línea llamamos a la base de datos
        if (insertarDonacionDinero(db, d, id_donante) == SQLITE_OK) {
            printf("[EXITO] Guardado.\n");
        }
    }
}

// Donar comida
void donarComida(sqlite3 *db, int id_donante) {
    Donacion d;
    Comida c;
    int respuesta, seleccion;
    const char *nombresCategorias[] = {"", "Carbohidratos", "Legumbres", "Conservas", "Lacteos", "Infantil"};

    printf("\n--- REALIZAR DONACION DE COMIDA ---\n");

    // 1. Selección de categoría (Validación original)
    do {
        printf("Seleccione el tipo de alimento:\n");
        for(int i=1; i<=5; i++) printf("%d. %s\n", i, nombresCategorias[i]);
        printf("Seleccion: ");
        
        if (scanf("%d", &seleccion) != 1) {
            printf("[!] Error: Introduce un numero.\n");
            while (getchar() != '\n'); 
            seleccion = -1;
            continue;
        }
        while (getchar() != '\n');

        if (seleccion < 1 || seleccion > 5)
            printf("[!] Opcion no valida, elige entre 1 y 5.\n");
    } while (seleccion < 1 || seleccion > 5);

    // Asignamos a la estructura Comida (usando tu Enum)
    c.tipo_comida = (TipoComida)seleccion;

    // 2. Validación de Kilogramos (Tu lógica robusta 3f3)
    printf("Cantidad en kilogramos: ");
    if (scanf("%f", &c.kilos) != 1) {
        printf("[!] Error: debes introducir un numero valido.\n");
        while (getchar() != '\n'); 
        return;
    }
    
    int ch = getchar();
    if (ch != '\n' && ch != EOF) {
        printf("[!] Error: Formato incorrecto. No incluyas letras en la cantidad.\n");
        while (getchar() != '\n'); 
        return;
    }

    if (c.kilos <= 0) { 
        printf("[!] Los kilogramos deben ser mayor que 0.\n"); 
        return; 
    }

    // 3. Confirmación y guardado
    printf("¿Confirmas donar %.2f kg de %s?\n0. No\n1. Si\nSeleccion: ",
           c.kilos, nombresCategorias[seleccion]);
    
    if (scanf("%d", &respuesta) != 1) {
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    if (respuesta == 1) {
        // Rellenamos la estructura Donacion antes de enviar
        d.id_usuario = id_donante;
        d.tipoDonacion = COMIDAD; // Asumo que tienes este enum para el tipo 2

        if (insertarDonacionComidaDB(db, d, c) == 0) {
            printf("\n[EXITO] Donacion registrada correctamente.\n");
            // Mantenemos tu función de feedback
            mostrarProximaRecogida(db, COMIDA); 
        }
    } else {
        printf("Operacion cancelada.\n");
    }
}

// Apuntarse beneficiario a taller
void apuntarseTaller(sqlite3 *db, int id_beneficiario) {
    sqlite3_stmt *stmt;
    int id_taller;

    printf("\n--- TALLERES DISPONIBLES ---\n");

    // 1. Mostrar talleres en los que el beneficiario NO está inscrito todavía
    // Nota: En tu DB la tabla es 'Taller' y las columnas son 'id_taller', 'tipo', 'descripcion'
    const char *sql_list = "SELECT id_taller, tipo, descripcion FROM Taller "
                           "WHERE id_taller NOT IN (SELECT id_taller FROM Asistencia WHERE id_beneficiario = ?);";

    if (sqlite3_prepare_v2(db, sql_list, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_beneficiario);
        
        int encontrados = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            encontrados++;
            printf("ID: %d | Tipo: %s | %s\n", 
                   sqlite3_column_int(stmt, 0), 
                   sqlite3_column_text(stmt, 1), 
                   sqlite3_column_text(stmt, 2));
        }
        sqlite3_finalize(stmt);

        if (encontrados == 0) {
            printf("No hay talleres nuevos disponibles para ti en este momento.\n");
            return;
        }
    } else {
        printf("[!] Error al consultar talleres: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\nIntroduce el ID del taller al que quieres asistir (0 para cancelar): ");
    if (scanf("%d", &id_taller) != 1 || id_taller <= 0) return;

    // 2. Inserción en la tabla Asistencia usando BIND (más seguro que sprintf)
    const char *sql_ins = "INSERT INTO Asistencia (id_beneficiario, id_taller) VALUES (?, ?);";

    if (sqlite3_prepare_v2(db, sql_ins, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_beneficiario);
        sqlite3_bind_int(stmt, 2, id_taller);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("\n[OK] Te has inscrito correctamente en el taller %d.\n", id_taller);
        } else {
            // Esto puede fallar si el ID del taller no existe (FK constraint)
            printf("\n[!] Error al inscribirse: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
    } else {
        printf("\n[!] Error de preparación: %s\n", sqlite3_errmsg(db));
    }
}

void crearTaller(sqlite3 *db) {
    sqlite3_stmt *stmt;
    Taller t; 
    int id_voluntario, opcion_tipo;
    char f_ini_str[20], f_fin_str[20];

    printf("\n--- CREAR NUEVO TALLER ---\n");

    // 1. LISTAR VOLUNTARIOS (RELLENADO)
    printf("\n--- VOLUNTARIOS DISPONIBLES ---\n");
    const char *sql_v = "SELECT v.id_voluntario, u.nombre FROM Voluntarios v "
                        "JOIN Usuarios u ON v.id_usuario = u.id_usuario;";
    
    int hay_voluntarios = 0;
    if (sqlite3_prepare_v2(db, sql_v, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            hay_voluntarios = 1;
            printf("ID: [%d] | Nombre: %s\n", 
                   sqlite3_column_int(stmt, 0), 
                   sqlite3_column_text(stmt, 1));
        }
        sqlite3_finalize(stmt);
    }

    if (!hay_voluntarios) {
        printf("[!] No hay voluntarios registrados. Registra uno antes de crear un taller.\n");
        return;
    }

    printf("\nIntroduce el ID del voluntario responsable: ");
    if (scanf("%d", &id_voluntario) != 1) {
        printf("[!] Error: ID no válido.\n");
        while (getchar() != '\n');
        return;
    }

    // 2. CAPTURA DE TIPO (ENUM)
    printf("\nSeleccione el tipo de taller:\n");
    printf("0. Cocina\n1. Aprendizaje\n2. Deportes\n: ");
    scanf("%d", &opcion_tipo);
    t.tipo = (TipoTaller)opcion_tipo; // Asignación al enum

    printf("Descripción: ");
    scanf(" %[^\n]", t.descripcion);

    // 3. VALIDACIÓN DE FECHAS (Estilo Evento)
    do {
        printf("Fecha inicio (DD/MM/AAAA HH:MM): ");
        if (leer_y_validar_fecha("", &t.fecha_ini)) break;
        printf(" [!] Error: Fecha inválida.\n");
    } while (1);

    do {
        printf("Fecha fin (DD/MM/AAAA HH:MM): ");
        if (leer_y_validar_fecha("", &t.fecha_fin)) break;
        printf(" [!] Error: Fecha inválida.\n");
    } while (1);

    // 4. FORMATEO DE FECHAS
    sprintf(f_ini_str, "%04d-%02d-%02d %02d:%02d", 
            t.fecha_ini.anyo, t.fecha_ini.mes, t.fecha_ini.dia, 
            t.fecha_ini.hora, t.fecha_ini.minutos);
            
    sprintf(f_fin_str, "%04d-%02d-%02d %02d:%02d", 
            t.fecha_fin.anyo, t.fecha_fin.mes, t.fecha_fin.dia, 
            t.fecha_fin.hora, t.fecha_fin.minutos);

    // 5. INSERTAR EN TABLA TALLER
    const char *sql_ins = "INSERT INTO Taller (tipo, fecha_ini, fecha_fin, descripcion, id_voluntario) VALUES (?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql_ins, -1, &stmt, 0) == SQLITE_OK) {
        // OPCIÓN A: Si tu base de datos guarda el número del enum
        sqlite3_bind_int(stmt, 1, (int)t.tipo); 
        
        // OPCIÓN B: Si tu base de datos guarda el TEXTO del taller
        // char *nombres[] = {"Carpinteria", "Cocina", "Costura", "Informatica"};
        // sqlite3_bind_text(stmt, 1, nombres[t.tipo], -1, SQLITE_STATIC);

        sqlite3_bind_text(stmt, 2, f_ini_str, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, f_fin_str, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, t.descripcion, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, id_voluntario);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("\n[OK] Taller creado con éxito.\n");
        } else {
            printf("\n[!] Error SQL: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
        // 1. Obtenemos el ID del taller que acabamos de crear
        int id_taller_recien_creado = (int)sqlite3_last_insert_rowid(db);

        // 2. Preparamos el insert para la tabla 'Impartir'
        sqlite3_stmt *stmt_rel;
        const char *sql_rel = "INSERT INTO Impartir (id_voluntario, id_taller) VALUES (?, ?);";

        if (sqlite3_prepare_v2(db, sql_rel, -1, &stmt_rel, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt_rel, 1, id_voluntario);
            sqlite3_bind_int(stmt_rel, 2, id_taller_recien_creado);

            if (sqlite3_step(stmt_rel) == SQLITE_DONE) {
                //printf("[INFO] Relación guardada en tabla 'Impartir'.\n");
            } else {
                printf("[!] Error al relacionar en 'Impartir': %s\n", sqlite3_errmsg(db));
            }
            sqlite3_finalize(stmt_rel);
        }
            }
}
/*
// Asignar profesor
// Función que usa el Administrador para asignar un voluntario
void asignarVoluntarioTaller(sqlite3 *db) {
    sqlite3_stmt *stmt;
    int id_voluntario, id_taller;

    printf("\n--- LISTA DE TALLERES DISPONIBLES ---\n");
    // 1. Consultamos la tabla Taller (ajusta los nombres de columna si varían)
    const char *sql_t = "SELECT id_taller, nombre, fecha FROM Taller;"; 
    
    int hay_talleres = 0;
    if (sqlite3_prepare_v2(db, sql_t, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            hay_talleres = 1;
            printf("ID Taller: %d | Nombre: %s | Fecha: %s\n", 
                   sqlite3_column_int(stmt, 0), 
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2));
        }
        sqlite3_finalize(stmt);
    }

    if (!hay_talleres) {
        printf("[!] No hay talleres registrados en la tabla 'Taller'.\n");
        return;
    }

    // 2. LISTAR VOLUNTARIOS (Como antes)
    printf("\n--- VOLUNTARIOS REGISTRADOS ---\n");
    const char *sql_v = "SELECT v.id_voluntario, u.nombre FROM Voluntarios v "
                        "JOIN Usuarios u ON v.id_usuario = u.id_usuario;";
    if (sqlite3_prepare_v2(db, sql_v, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("ID Voluntario: %d | Nombre: %s\n", 
                   sqlite3_column_int(stmt, 0), 
                   sqlite3_column_text(stmt, 1));
        }
        sqlite3_finalize(stmt);
    }

    printf("\nIntroduce el ID del Taller: ");
    scanf("%d", &id_taller);
    printf("Introduce el ID del Voluntario: ");
    scanf("%d", &id_voluntario);

    // 3. VERIFICAR SI EL VOLUNTARIO YA TIENE ALGO ESE DÍA
    // Obtenemos la fecha del taller seleccionado
    char fecha_taller[20] = "";
    const char *sql_f = "SELECT date(fecha) FROM Taller WHERE id_taller = ?;";
    if (sqlite3_prepare_v2(db, sql_f, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_taller);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            strcpy(fecha_taller, (const char*)sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    // Comprobamos si el voluntario ya está en Participaciones en esa fecha
    const char *sql_solape = 
        "SELECT COUNT(*) FROM Participaciones p "
        "JOIN Evento e ON p.id_evento = e.id_evento "
        "WHERE p.id_voluntario = ? AND date(e.fecha_ini) = ?;";

    int ocupado = 0;
    if (sqlite3_prepare_v2(db, sql_solape, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_voluntario);
        sqlite3_bind_text(stmt, 2, fecha_taller, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            ocupado = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (ocupado > 0) {
        printf("\n[ERROR] El voluntario ya tiene un evento el dia %s.\n", fecha_taller);
        return;
    }

    // 4. INSERTAR EN TU TABLA DE ASIGNACIÓN (Ajusta el nombre si no es Participaciones)
    // Si tienes una tabla específica para asignar voluntarios a talleres, cámbiala aquí
    const char *sql_ins = "INSERT INTO ParticipacionesTaller (id_voluntario, id_taller) VALUES (?, ?);";
    
    if (sqlite3_prepare_v2(db, sql_ins, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_voluntario);
        sqlite3_bind_int(stmt, 2, id_taller);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("\n[OK] Asignacion completada con exito.\n");
        } else {
            printf("\n[ERROR] No se pudo asignar: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
    }
}*/
// Borrar un evento
void borrarEvento(sqlite3 *db) {
    int id_borrar;
    char sql[200];
    char *error = 0;

    printf("\n--- ELIMINAR EVENTO ---\n");
    // Reutilizamos el listado para que el admin vea qué borrar
    listarEventos(db); 

    printf("\nIntroduce el ID del evento a eliminar (0 para cancelar): ");
    scanf("%d", &id_borrar);

    if (id_borrar <= 0) return;

    // Eliminamos de la tabla Evento
    // Nota: Si tienes FK con ON DELETE CASCADE, se borrarán también las participaciones
    sprintf(sql, "DELETE FROM Evento WHERE id_evento = %d;", id_borrar);

    if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) {
        printf("\n[OK] Evento eliminado correctamente.\n");
    } else {
        printf("\n[!] Error al borrar: %s\n", error);
        sqlite3_free(error);
    }
}
//listar eventos
void listarEventos(sqlite3 *db) {
    sqlite3_stmt *stmt;
    // Seleccionamos los datos clave de los eventos
    const char *sql = "SELECT id_evento, descripcion, fecha_ini, material, tipo FROM Evento ORDER BY fecha_ini ASC;";

    printf("\n%-5s | %-25s | %-18s | %-10s | %-10s\n", "ID", "DESCRIPCIÓN", "FECHA", "MATERIAL", "TIPO");
    printf("--------------------------------------------------------------------------------------\n");

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        int hay_eventos = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            hay_eventos = 1;
            int id = sqlite3_column_int(stmt, 0);
            const char *desc = (const char *)sqlite3_column_text(stmt, 1);
            const char *fecha = (const char *)sqlite3_column_text(stmt, 2);
            const char *mat_raw = (const char *)sqlite3_column_text(stmt, 3);
            const char *tipo_raw = (const char *)sqlite3_column_text(stmt, 4);

            // Convertimos los códigos de la DB a texto legible
            // Según tu DB: material '0'=Ropa, '1'=Comida | tipo '0'=Donación, '1'=Reparto
            char *mat_txt = (strcmp(mat_raw, "0") == 0) ? "Ropa" : "Comida";
            char *tipo_txt = (strcmp(tipo_raw, "0") == 0) ? "Donación" : "Reparto";

            printf("%-5d | %-25s | %-18s | %-10s | %-10s\n", 
                   id, 
                   desc ? desc : "Sin desc.", 
                   fecha ? fecha : "---", 
                   mat_txt, 
                   tipo_txt);
        }

        if (!hay_eventos) {
            printf("No hay eventos registrados en la base de datos.\n");
        }
        sqlite3_finalize(stmt);
    } else {
        printf("[!] Error al listar eventos: %s\n", sqlite3_errmsg(db));
    }
    printf("--------------------------------------------------------------------------------------\n");
}
// Dar baja usuario
void darBajaUsuario(sqlite3 *db) {
    int id_eliminar;
    int confirmar;

    printf("\n--- DAR DE BAJA USUARIO ---\n");
    listarUsuarios(db); // Ayudamos al admin a ver los IDs

    printf("Introduce ID del usuario para eliminar (0 para cancelar): ");
    if (scanf("%d", &id_eliminar) != 1 || id_eliminar <= 0) {
        printf("Operación cancelada.\n");
        return;
    }

    printf("¿Estás seguro de eliminar al usuario %d?\n0. No\n1. Sí\nSelección: ", id_eliminar);
    scanf("%d", &confirmar);

    if (confirmar == 1) {
        // Aquí ocurre la magia: llamamos a la lógica de DB
        if (eliminarUsuarioDB(db, id_eliminar) == 0) {
            printf("\n[ÉXITO] Usuario %d eliminado correctamente.\n", id_eliminar);
        } else {
            printf("\n[!] No se pudo eliminar el usuario.\n");
        }
    } else {
        printf("Operación cancelada.\n");
    }
}

//función para consultar eventos a los que está apuntado
void consultarMisEventos(sqlite3 *db, int id_voluntario) {
    sqlite3_stmt *stmt;
    char *sql_list = 
        "SELECT E.id_evento, E.descripcion, E.fecha_ini, E.tipo, E.material "
        "FROM Evento E "
        "JOIN Participaciones P ON E.id_evento = P.id_evento "
        "WHERE P.id_voluntario = ? AND E.fecha_ini >= datetime('now', 'localtime') "
        "ORDER BY E.fecha_ini ASC;";

    printf("\n--- CALENDARIO DE MIS EVENTOS ---\n");
    printf("%-5s | %-12s | %-10s | %-18s | %s\n", "ID", "TIPO", "MATERIAL", "FECHA", "DESCRIPCIÓN");
    printf("------------------------------------------------------------------------------------\n");

    if (sqlite3_prepare_v2(db, sql_list, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_voluntario);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char *desc = (const char*)sqlite3_column_text(stmt, 1);
            const char *fecha = (const char*)sqlite3_column_text(stmt, 2);
            int tipo_int = sqlite3_column_int(stmt, 3);
            int mat_int = sqlite3_column_int(stmt, 4);

            // Lógica que antes estaba en el callback, ahora aquí (o usando tus Enums)
            char *txtTipo = (tipo_int == 0) ? "Recogida" : "Reparto";
            char *txtMat = (mat_int == 0) ? "Ropa" : "Comida";

            printf("%-5d | %-12s | %-10s | %-18s | %s\n", id, txtTipo, txtMat, fecha, desc);
        }
        sqlite3_finalize(stmt);
    } else {
        printf("Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
    }
    printf("------------------------------------------------------------------------------------\n");

    // --- LÓGICA DE DESAPUNTARSE (Se queda igual porque es un DELETE) ---
    int opcion, id_borrar;
    printf("¿Quieres desapuntarte de alguno? (1: Sí / 0: No): ");
    if (scanf("%d", &opcion) != 1) {
        while (getchar() != '\n');
        return;
    }

    if (opcion == 1) {
        printf("Introduce el ID del evento: ");
        scanf("%d", &id_borrar);
        
        char sql_del[200];
        sprintf(sql_del, "DELETE FROM Participaciones WHERE id_voluntario = %d AND id_evento = %d;", 
                id_voluntario, id_borrar);
        
        char *error = 0;
        if (sqlite3_exec(db, sql_del, 0, 0, &error) == SQLITE_OK) {
            if (sqlite3_changes(db) > 0) printf("\n[OK] Te has desapuntado con éxito.\n");
            else printf("\n[!] No estabas inscrito en ese evento.\n");
        } else {
            printf("\nError: %s\n", error);
            sqlite3_free(error);
        }
    }
}

//función para ver eventos a los que he acudido
void consultarHistorialEventos(sqlite3 *db, int id_voluntario) {
    sqlite3_stmt *stmt;
    const char *sql = 
        "SELECT E.id_evento, E.descripcion, E.fecha_ini, E.tipo, E.material "
        "FROM Evento E "
        "JOIN Participaciones P ON E.id_evento = P.id_evento "
        "WHERE P.id_voluntario = ? AND date(E.fecha_ini) < date('now') "
        "ORDER BY E.fecha_ini DESC;";

    printf("\n--- HISTORIAL DE EVENTOS PASADOS ---\n");
    printf("%-5s | %-12s | %-10s | %-18s | %s\n", "ID", "TIPO", "MATERIAL", "FECHA", "DESCRIPCIÓN");
    printf("------------------------------------------------------------------------------------\n");

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_voluntario);

        int hay_eventos = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            hay_eventos = 1;
            int id = sqlite3_column_int(stmt, 0);
            const char *desc = (const char*)sqlite3_column_text(stmt, 1);
            const char *fecha = (const char*)sqlite3_column_text(stmt, 2);
            int tipo_int = sqlite3_column_int(stmt, 3);
            int mat_int = sqlite3_column_int(stmt, 4);

            // Traducimos los Enums a texto para el usuario
            char *txtTipo = (tipo_int == 0) ? "Recogida" : "Reparto";
            char *txtMat = (mat_int == 0) ? "Ropa" : "Comida";

            printf("%-5d | %-12s | %-10s | %-18s | %s\n", id, txtTipo, txtMat, fecha, desc);
        }
        
        if (!hay_eventos) {
            printf("\n[INFO] Aún no has participado en eventos anteriores.\n");
        }
        sqlite3_finalize(stmt);
    } else {
        printf("Error SQL: %s\n", sqlite3_errmsg(db));
    }

    printf("------------------------------------------------------------------------------------\n");
    printf("Presione Enter para volver...");
    while (getchar() != '\n');
    getchar();
}

// Listar donaciones
void listarDonaciones(sqlite3 *db, int id_donante) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT d.tipo, r.kilos, c.tipo_comida, c.kilos, din.cantidad, d.fecha "
        "FROM Donaciones d "
        "LEFT JOIN Ropa r ON d.id_donacion = r.id_donacion "
        "LEFT JOIN Comida c ON d.id_donacion = c.id_donacion "
        "LEFT JOIN Dinero din ON d.id_donacion = din.id_donacion "
        "WHERE d.id_donante = ? ORDER BY d.id_donacion DESC;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Error: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, id_donante);

    printf("\n%-12s | %-45s | %-20s\n", "TIPO", "DETALLES", "FECHA");
    printf("------------------------------------------------------------\n");

    int hay_donaciones = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        hay_donaciones = 1;
        int tipo = sqlite3_column_int(stmt, 0);
        const char *fecha = sqlite3_column_text(stmt, 5)
                            ? (const char *)sqlite3_column_text(stmt, 5)
                            : "Sin fecha";

        switch (tipo) {
            case 2: // COMIDA
                printf("%-12s | %s - %.2f kg | %s\n",
                       "COMIDA",
                       sqlite3_column_text(stmt, 2)
                           ? (const char *)sqlite3_column_text(stmt, 2)
                           : "Desconocido",
                       sqlite3_column_double(stmt, 3),
                       fecha);  // <- añadido
                break;
            case 1: // DINERO
                printf("%-12s | Importe: %.2f EUR | %s\n",
                       "DINERO",
                       sqlite3_column_double(stmt, 4),
                       fecha);  // <- añadido
                break;
            case 3: // ROPA
                printf("%-12s | Ropa variada - %.2f kg | %s\n",
                       "ROPA",
                       sqlite3_column_double(stmt, 1),
                       fecha);  
                break;
            default:
                printf("%-12s | Sin detalles especificos | %s\n", "OTROS", fecha);
                break;
        }
    }

    if (!hay_donaciones) {
        printf("No se han encontrado donaciones.\n");
    }
    printf("--------------------------------------------------------------------\n");

    sqlite3_finalize(stmt);
}

// crear evento de reparto de comida cada martes automaticamente
void crearEventoMartesAutomatico(sqlite3 *db) {
    sqlite3_stmt *stmt;
    char *error = 0;
    
    // 1. COMPROBACIÓN: ¿Ya existe el evento para el próximo martes?
    // Usamos date() sobre la columna para comparar solo el día, ignorando la hora guardada
    const char *sql_check = "SELECT COUNT(*) FROM Evento "
                            "WHERE descripcion = 'Reparto semanal comida' "
                            "AND date(fecha_ini) = date('now', 'weekday 2');";

    int existe = 0;
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            existe = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // 2. INSERCIÓN: Solo si no existe, lo creamos cumpliendo el formato HH:MM
    if (existe == 0) {
        /* Explicación de los valores:
           - material: '1' (Comida)
           - tipo: '1' (Reparto)
           - fecha_ini/fin: Usamos strftime para cumplir el CHECK constraint de tu DB
           - lim_voluntarios: Es el nombre que sale en tu captura (no 'cupo')
        */
        const char *sql_insert = 
            "INSERT INTO Evento (material, descripcion, fecha_ini, fecha_fin, tipo, lim_voluntarios) "
            "VALUES ('1', 'Reparto semanal comida', "
            "strftime('%Y-%m-%d 09:00', 'now', 'weekday 2'), " 
            "strftime('%Y-%m-%d 11:00', 'now', 'weekday 2'), " 
            "'1', 20);";
        
        if (sqlite3_exec(db, sql_insert, 0, 0, &error) == SQLITE_OK) {
            printf("[SISTEMA] Evento automatico de los martes creado (Reparto semanal comida).\n");
        } else {
            // Si esto falla ahora, te dirá exactamente qué restricción no cumples
            printf("[!] Error al crear evento automatico: %s\n", error);
            sqlite3_free(error);
        }
    } else {
        // Descomenta si quieres ver el log de que todo está en orden
        // printf("[SISTEMA] El evento del proximo martes ya esta registrado.\n");
    }
}


// crear evento reparto ropa
void crearEventoJuevesRopaAutomatico(sqlite3 *db) {
    sqlite3_stmt *stmt;
    char *error = 0;
    
    // 1. COMPROBACIÓN: ¿Ya existe la recogida de ropa para el próximo jueves?
    // 'weekday 4' es el Jueves en SQLite
    const char *sql_check = "SELECT COUNT(*) FROM Evento "
                            "WHERE descripcion = 'Recogida semanal ropa' "
                            "AND date(fecha_ini) = date('now', 'weekday 4');";

    int existe = 0;
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            existe = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // 2. INSERCIÓN: Si no existe, creamos el evento para el jueves
    if (existe == 0) {
        // material: '0' (Ropa), tipo: '0' (Recogida)
        // Usamos strftime para cumplir el CHECK constraint YYYY-MM-DD HH:MM
        const char *sql_insert = 
            "INSERT INTO Evento (material, descripcion, fecha_ini, fecha_fin, tipo, lim_voluntarios) "
            "VALUES ('0', 'Recogida semanal ropa', "
            "strftime('%Y-%m-%d 10:00', 'now', 'weekday 4'), " 
            "strftime('%Y-%m-%d 13:00', 'now', 'weekday 4'), " 
            "'0', 15);";
        
        if (sqlite3_exec(db, sql_insert, 0, 0, &error) == SQLITE_OK) {
            printf("[SISTEMA] Evento automatico de los jueves creado (Recogida semanal ropa).\n");
        } else {
            printf("[!] Error al crear evento ropa: %s\n", error);
            sqlite3_free(error);
        }
    }
}


// Ver proximo evento de reparto de comida para beneficiario
void verProximoRepartoComida(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT descripcion, fecha_ini, fecha_fin FROM Evento "
                      "WHERE material = 1 AND tipo = 1 "
                      "AND date(fecha_ini) >= date('now') "
                      "ORDER BY fecha_ini ASC LIMIT 1;";

    printf("\n--- PROXIMO REPARTO DE COMIDA ---\n");
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Evento: %s\n", sqlite3_column_text(stmt, 0));
            printf("Fecha de inicio: %s\n", sqlite3_column_text(stmt, 1));
            printf("Fecha final: %s\n", sqlite3_column_text(stmt, 2));
            printf("\nLos repartos de comida son CADA MARTES desde las 16:00 hasta las 20:00\n");
        } else {
            printf("No hay repartos de comida programados proximamente.\n");
        }
    } else {
        printf("Error al consultar: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

// ver proximo reparto ropa
void verProximoRepartoRopa(sqlite3 *db, int id_beneficiario) {
    sqlite3_stmt *stmt;
    char ultima_fecha[20] = "";

    // 1. Buscamos la última vez que el beneficiario recibió ropa
    // Nota: Aquí buscamos en tu tabla de registros de ayuda/recogida
    const char *sql_last = "SELECT MAX(fecha) FROM RecogidasRopa WHERE id_beneficiario = ?;";

    if (sqlite3_prepare_v2(db, sql_last, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_beneficiario);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_text(stmt, 0) != NULL) {
            strcpy(ultima_fecha, (const char*)sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    // 2. Comprobamos si han pasado los 6 meses
    if (strlen(ultima_fecha) > 0) {
        const char *sql_diff = "SELECT julianday('now') - julianday(?);";
        if (sqlite3_prepare_v2(db, sql_diff, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, ultima_fecha, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                double dias = sqlite3_column_double(stmt, 0);
                if (dias < 180) {
                    printf("\n[INFO] Ultima recogida: %s", ultima_fecha);
                    printf("\nDebes esperar a que pasen 180 dias (faltan %.0f).\n", 180 - dias);
                    return;
                }
            }
            sqlite3_finalize(stmt);
        }
    }

    // 3. Si puede recoger, mostramos el próximo evento de los jueves
    const char *sql_next = "SELECT fecha_ini FROM Evento "
                           "WHERE material = '0' AND tipo = '0' AND date(fecha_ini) >= date('now') "
                           "ORDER BY fecha_ini ASC LIMIT 1;";

    if (sqlite3_prepare_v2(db, sql_next, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("\nProximo reparto de ropa el dia: %s\n", sqlite3_column_text(stmt, 0));
        } else {
            printf("\nNo hay repartos de ropa programados actualmente.\n");
        }
        sqlite3_finalize(stmt);
    }
}
//igual tenemos que moner '1'

// Ver proximo evento reparo de ropa
/*
void verProximoRepartoRopa(sqlite3 *db, int id_beneficiario) {
    sqlite3_stmt *stmt;
    char fecha_corte[11] = "1900-01-01";
    int tiene_registro = 0;
    int id_usuario = -1;

    // Obtener el id_usuario real a partir del id_beneficiario
    const char *sql_get_user = "SELECT id_usuario FROM Beneficiario WHERE id_beneficiario = ?;";
    if (sqlite3_prepare_v2(db, sql_get_user, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_beneficiario);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            id_usuario = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    if (id_usuario == -1) {
        printf("\n--- PROXIMO REPARTO DE ROPA ---\n");
        printf("Error: No se encontró el usuario asociado.\n");
        return;
    }

    // Buscar última recogida de ropa de este beneficiario
    const char *sql_ultima = "SELECT MAX(e.fecha_ini) FROM Evento e "
                             "JOIN Participaciones p ON e.id_evento = p.id_evento "
                             "WHERE p.id_usuario = ? AND e.material = 0 AND e.tipo = 1;";

    if (sqlite3_prepare_v2(db, sql_ultima, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_usuario);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_text(stmt, 0) != NULL) {
            const char *ultima_fecha = (const char *)sqlite3_column_text(stmt, 0);
            tiene_registro = 1;

            // Calcular fecha límite = última recogida + 6 meses
            sqlite3_stmt *stmt_calc;
            const char *sql_meses = "SELECT date(?, '+6 months');";
            if (sqlite3_prepare_v2(db, sql_meses, -1, &stmt_calc, 0) == SQLITE_OK) {
                sqlite3_bind_text(stmt_calc, 1, ultima_fecha, -1, SQLITE_STATIC);
                if (sqlite3_step(stmt_calc) == SQLITE_ROW) {
                    strcpy(fecha_corte, (const char *)sqlite3_column_text(stmt_calc, 0));
                }
                sqlite3_finalize(stmt_calc);
            }
        }
    }
    sqlite3_finalize(stmt);

    // Buscar próximo reparto de ropa disponible (después de la fecha de corte)
    const char *sql_proximo = "SELECT descripcion, fecha_ini, fecha_fin FROM Evento "
                              "WHERE material = 0 AND tipo = 1 "
                              "AND date(fecha_ini) >= date('now') ";

    char sql_con_filtro[500];
    if (tiene_registro) {
        sprintf(sql_con_filtro, "%s AND date(fecha_ini) >= date('%s') ", sql_proximo, fecha_corte);
    } else {
        sprintf(sql_con_filtro, "%s", sql_proximo);
    }
    strcat(sql_con_filtro, "ORDER BY fecha_ini ASC LIMIT 1;");

    printf("\n--- PROXIMO REPARTO DE ROPA ---\n");
    if (sqlite3_prepare_v2(db, sql_con_filtro, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Evento: %s\n", sqlite3_column_text(stmt, 0));
            printf("Fecha inicio: %s\n", sqlite3_column_text(stmt, 1));
            printf("Fecha fin: %s\n", sqlite3_column_text(stmt, 2));
            if (tiene_registro) {
                printf("\nProximo reparto disponible a partir de: %s\n", fecha_corte);
                printf("(Deben pasar 6 meses desde tu ultima recogida)\n");
            }
        } else {
            if (tiene_registro) {
                printf("Aun no puedes recoger ropa. Fecha disponible a partir de: %s\n", fecha_corte);
            } else {
                printf("No hay repartos de ropa programados.\n");
                printf("Se creara automaticamente uno nuevo proximamente.\n");
            }
        }
    } else {
        printf("Error al consultar: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}
*/

void verProximaRecogidaRopa(sqlite3 *db, int id_voluntario) {
    sqlite3_stmt *stmt;
    char ultima_fecha[20] = "";

    // 1. Obtener última participación (Ropa = '0', Recogida = '0')
    const char *sql_last = 
        "SELECT MAX(e.fecha_ini) FROM Participaciones p "
        "JOIN Evento e ON p.id_evento = e.id_evento "
        "WHERE p.id_voluntario = ? AND e.material = '0' AND e.tipo = '0';";

    if (sqlite3_prepare_v2(db, sql_last, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_voluntario);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_text(stmt, 0) != NULL) {
            strcpy(ultima_fecha, (const char*)sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    // 2. Validar los 6 meses (180 días)
    if (strlen(ultima_fecha) > 0) {
        const char *sql_diff = "SELECT julianday('now') - julianday(?);";
        if (sqlite3_prepare_v2(db, sql_diff, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, ultima_fecha, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                double dias = sqlite3_column_double(stmt, 0);
                if (dias < 180) {
                    printf("\n[DENEGADO] Han pasado %.0f dias. Faltan %.0f para los 180.\n", dias, 180 - dias);
                    sqlite3_finalize(stmt);
                    return; 
                }
            }
            sqlite3_finalize(stmt);
        }
    }

    // 3. Mostrar próximos 6 meses
    const char *sql_list = "SELECT descripcion, fecha_ini FROM Evento WHERE material = '0' AND tipo = '0' "
                           "AND date(fecha_ini) >= date('now') ORDER BY fecha_ini ASC LIMIT 6;";

    printf("\n--- PROXIMAS RECOGIDAS DE ROPA ---\n");
    if (sqlite3_prepare_v2(db, sql_list, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("- %s: %s\n", sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }
}
// Ver proximos talleres
void verTalleresProximos(sqlite3 *db) {
    sqlite3_stmt *stmt;
    // Añadimos el filtro de fecha para que solo salgan los futuros
    const char *sql = "SELECT tipo, descripcion, fecha_ini, fecha_fin FROM Taller "
                      "WHERE date(fecha_ini) >= date('now') "
                      "ORDER BY fecha_ini ASC;";

    printf("\n--- LISTA DE PRÓXIMOS TALLERES ---\n");
    printf("%-15s | %-25s | %-17s | %-17s\n", "TIPO", "DESCRIPCIÓN", "INICIO", "FIN");
    printf("----------------------------------------------------------------------------------\n");

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        int hay_talleres = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            hay_talleres = 1;
            const char *tipo = (const char *)sqlite3_column_text(stmt, 0);
            const char *desc = (const char *)sqlite3_column_text(stmt, 1);
            const char *ini  = (const char *)sqlite3_column_text(stmt, 2);
            const char *fin  = (const char *)sqlite3_column_text(stmt, 3);

            // Usamos un operador ternario para evitar que el programa falle si hay nulos
            printf("%-15s | %-25s | %-17s | %-17s\n", 
                   tipo ? tipo : "General", 
                   desc ? desc : "Sin descripción", 
                   ini  ? ini  : "---", 
                   fin  ? fin  : "---");
        }
        
        if (!hay_talleres) {
            printf("No hay talleres programados para los próximos días.\n");
        }
    } else {
        printf("Error al consultar la tabla Taller: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    printf("----------------------------------------------------------------------------------\n");
}


// Calcular ayuda de dinero para beneficiario
float calcularAyudaDinero(Beneficiario b) {
    float renta = b.ingresos - b.gastos;
    // Cubrimos el déficit mensual + un pequeño bono de emergencia
    return fabsf(renta) + 50.0f; // fabsf(valor absoluto)
}

//Ayuda alimentación semanal
void mostrarAyudaComida(Beneficiario b) {
    float totalArrozPasta = (b.num_adultos * 1.0f) + (b.num_ninos * 0.75f);
    float totalLegumbres = (b.num_adultos + b.num_ninos) * 0.5f;
    float totalLeche = (b.num_adultos * 2.0f) + (b.num_ninos * 4.0f);
    int totalConservas = (b.num_adultos * 3) + (b.num_ninos * 2);


    printf("\n[ALIMENTACIÓN SEMANAL]");  
    printf("\n > Arroz/Pasta:        %.2f kg", totalArrozPasta);
    printf("\n > Legumbres:          %.2f kg", totalLegumbres);
    printf("\n > Leche:              %.0f litros", totalLeche);
    printf("\n > Conservas:          %d latas", totalConservas);
}


//Ayuda ropa semestral
void mostrarAyudaRopa(Beneficiario b) {
    int camNinos = b.num_ninos * 3;
    int panNinos = b.num_ninos * 2;
    int sudNinos = b.num_ninos * 1;


    int camAdultos = b.num_adultos * 2;
    int panAdultos = b.num_adultos * 1;


    printf("\n[VESTIMENTA SEMESTRAL]"); 
    if (b.num_ninos > 0) {
        printf("\n > NIÑOS/AS: %d camisetas, %d pantalones, %d sudaderas",
                camNinos, panNinos, sudNinos);
    }
    if (b.num_adultos > 0) {
        printf("\n > ADULTOS: %d camisetas, %d pantalones",
                camAdultos, panAdultos);
    }
}

// Menu principal
void menuPrincipal(sqlite3 *db, int tipo, int id_perfil)
{


    int opcion;


    do {
        printf("\n======= MENU PRINCIPAL =======");


        // Mostrar opciones ESPECÍFICAS según el tipo
        if (tipo == VOLUNTARIO)
        { // tipo 0
            printf("\n1. Apuntarse a un evento");
            printf("\n2. Consultar calendario de mis eventos"); // despauntarse calendario barrun zaudela ingo deu
            printf("\n3. Consultar historial de mi voluntariado");


        }
        else if (tipo == DONANTE) { // tipo 1
            printf("\n1. Realizar donación de dinero");
            printf("\n2. Realizar donación de comida");
            printf("\n3. Realizar donación de ropa");
            printf("\n4. Consultar historial de mis donaciones");
        }
        else if (tipo == BENEFICIARIO) { // tipo 2
            printf("\n1. Cambiar condiciones"); //iuel registrar condiciones registratzeakun derrigorra bezela jarri??
            printf("\n2. Consultar horarios para recoger ayudas");
            printf("\n3. Ver proximos talleres");
        }
       
        printf("\n0. Cerrar sesión");
        printf("\nSeleccione una opción: ");
        scanf("%d", &opcion);


        switch(opcion) {
            case 1:
            if(tipo == VOLUNTARIO) {
                apuntarseEvento(db, id_perfil);
            }
            else if (tipo == DONANTE)
            {
                donarDinero(db, id_perfil);
            }
            else if (tipo == BENEFICIARIO)
            {
                Beneficiario b_actualizada = guardarCondicionesBeneficiario(); // 2. Los mandamos a la base de datos para que el cambio sea real
                if (actualizarDatosBeneficiario(db, id_perfil, b_actualizada))
                {
                    printf("\n---------------------------------------------------------");
                    printf("\n[SISTEMA] Tus condiciones se han actualizado correctamente en tu perfil.\n");
                }
            }
                break;


            case 2:
                if(tipo == VOLUNTARIO) {
                    consultarMisEventos(db, id_perfil);
                } else if(tipo == DONANTE) {
                    donarComida(db, id_perfil);
                } else if(tipo == BENEFICIARIO) {
                    printf("\nCONSULTAR HORARIOS DE AYUDAS\n");
                    verProximoRepartoComida(db);
                    printf("\n");
                    verProximoRepartoRopa(db, id_perfil);
                    printf("\nInformacion adicional:\n");
                    printf("  - Comida: Todos los martes a las 18:00\n");
                    printf("  - Ropa: Cada 6 meses desde tu ultima recogida\n");
                }
                break;

            case 3:
                if(tipo == VOLUNTARIO) {
                    consultarHistorialEventos(db, id_perfil);
                } else if(tipo == DONANTE) {
                    donarRopa(db, id_perfil);
                } else if(tipo==BENEFICIARIO){
                    verTalleresProximos(db);
                }
                break;
           
            case 4:
                if (tipo == DONANTE) {
                    listarDonaciones(db, id_perfil);
                }
                break;


        }
    } while (opcion != 0);
}

// --- Función auxiliar para no ensuciar el código principal ---
int buscarIdEspecifico(sqlite3 *db, int id_usuario, int tipo) {
    sqlite3_stmt *stmt;
    int id_final = -1;
    const char *sql;

    if (tipo == VOLUNTARIO) sql = "SELECT id_voluntario FROM Voluntarios WHERE id_usuario = ?;";
    else if (tipo == DONANTE) sql = "SELECT id_donante FROM Donantes WHERE id_usuario = ?;";
    else if (tipo == BENEFICIARIO) sql = "SELECT id_beneficiario FROM Beneficiario WHERE id_usuario = ?;";
    else return id_usuario; // Para el Admin, usamos su ID de usuario normal

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_usuario);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            id_final = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return id_final;
}


//A PARTIR DE AQUÍ SIN ARREGLAR

void mostrarProximaRecogida(sqlite3 *db, int material) {
    sqlite3_stmt *stmt;
    // Buscamos eventos de tipo 0 (Recogida) que ocurran a partir de hoy
    const char *sql =
        "SELECT descripcion, fecha_ini, fecha_fin FROM Evento "
        "WHERE material = ? AND tipo = 0 "
        "AND date(fecha_ini) >= date('now') "
        "ORDER BY fecha_ini ASC LIMIT 1;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, material);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("\n--- PROXIMO EVENTO DE RECOGIDA ---\n");
            printf("Evento:  %s\n", sqlite3_column_text(stmt, 0));
            printf("Inicio:  %s\n", sqlite3_column_text(stmt, 1));
            printf("Fin:     %s\n", sqlite3_column_text(stmt, 2));
            printf("--------------------------------------------\n");
        } else {
            // Si llega aquí es porque la consulta no devolvió filas
            printf("\n[INFO] No hay eventos de recogida programados en el sistema.\n");
            printf("Por favor, acude a nuestra sede central de lunes a viernes.\n");
        }
    } else {
        printf("Error SQL: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}


void asegurarEventoRopa(sqlite3 *db) {
    sqlite3_stmt *stmt;
    char *error = 0;

    //printf("[SISTEMA] Verificando calendario de recogida de ropa (6 meses)...\n");

    for (int i = 0; i < 6; i++) {
        char sql_check[256];
        char sql_insert[512];
        int existe = 0;

        // 1. Preparamos la consulta para comprobar si hay evento en el mes 'i'
        // date('now', '+i month', 'start of month') busca en el rango de ese mes específico
        sprintf(sql_check, 
            "SELECT COUNT(*) FROM Evento "
            "WHERE material = '0' AND tipo = '0' "
            "AND strftime('%%m-%%Y', fecha_ini) = strftime('%%m-%%Y', 'now', '+%d month');", i);

        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                existe = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        // 2. Si no hay evento para ese mes, lo creamos
        if (existe == 0) {
            sprintf(sql_insert, 
                "INSERT INTO Evento (material, descripcion, fecha_ini, fecha_fin, tipo, lim_voluntarios) "
                "VALUES ('0', 'Recogida Mensual Ropa', "
                "strftime('%%Y-%%m-15 10:00', 'now', '+%d month'), " // El día 15 de cada mes
                "strftime('%%Y-%%m-15 14:00', 'now', '+%d month'), " 
                "'0', 25);", i, i);

            if (sqlite3_exec(db, sql_insert, 0, 0, &error) != SQLITE_OK) {
                printf("[!] Error creando evento mes %d: %s\n", i, error);
                sqlite3_free(error);
            } else {
                printf("[+] Creado evento de ropa para el mes +%d.\n", i);
            }
        }
    }
}

void registrarRecogidaRopa(sqlite3 *db, int id_beneficiario, int id_evento) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO RecogidaRopa (id_beneficiario, id_evento, fecha_recogida) VALUES (?, ?, date('now'));";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_beneficiario);
        sqlite3_bind_int(stmt, 2, id_evento);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("\n[OK] Recogida registrada con éxito.\n");
        } else {
            printf("\n[!] Error: Puede que esa recogida ya esté registrada.\n");
        }
        sqlite3_finalize(stmt);
    }
}

void registrarRecogidaRopaInterfaz(sqlite3 *db) {
    int id_beneficiario, id_evento;

    printf("\n--- REGISTRAR ENTREGA DE ROPA ---\n");

    // 1. Listar beneficiarios (Asegúrate de que esta función existe)
    // Si no tienes listarBeneficiarios, puedes usar un SELECT rápido aquí
    printf("\nLISTA DE BENEFICIARIOS:\n");
    const char *sql_b = "SELECT b.id_beneficiario, u.nombre, u.apellidos FROM Beneficiario b "
                        "JOIN Usuarios u ON b.id_usuario = u.id_usuario;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql_b, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("ID: %d | %s %s\n", sqlite3_column_int(stmt, 0), 
                   sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2));
        }
        sqlite3_finalize(stmt);
    }

    printf("\nIntroduce el ID del beneficiario: ");
    if (scanf("%d", &id_beneficiario) != 1) return;

    // 2. Listar eventos de ropa (Material '0' es ropa en tu DB)
    printf("\nEVENTOS DE ROPA DISPONIBLES:\n");
    const char *sql_e = "SELECT id_evento, descripcion, fecha_ini FROM Evento WHERE material = '0';";
    if (sqlite3_prepare_v2(db, sql_e, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("ID: %d | %s (%s)\n", sqlite3_column_int(stmt, 0), 
                   sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2));
        }
        sqlite3_finalize(stmt);
    }

    printf("\nIntroduce el ID del evento de ropa: ");
    if (scanf("%d", &id_evento) != 1) return;

    // 3. Llamar a la función que guarda en la tabla RecogidaRopa
    registrarRecogidaRopa(db, id_beneficiario, id_evento);
}

void registrarRecogidaRopaAdmin(sqlite3 *db) {
    sqlite3_stmt *stmt;
    int id_beneficiario, id_evento;
    
    // Mostrar lista de beneficiarios
    printf("\n--- LISTA DE BENEFICIARIOS ---\n");
    const char *sql_benef = "SELECT b.id_beneficiario, u.nombre, u.apellidos "
                             "FROM Beneficiario b JOIN Usuarios u ON b.id_usuario = u.id_usuario;";
    if (sqlite3_prepare_v2(db, sql_benef, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char *nombre = (const char *)sqlite3_column_text(stmt, 1);
            const char *apellidos = (const char *)sqlite3_column_text(stmt, 2);
            printf("ID: %d - %s %s\n", id, nombre, apellidos);
        }
    }
    sqlite3_finalize(stmt);
    
    printf("Introduce el ID del beneficiario: ");
    scanf("%d", &id_beneficiario);
    
    // Mostrar eventos de reparto de ropa futuros
    printf("\n--- EVENTOS DE REPARTO DE ROPA FUTUROS ---\n");
    const char *sql_eventos = "SELECT id_evento, descripcion, fecha_ini FROM Evento "
                              "WHERE material = 0 AND tipo = 1 AND date(fecha_ini) >= date('now') "
                              "ORDER BY fecha_ini ASC;";
    if (sqlite3_prepare_v2(db, sql_eventos, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char *desc = (const char *)sqlite3_column_text(stmt, 1);
            const char *fecha = (const char *)sqlite3_column_text(stmt, 2);
            printf("ID: %d - %s - %s\n", id, desc, fecha);
        }
    }
    sqlite3_finalize(stmt);
    
    printf("Introduce el ID del evento de ropa: ");
    scanf("%d", &id_evento);
    
    // Llamar a la función existente
    registrarRecogidaRopa(db, id_beneficiario, id_evento);
}




//ADMINISTRADOR

int es_bisiesto(int a)
{
    if (a % 400 == 0)
        return 1;
    if (a % 100 == 0)
        return 0;
    if (a % 4 == 0)
        return 1;
    return 0;
}

int comparar_fechas(Fecha f1, Fecha f2)
{
    if (f1.anyo < f2.anyo)
        return 1;
    if (f1.anyo > f2.anyo)
        return 0;

    if (f1.mes < f2.mes)
        return 1;
    if (f1.mes > f2.mes)
        return 0;

    if (f1.dia < f2.dia)
        return 1;
    if (f1.dia > f2.dia)
        return 0;

    if (f1.hora < f2.hora)
        return 1;
    if (f1.hora > f2.hora)
        return 0;

    if (f1.minutos < f2.minutos)
        return 1;

    return 0;
}
void listarBeneficiarios(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT b.id_beneficiario, u.nombre, u.apellidos "
                      "FROM Beneficiario b JOIN Usuarios u ON b.id_usuario = u.id_usuario;";
    printf("\n--- BENEFICIARIOS ---\n");
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char *nombre = (const char *)sqlite3_column_text(stmt, 1);
            const char *apellidos = (const char *)sqlite3_column_text(stmt, 2);
            printf("ID: %d - %s %s\n", id, nombre, apellidos);
        }
    }
    sqlite3_finalize(stmt);
}

void listarEventosRopaFuturos(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id_evento, descripcion, fecha_ini FROM Evento "
                      "WHERE material = 0 AND tipo = 1 AND date(fecha_ini) >= date('now') "
                      "ORDER BY fecha_ini ASC;";
    printf("\n--- EVENTOS DE ROPA FUTUROS ---\n");
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char *desc = (const char *)sqlite3_column_text(stmt, 1);
            const char *fecha = (const char *)sqlite3_column_text(stmt, 2);
            printf("ID: %d - %s - %s\n", id, desc, fecha);
        }
    }
    sqlite3_finalize(stmt);
}



// Admiñan menua
void menuAdministrador(sqlite3 *db) {
    int opcion;
    do {
        printf("\n==================================");
        printf("\n   PANEL DE ADMINISTRACIÓN");
        printf("\n==================================");
        printf("\n1. Crear nuevo evento");
        printf("\n2. Gestionar eventos (Borrar)");
        printf("\n3. Listar usuarios registrados");
        printf("\n4. Dar de baja a un usuario");
        printf("\n5. Registrar recogida de ropa");
        printf("\n6. Asignar voluntario a taller");
        printf("\n0. Cerrar sesión");
        printf("\n----------------------------------");
        printf("\nSeleccione una opción: ");

        if (scanf("%d", &opcion) != 1) {
            printf("\n[!] Por favor, introduce el número de la opción deseada.\n");
            
            opcion = -1; // Forzamos que entre en el default para repetir
            continue;
        }
    

        switch(opcion) {
            case 1: crearEvento(db); break;
            case 2: borrarEvento(db); break;
            case 3: listarUsuarios(db); break;
            case 4: darBajaUsuario(db); break;
            case 5: registrarRecogidaRopaAdmin(db); break;
            case 6: crearTaller(db); break;
            case 0: printf("\nFinalizando sesión administrativa. ¡Buen día!\n"); break;
            default: printf("\n[!] Esa opción no está en el menú. Inténtalo de nuevo.\n"); break;
        }
    } while (opcion != 0);
}

