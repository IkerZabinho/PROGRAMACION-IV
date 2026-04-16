#include <stdio.h>
#include <windows.h>
#include "sqlite3.h"
#include "funciones.h"
#include "estructuras.h"

// Función auxiliar para limpiar el rastro del teclado
// Evita bucles infinitos si el usuario mete letras en un scanf de números

int main() {
    sqlite3 *db;
    
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
                printf("\nGracias por usar el sistema. ¡Hasta pronto!\n");
                break;
            default:
                printf("\n[?] La opción '%d' no existe en el menú. Inténtalo de nuevo.\n", opcion);
                break;
        }

    } while (opcion != 0);

    sqlite3_close(db);
    return 0;
}