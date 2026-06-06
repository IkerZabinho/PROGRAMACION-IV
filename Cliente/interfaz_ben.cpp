#include <iostream>
#include <string>
#include <cstring>
#include <windows.h>
#include "../Comun/protocolo.h"
#include "RedCliente.h" // Para enviarPeticionServidor
#include "../ComunPrueba/Clases.h"
#include "interfazBen.h"

using namespace std;

// ============================================================================
    // 1. GESTIÓN ECONÓMICA DE BENEFICIARIOS
    // ============================================================================

    Beneficiario guardarCondicionesBeneficiario() {
        int correcto;
        int adultos = 0, ninos = 0;
        float sueldos = 0, ayudas = 0, alquiler = 0, suministros = 0, material_escolar = 0, estudios = 0, otros = 0;

        cout << "\n--- DETALLES ECONÓMICOS DEL BENEFICIARIO ---\n";
        cout << "* Responde a la pregunta y pulsa enter para continuar.\n";

        // 1. INTEGRANTES
        do {
            cout << "\n> INTEGRANTES DE LA FAMILIA\n";
            cout << "Número de adultos en casa: ";
            cin >> adultos;
            cout << "Número de niños/as en casa: ";
            cin >> ninos;
            
            cout << "  > ¿Deseas cambiar algún dato de los integrantes? (1: Sí / 0: No): ";
            cin >> correcto;
            std::cin.ignore(10000, '\n');

            if (correcto == 1) cout << "[!] Reintentando integrantes...\n";
        } while (correcto != 0);

        // 2. INGRESOS
        do {
            cout << "\n> INGRESOS\n";
            cout << "Sueldo mensual total: ";
            cin >> sueldos;
            cout << "Otras ayudas/pensiones: ";
            cin >> ayudas;
            
            cout << "  > ¿Deseas cambiar algún dato de los ingresos? (1: Sí / 0: No): ";
            cin >> correcto;
            std::cin.ignore(10000, '\n');

            if (correcto == 1) cout << "[!] Reintentando ingresos...\n";
        } while (correcto != 0);

        // 3. GASTOS
        do {
            cout << "\n> GASTOS\n";
            cout << "Alquiler o hipoteca: ";
            cin >> alquiler;
            cout << "Luz, agua y gas: ";
            cin >> suministros;
            cout << "Material escolar: ";
            cin >> material_escolar;
            cout << "Gastos en estudios: ";
            cin >> estudios;
            cout << "Otros gastos: ";
            cin >> otros;

            cout << "  > ¿Deseas cambiar algún dato de los gastos? (1: Sí / 0: No): ";
            cin >> correcto;
            std::cin.ignore(10000, '\n');

            if (correcto == 1) cout << "[!] Reintentando gastos...\n";
        } while (correcto != 0);

        float ingresos_totales = sueldos + ayudas;
        float gastos_totales = alquiler + suministros + material_escolar + estudios + otros;

        Beneficiario b(0, "", "", "", "", 0, adultos, ninos, ingresos_totales, gastos_totales);
        b.evaluarBeneficiario();

        return b;
    }
   int actualizarDatosBeneficiario(int socketServidor, int id_beneficiario, const GestionONG::Beneficiario& b) {
    // 1. Crear el paquete de red que vamos a enviar
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed)); // Limpiar memoria por seguridad

    // 2. Configurar la operación e ID
    paquete.tipoOperacion = OP_REGISTRO_BENEFICIARIO; // Asegúrate de usar el enum correcto de tu Protocolo.h
    paquete.idUsuario = id_beneficiario;

    // 3. Mapear los datos del objeto 'b' a la estructura 'economia' del paquete
    // (Ajusta los nombres si en tu protocolo se llaman float sueldo/gastos en vez de ingresos)
    paquete.economia.sueldo = b.getIngresos(); 
    paquete.economia.otros_gastos = b.getGastos();
    paquete.economia.adultos = b.getNumAdultos();
    paquete.economia.ninos = b.getNumNinos();

    // 4. Enviar el paquete al servidor
    int bytesEnviados = send(socketServidor, (char*)&paquete, sizeof(PaqueteRed), 0);
    if (bytesEnviados == -1) { // Error al enviar (SOCKET_ERROR)
        return 0;
    }

    // 5. Esperar la respuesta del servidor para saber si todo salió bien en la BD
    int bytesRecibidos = recv(socketServidor, (char*)&paquete, sizeof(PaqueteRed), 0);
    if (bytesRecibidos > 0 && paquete.tipoOperacion == OP_RESPUESTA_OK) {
        return 1; // Éxito, el servidor guardó los datos correctamente
    }

    return 0; // Falló el registro o el servidor devolvió un error
}

void verProximoRepartoComida(int socketServidor) {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    
    // Dejamos su enum original
    paquete.tipoOperacion = OP_VER_EVENTOS_DISPONIBLES; 

    printf("\n--- PROXIMO REPARTO DE COMIDA ---\n");
    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        printf("%s", respuesta.mensajeRespuesta);
        printf("\nLos repartos de comida son CADA MARTES desde las 16:00 hasta las 20:00\n");
    } else {
        printf("[Servidor]: %s\n", respuesta.mensajeRespuesta);
    }
}

void verProximoRepartoRopa(int socketServidor, int id_beneficiario) {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    
    paquete.tipoOperacion = OP_CONSULTAR_EVENTOS; 
    paquete.idUsuario = id_beneficiario; // <-- ID positivo para indicarle que es ROPA

    printf("\n--- PROXIMO REPARTO DE ROPA ---\n");
    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        printf("%s", respuesta.mensajeRespuesta);
    } else {
        printf("[Servidor]: %s\n", respuesta.mensajeRespuesta);
    }
}

void verTalleresProximos(int socketServidor) {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    
    paquete.tipoOperacion = OP_CONSULTAR_EVENTOS; 
    paquete.idUsuario = -999; // 💡 ¡EL TRUCO! Si es -999, el servidor sabrá que son TALLERES

    printf("\n--- LISTA DE PROXIMOS TALLERES ---\n");
    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        printf("%s", respuesta.mensajeRespuesta);
    } else {
        printf("[Servidor]: %s\n", respuesta.mensajeRespuesta);
    }
}

// ============================================================================
// FUNCIONES AUXILIARES MATEMÁTICAS (Se quedan en el cliente, no usan BD)
// ============================================================================

float calcularAyudaDinero(Beneficiario b) {
    float renta = b.getIngresos() - b.getGastos();
    return abs(int(renta)) + 50.0f; 
}

void mostrarAyudaComida(Beneficiario b) {
    float totalArrozPasta = (b.getNumAdultos() * 1.0f) + (b.getNumNinos() * 0.75f);
    float totalLegumbres = (b.getNumAdultos() + b.getNumNinos()) * 0.5f;
    float totalLeche = (b.getNumAdultos() * 2.0f) + (b.getNumNinos() * 4.0f);
    int totalConservas = (b.getNumAdultos() * 3) + (b.getNumNinos() * 2);

    printf("\n[ALIMENTACION SEMANAL]");  
    printf("\n > Arroz/Pasta:        %.2f kg", totalArrozPasta);
    printf("\n > Legumbres:          %.2f kg", totalLegumbres);
    printf("\n > Leche:              %.0f litros", totalLeche);
    printf("\n > Conservas:          %d latas\n", totalConservas);
}

void mostrarAyudaRopa(Beneficiario b) {
    int camNinos = b.getNumNinos() * 3;
    int panNinos = b.getNumNinos() * 2;
    int sudNinos = b.getNumNinos() * 1;
    int camAdultos = b.getNumAdultos() * 2;
    int panAdultos = b.getNumAdultos() * 1;

    printf("\n[VESTIMENTA SEMESTRAL]"); 
    if (b.getNumNinos() > 0) {
        printf("\n > NINOS/AS: %d camisetas, %d pantalones, %d sudaderas", camNinos, panNinos, sudNinos);
    }
    if (b.getNumAdultos() > 0) {
        printf("\n > ADULTOS: %d camisetas, %d pantalones", camAdultos, panAdultos);
    }
    printf("\n");
}

// 2. CAMBIADA A SOCKET: Envía los nuevos datos al servidor para que actualice la BD
int actualizarDatosBeneficiario(int socketServidor, int id_beneficiario, Beneficiario b) {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));

    paquete.tipoOperacion = OP_REGISTRO_BENEFICIARIO; // Revisa tu enum para actualizar perfil
    paquete.idUsuario = id_beneficiario;

    // Pasamos los datos del objeto a la estructura del protocolo de red
    paquete.economia.sueldo = b.getIngresos();
    paquete.economia.otros_gastos = b.getGastos();
    paquete.economia.adultos = b.getNumAdultos();
    paquete.economia.ninos = b.getNumNinos();

    int bytesEnviados = send(socketServidor, (char*)&paquete, sizeof(PaqueteRed), 0);
    if (bytesEnviados == -1) return 0;

    int bytesRecibidos = recv(socketServidor, (char*)&paquete, sizeof(PaqueteRed), 0);
    if (bytesRecibidos > 0 && paquete.tipoOperacion == OP_RESPUESTA_OK) {
        return 1; // Éxito
    }
    return 0; // Falló
}