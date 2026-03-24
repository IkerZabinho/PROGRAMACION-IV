#include <stdio.h>
#include "estructuras.h"
#include <stdbool.h>

// MENU PRINCIPAL
void menuPrincipal(){
   int opcion;
}

void menuPrincipal() {
    int opcion;
    do {
        printf("\n=== SISTEMA DE GESTIÓN: ASOCIACIÓN DE AYUDA SOCIAL ===\n");
        printf("¿Estás registrado en el sistema?\n");
        printf("1. Sí (Iniciar sesión)\n");
        printf("2. No (Registrarme)\n");
        printf("3. Salir\n");
        printf("Elige una opción: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1:
                iniciarSesion();
                break;
            case 2:
                registrarUsuario();
                break;
            case 3:
                printf("Saliendo del sistema...\n");
                break;
            default:
                printf("Opción no válida. Inténtalo de nuevo.\n");
        }
    } while(opcion != 3);
}

void iniciarSesion() {
    char user[50];
    char pass[50];
    int intentos = 0;
    bool loginExitoso = false;

    printf("\n--- INICIAR SESIÓN ---\n");
    printf("Usuario: ");
    scanf("%s", user);
    printf("Contraseña: ");
    scanf("%s", pass);

    //si introduce todo bien convertir loginExitoso a true
    
    if (!loginExitoso) {
        int opcError;
        printf("\nError: Usuario o contraseña incorrectos.\n");
        printf("Pulsa 1 para reintentar\n");
        scanf("%d", &opcError);
        // volver a iniciar sesión

    } else {
        printf("\n¡Bienvenido %s!\n", user);
        //saltar a la siguiente pantalla vol, don, ben

    }
}

void registrarUsuario() {
    Usuario nuevoUsuario;
    int rolElegido;

    printf("\n--- REGISTRO DE NUEVO USUARIO ---\n");
    printf("Elige tu rol en la asociación:\n");
    printf("1. Administrador\n");
    printf("2. Voluntario\n");
    printf("3. Donante\n");
    printf("4. Beneficiario\n");
    printf("Rol (1-4): ");
    scanf("%d", &rolElegido);

    //nuevoUsuario.rol = (Rol)rolElegido;

    printf("Crea un nombre de usuario: ");
    //scanf("%s", nuevoUsuario.usuario);
    printf("Crea una contraseña: ");
    scanf("%s", nuevoUsuario.contrasena);

    printf("\n¡Registro completado con éxito!\n");
    //printf("Rol asignado: %d | Usuario: %s\n", nuevoUsuario.rol, nuevoUsuario.usuario);
}

void recuperarContrasena() {
    char user[50];
    printf("\n--- RECUPERACIÓN DE CONTRASEÑA ---\n");
    printf("Introduce tu nombre de usuario para recuperar la contraseña: ");
    scanf("%s", user);
    printf("Si el usuario '%s' existe en nuestra base de datos, te indicaremos cómo recuperarla.\n", user);
}