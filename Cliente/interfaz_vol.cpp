
#include <iostream>
#include <string>
#include <cstring>
#include <windows.h>
#include "../Comun/protocolo.h" 
#include "RedCliente.h"      
#include "../ComunPrueba/Clases.h"
#include "interfazVol.h"

using namespace std;

bool cacheEventosValida = false;

void InterfazVoluntario::ejecutarMenu(int socketServidor, int id_perfil) {
    int opcion;
    do {
        printf("\n======= MODULO DE VOLUNTARIADO =======");
        printf("\n1. Apuntarse a un evento");
        printf("\n2. Consultar calendario de mis eventos");
        printf("\n3. Consultar historial de mi voluntariado");
        printf("\n0. Volver al menu anterior / Cerrar sesion");
        printf("\n======================================");
        printf("\nSeleccione una opcion: ");
        
        if (!(cin >> opcion)) {
            printf("\n[!] Entrada invalida. Introduce un numero.\n");
            cin.clear();
            while (cin.get() != '\n');
            opcion = -1;
            continue;
        }
        while (cin.get() != '\n'); 
        switch(opcion) {
            case 1:
                apuntarseEvento(socketServidor, id_perfil);
                break;
            case 2:
                consultarMisEventos(socketServidor, id_perfil);
                break;
            case 3:
                consultarHistorialEventos(socketServidor, id_perfil);
                break;
        }
    } while (opcion != 0);
}


void apuntarseEvento(int socketServidor, int id_voluntario) {
    printf("\n--- APUNTARSE A UN EVENTO ---\n");
    printf("\nEventos disponibles en los que aún no participas:\n");

    // FASE 1: Solicitar lista al servidor
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    
    paquete.tipoOperacion = OP_VER_EVENTOS_DISPONIBLES;
    paquete.idUsuario = id_voluntario;
   // ID positivo = historial voluntario

    // Enviamos la petición y capturamos la respuesta formateada
    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    // Imprimimos la respuesta directa (venga la lista OK o un mensaje de error/vacío)
    printf("%s", respuesta.mensajeRespuesta);

    // Si el servidor detectó que no hay eventos, salimos directamente
    if (respuesta.tipoOperacion == OP_RESPUESTA_ERROR) {
        return;
    }

    // Pedimos la interacción al usuario
    int id_evento_seleccionado;
    printf("Introduce el ID del evento (0 para cancelar): ");
    if (!(cin >> id_evento_seleccionado)) {
        printf("[!] Entrada no valida.\n");
        cin.clear();
        while (cin.get() != '\n'); // Limpiar buffer de entrada
        return;
    }
    while (cin.get() != '\n'); // Limpiar enter sobrante

    if (id_evento_seleccionado <= 0) {
        printf("Operacion cancelada.\n");
        return;
    }

    // FASE 2: Enviar el ID del evento seleccionado para verificar y guardar
    PaqueteRed paqueteInscripcion;
    memset(&paqueteInscripcion, 0, sizeof(PaqueteRed));
    
    paqueteInscripcion.tipoOperacion = OP_INSCRIBIR_EN_EVENTO;
    paqueteInscripcion.idUsuario = id_voluntario;
    paqueteInscripcion.idEvento = id_evento_seleccionado;

    printf("\n[Red] Enviando solicitud de inscripcion segura al servidor...\n");
    PaqueteRed respuestaFinal = enviarPeticionServidor(paqueteInscripcion);

    // Imprimimos el veredicto del servidor (Si hubo choque de fecha, si se llenó o si fue OK)
    printf("%s", respuestaFinal.mensajeRespuesta);

    cacheEventosValida = false;
}

// Rellenar esta sección dentro de interfaz_vol.cpp

void consultarMisEventos(int socketServidor, int id_voluntario) {
    printf("\n--- CALENDARIO DE MIS EVENTOS ---\n");

    // FASE 1: Pedir al servidor la lista de eventos en los que participa
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    
    paquete.tipoOperacion = OP_CONSULTAR_MIS_EVENTOS;
    paquete.idUsuario = id_voluntario;

    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    // Mostramos la tabla construida por el servidor o el mensaje de que no tiene eventos
    printf("%s", respuesta.mensajeRespuesta);
    printf("------------------------------------------------------------------------------------\n");

    // Si el servidor responde con un código que indica que no hay eventos (o error), salimos sin preguntar por bajas
    if (respuesta.tipoOperacion == OP_RESPUESTA_ERROR) {
        return;
    }

    // FASE 2: Lógica local de interacción para desapuntarse
    int opcion;
    printf("¿Quieres desapuntarte de alguno? (1: Sí / 0: No): ");
    if (!(cin >> opcion)) {
        cin.clear();
        while (cin.get() != '\n'); // Limpiar buffer si meten letras
        return;
    }
    while (cin.get() != '\n'); // Limpiar enter sobrante

    if (opcion == 1) {
        int id_borrar;
        printf("Introduce el ID del evento: ");
        if (!(cin >> id_borrar)) {
            cin.clear();
            while (cin.get() != '\n');
            printf("[!] ID no válido.\n");
            return;
        }
        while (cin.get() != '\n'); // Limpiar enter

        // Preparamos el paquete de red de cancelación
        PaqueteRed paqueteBaja;
        memset(&paqueteBaja, 0, sizeof(PaqueteRed));
        
        paqueteBaja.tipoOperacion = OP_DESAPUNTAR_EVENTO;
        paqueteBaja.idUsuario = id_voluntario;
        paqueteBaja.idEvento = id_borrar; // Enviamos el ID del evento a eliminar

        printf("\n[Red] Enviando solicitud de baja al servidor...\n");
        PaqueteRed respuestaBaja = enviarPeticionServidor(paqueteBaja);

        // Imprimimos el resultado final devuelto por el servidor ([OK] o [!])
        printf("%s", respuestaBaja.mensajeRespuesta);
    }
}

// Rellenar esta sección dentro de interfaz_vol.cpp

void consultarHistorialEventos(int socketServidor, int id_voluntario) {
    printf("\n--- HISTORIAL DE EVENTOS PASADOS ---\n");

    static char cacheHistorialEventos[2048] = ""; 

    // Si ya los descargamos antes y no nos hemos apuntado a nada nuevo, usamos la RAM
    if (cacheEventosValida) {
        printf("\n[CACHE CLIENTE] Mostrando eventos desde memoria local:\n");
        printf("%s\n", cacheHistorialEventos);
        return;
    }

    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    paquete.tipoOperacion = OP_CONSULTAR_EVENTOS;
    paquete.idUsuario = id_voluntario;
    paquete.idEvento = id_voluntario; // ID positivo = historial

    printf("\n[Red] Descargando historial de eventos desde el servidor...\n");
    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        strcpy(cacheHistorialEventos, respuesta.mensajeRespuesta);
        cacheEventosValida = true; // La caché ahora es válida
        printf("%s\n", cacheHistorialEventos);
    } else {
        printf("[ERROR] %s\n", respuesta.mensajeRespuesta);
    }
}