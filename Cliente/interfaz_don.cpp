
#include <iostream>
#include <string>
#include <cstring>
#include <windows.h>
#include "../Comun/protocolo.h"
#include "RedCliente.h" 
#include "../ComunPrueba/Clases.h"
#include "interfazDon.h"

using namespace std;
void InterfazDonante::ejecutarMenu(int socketServidor, int id_perfil)
{
    int opcion;
    do
    {
        printf("\n======= MODULO DE DONACIONES =======");
        printf("\n1. Realizar donacion de dinero");
        printf("\n2. Realizar donacion de comida");
        printf("\n3. Realizar donacion de ropa");
        printf("\n4. Consultar historial de mis donaciones");
        printf("\n0. Volver al menu anterior / Cerrar sesion");
        printf("\n====================================");
        printf("\nSeleccione una opcion: ");

        if (!(cin >> opcion))
        {
            printf("\n[!] Entrada invalida. Introduce un numero.\\n");
            cin.clear();
            while (cin.get() != '\n')
                ;
            opcion = -1;
            continue;
        }
        while (cin.get() != '\n'); // Limpiar buffer

        switch(opcion) {
            case 1:
                donarDinero(socketServidor, id_perfil);
                break;
            case 2:
                donarComida(socketServidor, id_perfil);
                break;
            case 3:
                donarRopa(socketServidor, id_perfil);
                break;
            case 4:
                consultarHistorialDonaciones(socketServidor, id_perfil);
                break;
        }
    } while (opcion != 0);
}

void donarDinero(int socketServidor, int id_donante)
{
    float cantidad;
    int confirmar;

    printf("\n--- DONACIÓN DE DINERO ---\n");
    printf("Cantidad: ");

    if (!(cin >> cantidad) || cantidad <= 0)
    {
        printf("[!] Error en la cantidad introducida.\n");
        cin.clear();
        while (cin.get() != '\n')
            ; // Limpiar el buffer de entrada de letras o basura
        return;
    }
    while (cin.get() != '\n')
        ; // Limpiar buffer tras leer el float

    printf("¿Confirmar %.2f€? (1:Si / 0:No): ", cantidad);
    if (!(cin >> confirmar))
    {
        cin.clear();
        while (cin.get() != '\n')
            ;
        printf("Operacion cancelada por entrada incorrecta.\n");
        return;
    }
    while (cin.get() != '\n')
        ; // Limpiar enter sobrante

    if (confirmar == 1)
    {
        // Preparamos el paquete de red para el servidor
        PaqueteRed paquete;
        memset(&paquete, 0, sizeof(PaqueteRed));

        paquete.tipoOperacion = OP_DONACION_DINERO;
        paquete.idUsuario = id_donante;
        paquete.cantidadDonada = cantidad; // Pasamos la cantidad introducida

        printf("\n[Red] Procesando la donación con el servidor...\n");

        // Usamos la función modularizada del cliente para transacciones síncronas de red
        PaqueteRed respuesta = enviarPeticionServidor(paquete);

        // Mostramos el resultado retornado por la base de datos del servidor
        if (respuesta.tipoOperacion == OP_RESPUESTA_OK)
        {
            printf("%s", respuesta.mensajeRespuesta); // Imprime el [EXITO] o lo que mande el servidor
        }
        else
        {
            printf("[ERROR] %s\n", respuesta.mensajeRespuesta);
        }
    }
    else
    {
        printf("Donación cancelada.\n");
    }
}

void donarComida(int socketServidor, int id_donante) {
    int respuesta, seleccion;
    float kilosIntroducidos;
    const char *nombresCategorias[] = {"", "Carbohidratos", "Legumbres", "Conservas", "Lacteos", "Infantil"};

    printf("\n--- REALIZAR DONACION DE COMIDA ---\n");

    // 1. Selección de categoría usando flujos C++ (cin) con tus validaciones originales
    do {
        printf("Seleccione el tipo de alimento:\n");
        for(int i = 1; i <= 5; i++) {
            printf("%d. %s\n", i, nombresCategorias[i]);
        }
        printf("Seleccion: ");
        
        if (!(cin >> seleccion)) {
            printf("[!] Error: Introduce un numero.\n");
            cin.clear();
            while (cin.get() != '\n'); 
            seleccion = -1;
            continue;
        }
        while (cin.get() != '\n'); // Limpiar buffer

        if (seleccion < 1 || seleccion > 5) {
            printf("[!] Opcion no valida, elige entre 1 y 5.\n");
        }
    } while (seleccion < 1 || seleccion > 5);

    // 2. Validación de Kilogramos (Mantenemos tu lógica robusta 3f3 adaptada a cin)
    printf("Cantidad en kilogramos: ");
    if (!(cin >> kilosIntroducidos) || kilosIntroducidos <= 0) {
        printf("[!] Error: Debes introducir un peso válido mayor que 0.\n");
        cin.clear();
        while (cin.get() != '\n'); // Limpiar el buffer de entrada si meten letras
        return;
    }
    while (cin.get() != '\n'); // Consumir cualquier carácter sobrante (como el .6 de 4..6 o letras) de forma silenciosa

    // 3. Confirmación interactiva (Corregido: ahora se ejecuta una sola vez y lee bien la respuesta)
    printf("¿Confirmas donar %.2f kg de %s?\n0. No\n1. Si\nSeleccion: ",
           kilosIntroducidos, nombresCategorias[seleccion]);
    
    if (!(cin >> respuesta)) {
        cin.clear();
        while (cin.get() != '\n');
        return;
    }
    while (cin.get() != '\n'); // Limpiar el buffer tras el número de confirmación

    if (respuesta == 1) {
        // 4. Preparación y empaquetado seguro para el Servidor
        PaqueteRed paquete;
        memset(&paquete, 0, sizeof(PaqueteRed));
        
        // Mantenemos tus asignaciones intactas tal y como las tenías escritas
        paquete.tipoOperacion = OP_DONACION_COMIDA; 
        paquete.idUsuario = id_donante;
        paquete.cantidadDonada = kilosIntroducidos;   // Pasamos los kilos en el float
        paquete.idEvento = seleccion;                 // Reutilizamos este entero para pasar el subtipo/categoría (TipoComida)

        printf("\n[Red] Enviando registro de alimentos al servidor...\n");
        
        // Enviamos la petición y esperamos la respuesta del motor SQLite remoto del servidor
        PaqueteRed respuestaServidor = enviarPeticionServidor(paquete);

        // Imprimimos el resultado directo devuelto por el servidor
        if (respuestaServidor.tipoOperacion == OP_RESPUESTA_OK) {
            printf("%s", respuestaServidor.mensajeRespuesta);
        } else {
            printf("[ERROR] %s\n", respuestaServidor.mensajeRespuesta);
        }
    } else {
        printf("Operacion cancelada.\n");
    }
}


void donarRopa(int socketServidor, int id_donante) {
    float kilos;
    int respuesta;

    cout << "\n--- REALIZAR DONACIÓN DE ROPA ---\n";
    cout << "Cantidad en kilogramos: ";
    
    if (!(cin >> kilos) || kilos <= 0) {
        cout << "[!] Error: Debes introducir un peso válido mayor que 0.\n";
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }
    cin.ignore(10000, '\n');
    cout << "¿Confirmas donar " << kilos << " kg de ropa?\n0. No\n1. Sí\nSelección: ";
    cin >> respuesta;

    if (respuesta == 1) {
        // Preparación y empaquetado seguro para el Servidor
        PaqueteRed paquete;
        memset(&paquete, 0, sizeof(PaqueteRed));
        
        // Asignamos la operación del protocolo para ropa
        paquete.tipoOperacion = OP_DONACION_ROPA; 
        paquete.idUsuario = id_donante;
        
        // Mapeamos los kilogramos en el campo float reutilizable que definió tu compañera
        paquete.cantidadDonada = kilos; 

        printf("\n[Red] Enviando registro de ropa al servidor...\n");
        
        // Enviamos la petición y esperamos la respuesta del motor SQLite remoto del servidor
        PaqueteRed respuestaServidor = enviarPeticionServidor(paquete);

        // Imprimimos el resultado directo devuelto por el servidor ([ÉXITO] o [ERROR])
        if (respuestaServidor.tipoOperacion == OP_RESPUESTA_OK) {
            printf("%s", respuestaServidor.mensajeRespuesta);
        } else {
            printf("[ERROR] %s\n", respuestaServidor.mensajeRespuesta);
        }
    } else {
        cout << "Operación cancelada.\n";
    }
}

void consultarHistorialDonaciones(int socketServidor, int id_donante) {
    printf("\n--- HISTORIAL DE MIS DONACIONES ---\n");

    // 1. Preparamos el paquete de red para solicitar la consulta
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    
    // Usamos el código de operación definido en protocolo.h
    paquete.tipoOperacion = OP_CONSULTAR_DONACIONES; 
    paquete.idUsuario = id_donante;

    printf("\n[Red] Solicitando historial de donaciones...\n");
    
    // 2. Enviamos al servidor y capturamos el paquete con la tabla de texto ya armada
    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    // 3. Imprimimos el resultado directo (sea la tabla formateada o el mensaje de error/vacío)
    printf("%s", respuesta.mensajeRespuesta);

    printf("\nPresione Enter para volver...");
    cin.get(); // Absorbe posibles saltos de línea residuales en el búfer
}