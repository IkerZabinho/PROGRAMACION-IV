#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "funciones.h"
#include "estructuras.h"
#include "sqlite3.h"


// Insertar beneficiarios

int insertarDatosBeneficiario(sqlite3 *db, long long id, float ing, float gas, int adu, int nin)
{
    char sql[400];
    sprintf(sql, "INSERT INTO Beneficiario (id_usuario, ingresos, gastos, num_adultos, num_nino) "
                 "VALUES (%lld, %.2f, %.2f, %d, %d);",
            id, ing, gas, adu, nin);
    return (sqlite3_exec(db, sql, 0, 0, 0) == SQLITE_OK);
}
// insertar voluntarios
int insertarDatosVoluntario(sqlite3 *db, long long id, const char *rol)
{
    char sql[300];
    sprintf(sql, "INSERT INTO Voluntarios (id_usuario, rol) VALUES (%lld, '%s');", id, rol);
    return (sqlite3_exec(db, sql, 0, 0, 0) == SQLITE_OK);
}



//funcion para condiciones de beneficiario (para registrar y cambiar)
Beneficiario guardarCondicionesBeneficiario (){

    Beneficiario b;
    int correcto;

    float sueldos=0;
    float ayudas=0;
    float alquiler=0;
    float suministros=0;
    float material_escolar=0;
    float estudios=0;
    float otros=0;

    printf("\n--- DETALLES ECONÓMICOS DEL BENEFICIARIO ---\n");
    
    printf("\n*responde la pregunta y pulsa enter para pasar a la siguiente pregunta\n");


    do {
        printf("\n> INTEGRANTES DE LA FAMILIA\n");
        printf("Número de adultos en casa: ");
        scanf("%d", &b.num_adultos);
        while (getchar() != '\n');
        printf("Número de niños/as en casa: ");
        scanf("%d", &b.num_ninos);
        while (getchar() != '\n');
        printf("  > ¿Deseas cambiar algún dato de los integrantes de la familia? (1: Sí / 0: No): ");
        scanf("%d", &correcto);
        

        //limpia buffer
        while (getchar() != '\n');

        if (correcto == 1) {
        printf("[!] Volviendo a pedir los datos de integrantes...\n");
    }

    } while (correcto != 0);

    do {
    printf("\n> INGRESOS\n");
    printf("Sueldo mensual total: ");
    scanf("%f", &sueldos);
    while (getchar() != '\n');
    printf("Otras ayudas/pensiones: ");
    scanf("%f", &ayudas);
    while (getchar() != '\n');

    printf("  > ¿Deseas cambiar algún dato de los ingresos? (1: Sí / 0: No): ");
    scanf("%d", &correcto);
    

    //limpia buffer
    while (getchar() != '\n');

    if (correcto == 1) {
    printf("[!] Volviendo a pedir los datos de ingresos...\n");

    }
    } while (correcto != 0);


    do {
    printf("\n> GASTOS\n");
    printf("Alquiler o hipoteca: ");
    scanf("%f", &alquiler);
    while (getchar() != '\n');
    printf("Luz, agua y gas (normalmente): ");
    scanf("%f", &suministros);
    while (getchar() != '\n');
    printf("Material escolar (0 si no hay): ");
    scanf("%f", &material_escolar);
    while (getchar() != '\n');
    printf("Gastos en estudios (0 si no hay): ");
    scanf("%f", &estudios);
    while (getchar() != '\n');
    printf("Otros gastos (0 si no hay): ");
    scanf("%f", &otros);
    while (getchar() != '\n');

    printf("  > ¿Deseas cambiar algún dato de los gastos? (1: Sí / 0: No): ");
    scanf("%d", &correcto);
    

    //limpia buffer
    while (getchar() != '\n');

    if (correcto == 1) {
    printf("[!] Volviendo a pedir los datos de gastos...\n");

    } 
    } while (correcto != 0);

    float gastosTotal = alquiler + suministros + material_escolar + estudios + otros;
    float ingresosTotal = sueldos + ayudas;

    //guardamos datos en structura para luego calcular ayuda
    b.ingresos = ingresosTotal;
    b.gastos = gastosTotal;


    //funcion para calcular la ayuda que necesitarán
    
    evaluarBeneficiario(b);

    return b;
};

//función para actualizar los datos de beneficiario
int actualizarDatosBeneficiario(sqlite3 *db, int id_usuario, Beneficiario b) {
    char sql[400];
    char *error = 0;

    // Usamos UPDATE para modificar la fila que ya existe
    sprintf(sql, 
        "UPDATE Beneficiario SET ingresos = %.2f, gastos = %.2f, "
        "num_adultos = %d, num_nino = %d WHERE id_usuario = %d;",
        b.ingresos, b.gastos, b.num_adultos, b.num_ninos, id_usuario);

    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("Error al actualizar base de datos: %s\n", error);
        sqlite3_free(error);
        return 0;
    }
    return 1;
}

// insertar usuarios
int insertarUsuario(sqlite3 *db, Usuario u)
{
char sql[500];
char *error = 0;

// 1. RECOLECTAR DATOS ESPECÍFICOS PRIMERO (en memoria)

char *rolVoluntario = "General"; // Para voluntario
Beneficiario b = {0}; //por si acaso
if (u.tipoUsuario == BENEFICIARIO)
{
     b = guardarCondicionesBeneficiario();

    } else if (u.tipoUsuario == VOLUNTARIO)
    {
        int op;
        printf("1.Profesor\n 2.Entrenador\n 3.Cocinero\n 4.Repartidor\n Seleccione:\n ");
        scanf("%d", &op);
        if (op == 1)
            rolVoluntario = "profesor";
        else if (op == 2)
            rolVoluntario = "entrenador";
        else if (op == 3)
            rolVoluntario = "cocinero";
        else if (op == 4)
            rolVoluntario = "repartidor";
    }

    // 2. INICIAR TRANSACCIÓN SQL
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    // 3. INSERTAR EN TABLA PRINCIPAL (USUARIOS)
    sprintf(sql, "INSERT INTO Usuarios (nombre, apellidos, nombre_usuario, contrasena, tipo) "
                 "VALUES ('%s', '%s', '%s', '%s', %d);",
            u.nombre, u.apellidos, u.nombre_usuario, u.contrasena, u.tipoUsuario);

    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK)
    {
        printf("Error crítico: %s\n", error);
        sqlite3_free(error);
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0); // Cancelar todo
    }

    // 4. INSERTAR EN TABLA ESPECÍFICA
    long long id = sqlite3_last_insert_rowid(db);
    int exitoEspecifico = 0;

    if (u.tipoUsuario == BENEFICIARIO)
    {
        exitoEspecifico = insertarDatosBeneficiario(db, id, b.ingresos, b.gastos, b.num_adultos, b.num_ninos);
    }
    else if (u.tipoUsuario == VOLUNTARIO)
    {
        exitoEspecifico = insertarDatosVoluntario(db, id, rolVoluntario);
    }
    else if (u.tipoUsuario == DONANTE)
    {
        char sqlD[100];
        sprintf(sqlD, "INSERT INTO Donantes (id_usuario) VALUES (%lld);", id);
        exitoEspecifico = (sqlite3_exec(db, sqlD, 0, 0, 0) == SQLITE_OK);
    }

    // 5. FINALIZAR: ¿Salió todo bien?
    if (exitoEspecifico)
    {
        sqlite3_exec(db, "COMMIT;", 0, 0, 0); // Guardar permanentemente
        printf("Registro completado con éxito en todas las tablas.\n");
        return 1;
    }
    else
    {
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0); // Deshacer el usuario base si falló el detalle
        printf("Error en datos específicos. Registro cancelado totalmente.\n");
        return 0;
    }
}


//calcular ayuda de Beneficiario




// Callback login
int callbackLogin(void *data, int argc, char **argv, char **colName)
{
    int *encontrado = (int *)data;
    *encontrado = 1;
    return 0;
}

// Comprobar login
// int comprobarLogin(sqlite3 *db, char *user, char *pass, int *tipo)  //TIPO ERABILI GABE DAO???
// {
//     char sql[300];
//     int encontrado = 0;
//     sprintf(sql, "SELECT * FROM Usuarios WHERE nombre_usuario='%s' AND contrasena='%s';", user, pass);

//     char *error = 0;
//     sqlite3_exec(db, sql, callbackLogin, &encontrado, &error);

//     return encontrado;
// }

//PRUEBA
int comprobarLogin(sqlite3 *db, char *user, char *pass, int *tipo_res, int *id_res) {
    sqlite3_stmt *stmt;
    char *sql = "SELECT tipo, id_usuario FROM Usuarios WHERE nombre_usuario = ? AND contrasena = ?;";
    int encontrado = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        // Enlazamos los parámetros para evitar Inyección SQL
        sqlite3_bind_text(stmt, 1, user, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, pass, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            // "Extraemos" los valores de la fila encontrada
            *tipo_res = sqlite3_column_int(stmt, 0); 
            *id_res = sqlite3_column_int(stmt, 1);
            encontrado = 1;
        }
    }
    sqlite3_finalize(stmt);
    return encontrado;
}

// Mostrar usuarios
int callbackMostrar(void *data, int argc, char **argv, char **colName)
{
    printf("\n--- Usuario ---\n");
    for (int i = 0; i < argc; i++)
    {
        // Contraseña ez azaltzeko
        if (strcmp(colName[i], "contrasena") == 0)
            continue;
        printf("%s: %s\n", colName[i], argv[i] ? argv[i] : "NULL");
    }
    return 0;
}

void mostrarUsuarios(sqlite3 *db)
{
    char *sql = "SELECT * FROM Usuarios;";
    char *error = 0;
    if (sqlite3_exec(db, sql, callbackMostrar, 0, &error) != SQLITE_OK)
    {
        printf("Error al mostrar usuarios: %s\n", error);
        sqlite3_free(error);
    }
}

// DONAZIUAK

//DINERO
void donarDinero(sqlite3 *db, int id_usuario) 
{
    float cantidad;
    char sql[300];
    char *error = 0;
    int respuesta;

    printf("\n--- REALIZAR DONACIÓN DE DINERO ---\n");

    printf("Introduce la cantidad a donar: ");
    if (scanf("%f", &cantidad) != 1) {
        printf("Error: Entrada no válida.\n");
        while (getchar() != '\n'); // Limpiar buffer
        return;
    }

    if (cantidad <= 0) {
        printf("La cantidad debe ser mayor que 0.\n");
        return;
    }

    printf("¿Estás seguro de que quieres donar %.2f€?\n0. No\n1. Sí\nSelección: ", cantidad);
    scanf("%d", &respuesta);

    if (respuesta == 1) {
        // PASO A: Insertar en la tabla 'Donaciones' (La tabla padre)
        // El id_usuario vincula quién dona, y el '1' indica que es tipo dinero.
        sprintf(sql, "INSERT INTO Donaciones (id_donante, tipo) VALUES (%d, 1);", id_usuario);

        if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) 
        {
            // PASO B: Obtener el ID que SQLite acaba de crear para esta donación
            long long id_padre = sqlite3_last_insert_rowid(db);

            // PASO C: Insertar en tu tabla 'Dinero'
            // No insertamos 'id_dinero' porque es PRIMARY KEY AUTOINCREMENT
            char sqlDinero[400];
            sprintf(sqlDinero, "INSERT INTO Dinero (cantidad, id_donacion) VALUES (%.2f, %lld);", 
                    cantidad, id_padre);

            if (sqlite3_exec(db, sqlDinero, 0, 0, &error) == SQLITE_OK) {
                printf("\n[ÉXITO] Se han registrado %.2f€ correctamente.\n", cantidad);
                printf("ID de Donante: %lld\n", id_padre);
            } else {
                printf("Error en tabla Dinero: %s\n", error);
                sqlite3_free(error);
            }
        } 
        else 
        {
            printf("Error en tabla Donaciones: %s\n", error);
            sqlite3_free(error);
        }
    } 
    else 
    {
        printf("Operación cancelada por el usuario.\n");
    }
}

// En funciones.c - añadir estas dos funciones

//COMIDA
void donarComida(sqlite3 *db, int id_usuario) 
{
    float kilos;
    char tipo_comida[100];
    char sql[300];
    char *error = 0;
    int respuesta;
    int seleccion;

    const char *nombresCategorias[] = {"", "Carbohidratos", "Legumbres", "Conservas", "Lacteos", "Infantil"};

    printf("\n--- REALIZAR DONACIÓN DE COMIDA ---\n");

    printf("Seleccione el tipo de alimento:\n");
    printf("1. %s\n2. %s\n3. %s\n4. %s\n5. %s\n", 
            nombresCategorias[1], nombresCategorias[2], nombresCategorias[3], 
            nombresCategorias[4], nombresCategorias[5]);
    printf("Selección: ");
    
    if (scanf("%d", &seleccion) != 1 || seleccion < 1 || seleccion > 5) {
        printf("Opción no válida.\n");
        while (getchar() != '\n');
        return;
    } 

    printf("Cantidad en kilogramos: ");
    if (scanf("%f", &kilos) != 1 || kilos <= 0) {
        printf("Cantidad no válida.\n");
        while (getchar() != '\n');
        return;
    }

    printf("¿Confirmas donar %.2f kg de %s?\n0. No\n1. Sí\nSelección: ", kilos, tipo_comida);
    scanf("%d", &respuesta);

    if (respuesta == 1) {
        // PASO A: Insertar en tabla padre 'Donaciones' (tipo 2 = comida)
        sprintf(sql, "INSERT INTO Donaciones (id_donante, tipo) VALUES (%d, 2);", id_usuario);

        if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) 
        {
            long long id_padre = sqlite3_last_insert_rowid(db);

            // PASO B: Insertar en tabla 'Comida'
            char sqlComida[400];
           sprintf(sqlComida, 
                    "INSERT INTO Comida (tipo_comida, kilos, id_donacion) VALUES ('%s', %.2f, %lld);",
                    nombresCategorias[seleccion], kilos, id_padre);

            if (sqlite3_exec(db, sqlComida, 0, 0, &error) == SQLITE_OK) {
                printf("\n[ÉXITO] Donación de %.2f kg de %s registrada correctamente.\n", kilos, tipo_comida);
            } else {
                printf("Error en tabla Comida: %s\n", error);
                sqlite3_free(error);
            }
        } 
        else 
        {
            printf("Error en tabla Donaciones: %s\n", error);
            sqlite3_free(error);
        }
    } 
    else 
    {
        printf("Operación cancelada.\n");
    }
}


//ROPA
void donarRopa(sqlite3 *db, int id_usuario) 
{
    float kilos;
    char sql[300];
    char *error = 0;
    int respuesta;

    printf("\n--- REALIZAR DONACIÓN DE ROPA ---\n");

    // printf("Tipo de ropa (ej: camisetas, abrigos, pantalones...): ");
    // while (getchar() != '\n');
    // fgets(tipo_ropa, sizeof(tipo_ropa), stdin);
    // tipo_ropa[strcspn(tipo_ropa, "\n")] = 0;

    printf("Cantidad en kilogramos: ");
    if (scanf("%f", &kilos) != 1 || kilos <= 0) {
        printf("Cantidad no válida.\n");
        while (getchar() != '\n');
        return;
    }

    printf("¿Confirmas donar %.2f kg de %s?\n0. No\n1. Sí\nSelección: ", kilos);
    scanf("%d", &respuesta);

    if (respuesta == 1) {
      //Sartu donaziuetan 3.tipua
        sprintf(sql, "INSERT INTO Donaciones (id_donante, tipo) VALUES (%d, 3);", id_usuario);

        if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) 
        {
            long long id_padre = sqlite3_last_insert_rowid(db);

            //Meter ropa 
            char sqlRopa[400];
            // sprintf(sqlRopa, 
            //         "INSERT INTO Ropa (tipo_ropa, kilos, id_donacion) VALUES ('%s', %.2f, %lld);",
            //         tipo_ropa, kilos, id_padre);

           sprintf(sqlRopa, 
                "INSERT INTO Ropa (id_donacion, kilos) VALUES (%lld,%.2f);",
                id_padre, kilos);

            if (sqlite3_exec(db, sqlRopa, 0, 0, &error) == SQLITE_OK) {
                printf("\n[ÉXITO] Donación de %.2f kg de %s registrada correctamente.\n", kilos);
            } else {
                printf("Error en tabla Ropa: %s\n", error);
                sqlite3_free(error);
            }
        } 
        else 
        {
            printf("Error en tabla Donaciones: %s\n", error);
            sqlite3_free(error);
        }
    } 
    else 
    {
        printf("Operación cancelada.\n");
    }
}




// EVENTUAK



// ROLAN ARABERA MENUA



// Funciones de menú
// HAU ZABANA GEHIXKI BADAO NERIA ZABANA JARRI BERRIZ!!
/*
void iniciarSesion(sqlite3 *db) {
    char user[50], pass[50];
    printf("\n--- INICIAR SESIÓN ---\n");
    printf("Usuario: ");
    scanf("%s", user);
    printf("Contraseña: ");
    scanf("%s", pass);

    if (comprobarLogin(db, user, pass)) {
        printf("\n¡Bienvenido %s!\n", user);
    } else {
        printf("\nError: Usuario o contraseña incorrectos.\n");
    }
}
    */

// INICIAR SESION 2.0
void iniciarSesion(sqlite3 *db)
{

    char user[50], pass[50];
    int tipo_detectado = -1;
    int id_detectado = -1;
    printf("\n--- INICIAR SESIÓN ---\n");

    int tipo;

    printf("\nUsuario: ");
    scanf("%s", user);

    printf("Contraseña: ");
    scanf("%s", pass);

   if (comprobarLogin(db, user, pass, &tipo_detectado, &id_detectado)) {
    printf("DEBUG tipo=%d, ADMINISTRADOR=%d\n", tipo_detectado, (int)ADMINISTRADOR);
    printf("\n");
    printf("Bienvenido!\n");

    if (tipo_detectado == (int)ADMINISTRADOR) {
    menuAdministrador(db);
    }else {
    menuPrincipal(db, tipo_detectado, id_detectado);
    }
} else {
    printf("Error login\n");
}
}

void registrarUsuario(sqlite3 *db)
{
    Usuario nuevoUsuario;
    char buffer[50];
    int rolElegido;

    printf("\n--- REGISTRO DE NUEVO USUARIO ---\n");
    printf("Elige tu rol:\n1. Voluntario\n2. Donante\n3. Beneficiario\nRol (1-3): ");
    if (scanf("%d", &rolElegido) != 1 || rolElegido < 1 || rolElegido > 3)
    {
        printf("Rol no válido. Volviendo al menú principal...\n");
        return;
    }
    nuevoUsuario.tipoUsuario = (TipoUsuario)(rolElegido - 1);

    // Captura de datos
    printf("Nombre: ");
    scanf("%s", buffer);
    nuevoUsuario.nombre = strdup(buffer);

    printf("Apellidos: "); //Estas lineas para que funcione al escribir 2 apellidos
    while (getchar() != '\n'); 
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    nuevoUsuario.apellidos = strdup(buffer);

    printf("Nombre de usuario: ");
    scanf("%s", buffer);
    nuevoUsuario.nombre_usuario = strdup(buffer);

    printf("Contraseña: ");
    scanf("%s", buffer);
    nuevoUsuario.contrasena = strdup(buffer);

    // INTENTO DE INSERCIÓN
    if (insertarUsuario(db, nuevoUsuario))
    {
        int id_recien_creado = (int)sqlite3_last_insert_rowid(db);
        // SI SALE BIEN:
        printf("\nRegistro completado con éxito. Accediendo a tu menú...\n");

        // Liberamos memoria antes de entrar en otro bucle (menú)
        free(nuevoUsuario.nombre);
        free(nuevoUsuario.apellidos);
        free(nuevoUsuario.nombre_usuario);
        free(nuevoUsuario.contrasena);

        menuPrincipal(db, (int)nuevoUsuario.tipoUsuario, id_recien_creado);
    }
    else
    {
        // SI SALE MAL:

        printf("Volviendo al menú principal...\n");

        // Liberamos memoria igualmente
        free(nuevoUsuario.nombre);
        free(nuevoUsuario.apellidos);
        free(nuevoUsuario.nombre_usuario);
        free(nuevoUsuario.contrasena);

        // No llamamos a menuUsuario, simplemente termina la función y vuelve al main
    }
}



//FUNCIONES PARA VOLUNTARIO

void apuntarseEvento(sqlite3 *db, int id_usuario) {
    int id_evento_elegido;
    char sql[500];
    char *error = 0;
    printf("\n--- APUNTARSE A UN EVENTO ---\n");

    //1. Mostrar eventos de los próximos 3 meses
    printf("\nEventos disponibles (próximos 3 meses):\n");

    char *sql_ver = "SELECT id_evento, descripcion, fecha_ini FROM Evento "
    "WHERE date(fecha_ini) BETWEEN date('now') AND date('now', '+3 months');";
    
    sqlite3_exec(db, sql_ver, callbackMostrarEventos, 0, &error);
    
    printf("----------------------------------------------------------\n");
    printf("Introduce el ID del evento (0 para cancelar): "); //qué quiere decir?
    scanf("%d", &id_evento_elegido);
    if (id_evento_elegido <= 0) return;

    //2. Comprobar que el usuario ya tiene un evento ese día
    int ya_ocupado = 0;
    sprintf(sql, 
        "SELECT COUNT(*) FROM Participaciones P "
        "JOIN Evento E1 ON P.id_evento = E1.id_evento "
        "WHERE P.id_usuario = %d AND date(E1.fecha_ini) = "
        "(SELECT date(fecha_ini) FROM Evento WHERE id_evento = %d);", 
        id_usuario, id_evento_elegido);

    sqlite3_exec(db, sql, callbackCheckFecha, &ya_ocupado, &error);
    //habrá que mirar después de cambiar la  base de datos
    if (ya_ocupado) {
        printf("\n¡ERROR! Ya tienes otro compromiso registrado para ese mismo día.\n");
        return;
    }

    //3. Si no tiene evento en el mismo día, hacemos inscripción
    sprintf(sql, "INSERT INTO Participaciones (id_usuario, id_evento) VALUES (%d, %d);", 
            id_usuario, id_evento_elegido);
    
    if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) {
        printf("\n¡Inscripción realizada con éxito! Nos vemos allí.\n");
    } else {
        printf("\nError al apuntarse: %s\n", error);
        sqlite3_free(error);
    }

}

//Función callbackMostrarEventos
int callbackMostrarEventos(void *data, int argc, char **argv, char **colName)
{
   printf("ID: %-4s | Tipo: %-10s | Fecha: %-16s | Desc: %s\n", 
           argv[0] ? argv[0] : "NULL", //ID
           argv[3] ? argv[3] : "NULL", //TIPO
           argv[2] ? argv[2] : "NULL", //Fecha inicio
           argv[1] ? argv[1] : "NULL"); //Descripcion
    return 0;
} //los signos de interrogacion son como escribir esto
/*if (argv[0] != NULL) {
    printf("%s", argv[0]);
} else {
    printf("?");
}
*/

//Función para que verifique fecha, el voluntario no podrá apuntarse a 2 eventos del mismo día
int callbackCheckFecha(void *data, int argc, char **argv, char **colName) {
    int *existe = (int *)data;
    if (atoi(argv[0]) > 0) {
        *existe = 1;
    }; // Si el COUNT es > 0, hay colisión
    return 0;
}

//función para consultar eventos a los que está apuntado
void consultarMisEventos(sqlite3 *db, int id_usuario) {
    char sql[600];
    char *error = 0;

    printf("\n--- CALENDARIO DE MIS EVENTOS ---\n");
    printf("%-5s | %-25s | %-20s\n", "ID", "DESCRIPCIÓN", "FECHA INICIO");
    printf("------------------------------------------------------------\n");

    // Usamos un JOIN para cruzar la tabla Participaciones con Evento
    sprintf(sql, 
        "SELECT E.id_evento, E.descripcion, E.fecha_ini "
        "FROM Evento E "
        "JOIN Participaciones P ON E.id_evento = P.id_evento "
        "WHERE P.id_usuario = %d "
        "ORDER BY E.fecha_ini ASC;", id_usuario);

    // Reutilizamos el callback que ya tenemos para mostrar eventos
    if (sqlite3_exec(db, sql, callbackMostrarEventos, 0, &error) != SQLITE_OK) {
        printf("Error al consultar tus eventos: %s\n", error);
        sqlite3_free(error);
    }
    
    printf("------------------------------------------------------------\n");
}


//Función apuntarse a un evento
void listarDonaciones(sqlite3 *db, int id_usuario) {
    sqlite3_stmt *stmt;
    // 0:tipo, 1:r.kilos, 2:c.tipo_comida, 3:c.kilos, 4:din.cantidad
    const char *sql = 
        "SELECT d.tipo, r.kilos, c.tipo_comida, c.kilos, din.cantidad "
        "FROM Donaciones d "
        "LEFT JOIN Ropa r ON d.id_donacion = r.id_donacion "
        "LEFT JOIN Comida c ON d.id_donacion = c.id_donacion "
        "LEFT JOIN Dinero din ON d.id_donacion = din.id_donacion "
        "WHERE d.id_donante = ? ORDER BY d.id_donacion DESC;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Error: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, id_usuario);

    printf("\n%-12s | %-45s\n", "TIPO", "DETALLES");
    printf("------------------------------------------------------------\n");

    int hay_donaciones = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        hay_donaciones = 1;
        int tipo = sqlite3_column_int(stmt, 0);

        switch (tipo) {
            case 2: // COMIDA
                // El nombre está en el 2 y los kilos en el 3
                printf("%-12s | %s - %.2f kg\n", 
                       "COMIDA", 
                       sqlite3_column_text(stmt, 2) ? (const char*)sqlite3_column_text(stmt, 2) : "Desconocido", 
                       sqlite3_column_double(stmt, 3));
                break;
            case 1: // DINERO
                // El dinero está en el 4
                printf("%-12s | Importe: %.2f€\n", 
                       "DINERO", 
                       sqlite3_column_double(stmt, 4));
                break;
            case 3: // ROPA
                // Solo mostramos los kilos que están en el índice 1
                printf("%-12s | %-15s - %.2f kg\n", 
                       "ROPA", 
                       "Ropa variada",
                       sqlite3_column_double(stmt, 1));
                break;
            default:
                printf("%-12s | Sin detalles específicos\n", "OTROS");
                break;
        }
    }

    if (!hay_donaciones) {
        printf("No se han encontrado donaciones.\n");
    }
    printf("------------------------------------------------------------\n");

    sqlite3_finalize(stmt);
}

void menuPrincipal(sqlite3 *db, int tipo, int id_usuario)
{

    int opcion;

    do {
        printf("\n======= MENU PRINCIPAL =======");

        // Mostrar opciones ESPECÍFICAS según el tipo
        if (tipo == VOLUNTARIO) { // tipo 0
            printf("\n1.Apuntarse a un evento");
            printf("\n2. Consultar calendario de mis eventos");  //despauntarse calendario barrun zaudela ingo deu
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

            printf("\n2. Consultar horarios");
        }
        
        printf("\n0. Cerrar sesión");
        printf("\nSeleccione una opción: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1:
            if(tipo == VOLUNTARIO) {
                apuntarseEvento(db, id_usuario);
            } else if(tipo == DONANTE) {
                donarDinero(db, id_usuario);
            } else if(tipo == BENEFICIARIO) {
                Beneficiario b_actualizada = guardarCondicionesBeneficiario();                                    // 2. Los mandamos a la base de datos para que el cambio sea real
                if (actualizarDatosBeneficiario(db, id_usuario, b_actualizada)) {
                    printf("\n---------------------------------------------------------");
                    printf("\n[SISTEMA] Tus condiciones se han actualizado correctamente en tu perfil.\n");
                }
            } 
                break;

            case 2:
                if(tipo == VOLUNTARIO) {
                    consultarMisEventos(db, id_usuario);
                } else if(tipo == DONANTE) {
                    donarComida(db, id_usuario); 
                } else if(tipo == BENEFICIARIO) {
                    //función consultar horarios, jakitteko noiz jun ber dun eta zer jasotzea
                }
                break;

            case 3:
                if(tipo == VOLUNTARIO) {
                    //función historiala azaltzeko de los eventos acudidos hasta ahora
                } else if(tipo == DONANTE) {
                    donarRopa(db, id_usuario); 
                } 
                break;
            
            case 4:
                if(tipo == DONANTE) {
                    listarDonaciones(db, id_usuario);
                }

        }
    } while (opcion != 0);
}





//FUNCIONES PARA BENEFICIARIO

//Ayuda de dinero
float calcularAyudaDinero(Beneficiario b) {
    float renta = b.ingresos - b.gastos;
    // Cubrimos el déficit mensual + un pequeño bono de emergencia
    return fabsf(renta) + 50.0f; //fabsf(valor absoluto)
}

//Ayuda alimentación semanal
void mostrarAyudaComida(Beneficiario b) {
    float totalArrozPasta = (b.num_adultos * 1.0f) + (b.num_ninos * 0.75f);
    float totalLegumbres   = (b.num_adultos + b.num_ninos) * 0.5f;
    float totalLeche       = (b.num_adultos * 2.0f) + (b.num_ninos * 4.0f);
    int totalConservas     = (b.num_adultos * 3) + (b.num_ninos * 2);

    printf("\n[ALIMENTACIÓN SEMANAL - RECOGIDA MARTES 18:00]");   //la recogida cómo lo gestionamos??
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

    printf("\n[VESTIMENTA SEMESTRAL - PRÓXIMA CAMPAÑA]");  //la fecha cómo la ponemos???
    if (b.num_ninos > 0) {
        printf("\n > NIÑOS/AS: %d camisetas, %d pantalones, %d sudaderas", 
                camNinos, panNinos, sudNinos);
    }
    if (b.num_adultos > 0) {
        printf("\n > ADULTOS: %d camisetas, %d pantalones", 
                camAdultos, panAdultos);
    }
}

//Evaluar la ayuda que necesita
void evaluarBeneficiario(Beneficiario b) {
    float renta = b.ingresos - b.gastos;
    
    // Cálculos de umbrales mensuales
    float gastoComidaMensual = (b.num_adultos * 150.0f) + (b.num_ninos * 140.0f);
    float gastoRopaMensual   = (b.num_adultos * 5.50f) + (b.num_ninos * 9.0f);
    float umbralTotal        = gastoComidaMensual + gastoRopaMensual;

    printf("\n===========================================");
    printf("\n      RESULTADO DEL ANÁLISIS SOCIAL");
    printf("\n===========================================");

    // Lógica de Escenarios
    if (renta > umbralTotal) {
        // --- AUTOSUFICIENTE ---
        printf("\nESTADO: Evaluación Finalizada -> Autosuficiente");
        printf("\nTras analizar tu renta disponible, el sistema indica que puedes cubrir");
        printf("\nlas necesidades básicas de alimentación y vestimenta por tu cuenta.");
        printf("\nPriorizamos nuestros recursos para casos en situación de mayor urgencia.");
        printf("\n-------------------------------------------");
        printf("\nSi tu situación económica cambia, puedes solicitar una nueva evaluación.");
    } 
    else if (renta >= gastoComidaMensual && renta <= umbralTotal) {
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

//ADMINISTRADOR
void listarUsuarios(sqlite3 *db) {
    sqlite3_stmt *stmt;
    char *sql = "SELECT id_usuario, nombre FROM Usuarios;";
    
    printf("\n--- LISTA ACTUAL DE USUARIOS ---\n");
    printf("%-10s %-20s\n", "ID", "NOMBRE");
    printf("-------------------------------\n");

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char *nombre = sqlite3_column_text(stmt, 1);
            printf("%-10d %-20s\n", id, nombre);
        }
    }
    sqlite3_finalize(stmt);
    printf("-------------------------------\n");
}

void crearEvento(sqlite3 *db)
{
    char descripcion[100];
    int tipo, limite;
    Fecha inicio, final;

    printf("\n--- Crear Evento ---\n");
    printf("Descripción: ");
    scanf(" %[^\n]", descripcion); // Para leer con espacios

    printf("Tipo de evento (0 = Ropa, 1 = Comida): ");
    scanf("%d", &tipo);

    printf("Límite de voluntarios: ");
    scanf("%d", &limite);
    
    printf("Fecha inicio (DD/MM/AAAA HH:MM): ");
    scanf("%d/%d/%d %d:%d", &inicio.dia, &inicio.mes, &inicio.anyo, &inicio.hora, &inicio.minutos);
    while (getchar() != '\n');
    printf("Fecha final (DD/MM/AAAA HH:MM): ");
    scanf("%d/%d/%d %d:%d", &final.dia, &final.mes, &final.anyo, &final.hora, &final.minutos);
    while (getchar() != '\n');

    char sql[300];
    sprintf(sql,
    "INSERT INTO Evento (tipo, descripcion, fecha_ini, fecha_fin, lim_voluntarios) "
    "VALUES (%d, '%s', '%04d-%02d-%02d %02d:%02d', '%04d-%02d-%02d %02d:%02d', %d);",
    tipo, 
    descripcion, 
    inicio.anyo, inicio.mes, inicio.dia, inicio.hora, inicio.minutos, // Fecha Inicio
    final.anyo, final.mes, final.dia, final.hora, final.minutos,    // Fecha Fin
    limite);
    char *error = 0;
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK)
    {
        printf("Error al crear evento: %s\n", error);   //berrize menure bueltatzie nahi deu??
        sqlite3_free(error);
    }
    else
    {
        printf("Evento registrado correctamente\n");
    }
}

//Usuariuei baja eman
void darBajaUsuario(sqlite3 *db) {
    int id_eliminar;
    char sql[200];
    char *error = 0;

    printf("\n--- DAR DE BAJA USUARIO ---\n");
    listarUsuarios(db);

    printf("Introduce ID del usuario para eliminar (0 para cancelar): ");
    scanf("%d", &id_eliminar);
    if (id_eliminar <= 0) {
        printf("Operación cancelada.\n");
        return;
    }

    int confirmar;
    printf("¿Estás seguro de eliminar usuario %d?\n0. No\n1. Sí\nSelección: ", id_eliminar);
    scanf("%d", &confirmar);
    if (confirmar != 1) {
        printf("Operación cancelada.\n");
        return;
    }

    
    sprintf(sql, "DELETE FROM Usuarios WHERE id_usuario = %d;", id_eliminar);


    
    if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) {
        printf("\n[ÉXITO] Usuario %d eliminado correctamente.\n", id_eliminar);
    } else {
        printf("Error al eliminar usuario: %s\n", error);
        sqlite3_free(error);
    }
}

// Borrar un evento
void borrarEvento(sqlite3 *db) {
    int id_evento;
    char sql[200];
    char *error = 0;

    printf("\n--- BORRAR EVENTO ---\n");

    
    printf("\nEventos existentes:\n");
    char *sql_ver = "SELECT id_evento, descripcion, fecha_ini, tipo FROM Evento;";
    sqlite3_exec(db, sql_ver, callbackMostrarEventos, 0, &error);
    printf("\n");

    printf("Introduce el ID del evento a borrar (0 para cancelar): ");
    scanf("%d", &id_evento);
    if (id_evento <= 0) {
        printf("Operación cancelada.\n");
        return;
    }

    int confirmar;
    printf("¿Estás seguro de borrar el evento %d?\n0. No\n1. Sí\nSelección: ", id_evento);
    scanf("%d", &confirmar);
    if (confirmar != 1) {
        printf("Operación cancelada.\n");
        return;
    }

    
    sprintf(sql, "DELETE FROM Participaciones WHERE id_evento = %d;", id_evento);
    sqlite3_exec(db, sql, 0, 0, &error);

   
    sprintf(sql, "DELETE FROM Evento WHERE id_evento = %d;", id_evento);
    if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) {
        printf("\n[ÉXITO] Evento %d borrado correctamente.\n", id_evento);
    } else {
        printf("Error al borrar evento: %s\n", error);
        sqlite3_free(error);
    }
}

// Admiñan menua
void menuAdministrador(sqlite3 *db) {
    int opcion;

    do {
        printf("\n======= MENU ADMINISTRADOR =======");
        printf("\n1. Crear evento");
        printf("\n2. Borrar evento");
        printf("\n3. Ver usuarios");
        printf("\n4. Dar de baja a un usuario");
        printf("\n0. Cerrar sesión");
        printf("\nSeleccione una opción: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1:
                crearEvento(db);
                break;
            case 2:
                borrarEvento(db);
                break;
            case 3:
                listarUsuarios(db);
                break;
            case 4:
                darBajaUsuario(db);
                break;
            case 0:
                printf("\nCerrando sesión...\n");
                break;
            default:
                printf("\nOpción no válida.\n");
                break;
        }
    } while (opcion != 0);
}