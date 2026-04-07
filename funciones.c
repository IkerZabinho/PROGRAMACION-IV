#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "estructuras.h"
#include "sqlite3.h"
// hola

// Insertar beneficiarios

int insertarDatosBeneficiario(sqlite3 *db, long long id, float ing, float gas, int adu, int nin)
{
    char sql[400];
    sprintf(sql, "INSERT INTO Beneficiarios (id_usuario, ingresos, gastos, num_adultos, num_nino) "
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

// insertar usuarios
int insertarUsuario(sqlite3 *db, Usuario u)
{
    char sql[500];
    char *error = 0;

    // 1. RECOLECTAR DATOS ESPECÍFICOS PRIMERO (en memoria)
    float ing = 0, gas = 0;
    int adu = 0, nin = 0;            // Para beneficiario
    char *rolVoluntario = "General"; // Para voluntario

    if (u.tipoUsuario == BENEFICIARIO)
    {
        printf("Ingresos: ");
        scanf("%f", &ing);
        printf("Gastos: ");
        scanf("%f", &gas);
        printf("Adultos: ");
        scanf("%d", &adu);
        printf("Niños: ");
        scanf("%d", &nin);
    }
    else if (u.tipoUsuario == VOLUNTARIO)
    {
        int op;
        printf("1.Profesor 2.Entrenador 3.Cocinero 4.Repartidor: ");
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
        exitoEspecifico = insertarDatosBeneficiario(db, id, ing, gas, adu, nin);
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

// Callback login
int callbackLogin(void *data, int argc, char **argv, char **colName)
{
    int *encontrado = (int *)data;
    *encontrado = 1;
    return 0;
}

// Comprobar login
int comprobarLogin(sqlite3 *db, char *user, char *pass, int *tipo)  //TIPO ERABILI GABE DAO???
{
    char sql[300];
    int encontrado = 0;
    sprintf(sql, "SELECT * FROM Usuarios WHERE nombre_usuario='%s' AND contrasena='%s';", user, pass);

    char *error = 0;
    sqlite3_exec(db, sql, callbackLogin, &encontrado, &error);

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
void donarDinero(sqlite3 *db, int id_usuario)  //he puesto id aquí, para que otra persona no haga donacion en tu nombre
{
    double cantidad;
    char sql[200];
    char *error = 0;

    printf("\n--- REALIZAR DONACIÓN DE DINERO ---\n");

    printf("\nIntroduce la cantidad a donar: ");  //he añadido cantidad
    scanf("%f", &cantidad); 

    if (cantidad <= 0) {
        printf("La cantidad debe ser mayor que 0.\n");
        return;
    }

    sprintf(sql,
            "INSERT INTO Donaciones (id_usuario, tipo) VALUES (%d, 1);",
            id_usuario);

    if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) 
    {
        // PASO 2: Recuperar el ID que la base de datos acaba de generar para esta donación
        //para ponder id_donacion en la estrucutra dinero que crearemos
        long long id_generado = sqlite3_last_insert_rowid(db);

        // PASO 3: Insertar en la tabla 'Dinero' usando ese ID como referencia
        char sqlDinero[300];
        sprintf(sqlDinero, "INSERT INTO Dinero (id_donacion, cantidad) VALUES (%lld, %.2f);", 
                id_generado, cantidad);  //no se si poner lld, es porque caben más números que en un int

        if (sqlite3_exec(db, sqlDinero, 0, 0, &error) == SQLITE_OK) {
            printf("\nSe han registrado %.2f€ en la donación nº %lld.\n", cantidad, id_generado);
        } else {
            printf("Error al guardar la cantidad: %s\n", error);
            sqlite3_free(error);
        }
    }

}

// EVENTUAK

void crearEvento(sqlite3 *db)
{
    char descripcion[100];
    int tipo, limite;

    printf("\n--- Crear Evento ---\n");
    printf("Descripción: ");
    scanf(" %[^\n]", descripcion); // Para leer con espacios

    printf("Tipo de evento (0 = Ropa, 1 = Comida): ");
    scanf("%d", &tipo);

    printf("Límite de voluntarios: ");
    scanf("%d", &limite);

    char sql[300];
    sprintf(sql,
            "INSERT INTO Eventos (descripcion, tipo, lim_voluntarios) VALUES ('%s', %d, %d);",
            descripcion, tipo, limite);

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
    printf("\n--- INICIAR SESIÓN ---\n");

    int tipo;

    printf("\nUsuario: ");
    scanf("%s", user);

    printf("Contraseña: ");
    scanf("%s", pass);

    if (comprobarLogin(db, user, pass, &tipo))
    {
        printf("Bienvenido!\n");
        menuUsuario(db, tipo);
    }
    else
    {
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
    printf("Apellidos: ");
    scanf("%s", buffer);
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
        // SI SALE BIEN:
        printf("\nRegistro completado con éxito. Accediendo a tu menú...\n");

        // Liberamos memoria antes de entrar en otro bucle (menú)
        free(nuevoUsuario.nombre);
        free(nuevoUsuario.apellidos);
        free(nuevoUsuario.nombre_usuario);
        free(nuevoUsuario.contrasena);

        menuUsuario(db, nuevoUsuario.tipoUsuario);
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
            printf("\n1. Registrar o cambiar condiciones"); //iuel registrar condiciones registratzeakun derrigorra bezela jarri??
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
                    //falta registrar/cambiar usuario, funtzixo bat in ber da
                }
                break;

            case 2:
                if(tipo == VOLUNTARIO) {
                    //función para consultar calendario de mis eventos
                } else if(tipo == DONANTE) {
                    //función azaltzeko los siguientes eventos de recogida de comida
                } else if(tipo == BENEFICIARIO) {
                    //función consultar horarios, jakitteko noiz jun ber dun eta zer jasotzea
                }
                break;

            case 3:
                if(tipo == VOLUNTARIO) {
                    //función historiala azaltzeko de los eventos acudidos hasta ahora
                } else if(tipo == DONANTE) {
                    //función azaltzeko los siguientes eventos de recogidad de ropa
                } 
                break;
            
            case 4:
                if(tipo == DONANTE) {
                    //función historiala azaltzeko de las donaciones realizadas hasta ahora
                }

        }
    } while (opcion != 0);
}



//FUNCIONES PARA VOLUNTARIO

//Función callbackMostrarEventos
int callbackMostrarEventos(void *data, int argc, char **argv, char **colName)
{
    printf("\nID: %s | %s | Fecha: %s", 
           argv[0] ? argv[0] : "?", // id_evento
           argv[1] ? argv[1] : "?", // descripcion
           argv[2] ? argv[2] : "?"); // fecha_inicio
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

//Función apuntarse a un evento
void apuntarseEvento(sqlite3 *db, int id_usuario) {
    int id_evento_elegido;
    char sql[500];
    char *error = 0;
    printf("\n--- APUNTARSE A UN EVENTO ---\n");

    //1. Mostrar eventos de los próximos 3 meses
    printf("\nEventos disponibles (próximos 3 meses):\n");

    char *sql_ver = "SELECT id_evento, descripcion, fecha_inicio FROM Eventos "
    "WHERE fecha_inicio BETWEEN date('now') AND date('now', '+3 months');";
    
    sqlite3_exec(db, sql_ver, callbackMostrarEventos, 0, &error);
    
    printf("----------------------------------------------------------\n");
    printf("Introduce el ID del evento (0 para cancelar): "); //qué quiere decir?
    scanf("%d", &id_evento_elegido);
    if (id_evento_elegido <= 0) return;

    //2. Comprobar que el usuario ya tiene un evento ese día
    int ya_ocupado = 0;
    sprintf(sql, 
        "SELECT COUNT(*) FROM Participaciones P "
        "JOIN Eventos E1 ON P.id_evento = E1.id_evento "
        "WHERE P.id_usuario = %d AND E1.fecha_inicio = "
        "(SELECT fecha_inicio FROM Eventos WHERE id_evento = %d);", 
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

