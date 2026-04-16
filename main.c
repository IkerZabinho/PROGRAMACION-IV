#include <stdio.h>
#include <windows.h>
#include "sqlite3.h"
#include "funciones.h"
#include "estructuras.h"

// Función auxiliar para limpiar el rastro del teclado
// Evita bucles infinitos si el usuario mete letras en un scanf de números

int main() {
    sqlite3 *db=NULL;
    Config miConfig;
    // Intentar cargar el fichero
    if (!cargar_configuracion("config.conf", &miConfig)) {
        printf("Error: No se encontró el archivo de configuración.\n");
        return 1;
    }

    // Ahora usas los datos parametrizados
    printf("Iniciando servidor con usuario: %s\n", miConfig.admin_user);

    
    // Usamos la ruta del archivo de configuración
    if (sqlite3_open(miConfig.db_path, &db) != SQLITE_OK) {
        printf("Error al abrir la BD definida en config: %s\n", miConfig.db_path);
        return 1;
    }

    // Al crear el resumen, usamos el nombre del archivo del config
    FILE *archivo = fopen(miConfig.report_name, "w");
    // Configuración para que se vean bien las tildes y la Ñ en Windows
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    if (sqlite3_open("asociacion.db", &db) != SQLITE_OK) {
        printf("[!] ERROR CRÍTICO: No se pudo abrir la base de datos.\n");
        return 1;
    }

    // Activar claves foráneas (importante para que los DELETE funcionen en cascada)
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, 0);

    // Mantenimiento automático al arrancar
    crearEventoMartesAutomatico(db);
    asegurarEventoRopa(db);

    int opcion;
    do {
        printf("\n==============================");
        printf("\n   SISTEMA DE GESTIÓN ONG");
        printf("\n==============================");
        printf("\n1. Iniciar Sesión");
        printf("\n2. Registrarse");
        printf("\n0. Salir");
        printf("\n------------------------------");
        printf("\nSeleccione una opción: ");
        // Validación de entrada numérica
        if (scanf("%d", &opcion) != 1) {
            printf("\n[!] Ups, parece que no has introducido un número.");
            printf("\nPor favor, elige una opción del 0 al 2.\n");
            while (getchar() != '\n');
            opcion = -1;     // Valor neutro para repetir el bucle
            continue;
        }
         // Limpiamos el 'Enter' sobrante
        while (getchar() != '\n');
        switch (opcion) {
            case 1:
                iniciarSesion(db); // Dentro de esta función deberías usar la misma lógica de reintentos
                break;
            case 2:
                registrarUsuario(db);
                break;
            case 0:
                printf("\nGenerando reporte en %s...", miConfig.report_name);
                generarReporteResumen(db, miConfig.report_name);
                printf("\nGracias por usar el sistema. ¡Hasta pronto!\n");
                break;
            default:
                printf("\n[?] La opción '%d' no existe en el menú. Inténtalo de nuevo.\n", opcion);
                break;
        }

    } while (opcion != 0);

   
    printf("Reporte guardado como: %s\n", miConfig.report_name);
    
    fclose(archivo);
    sqlite3_close(db);
    return 0;
}