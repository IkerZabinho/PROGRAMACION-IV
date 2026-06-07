#include <iostream>
#include <string>
#include <cstring>
#include <windows.h>
#include <cmath>
#include "../Comun/protocolo.h"
#include "RedCliente.h" // Para enviarPeticionServidor
#include "../ComunPrueba/Clases.h"
#include "interfazBen.h"

using namespace std;
using namespace GestionONG;

// Función auxiliar local para evitar bloqueos del cin
void limpiarBufferLocal() {
    cin.clear();
    while (cin.get() != '\n');
}

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
        if (!(cin >> adultos)) { cout << "[!] Entrada inválida.\n"; limpiarBufferLocal(); continue; }
        cout << "Número de niños/as en casa: ";
        if (!(cin >> ninos)) { cout << "[!] Entrada inválida.\n"; limpiarBufferLocal(); continue; }
        
        cout << "  > ¿Deseas cambiar algún dato de los integrantes? (1: Sí / 0: No): ";
        cin >> correcto;
        limpiarBufferLocal();

        if (correcto == 1) cout << "[!] Reintentando integrantes...\n";
    } while (correcto != 0);

    // 2. INGRESOS
    do {
        cout << "\n> INGRESOS\n";
        cout << "Sueldo mensual total: ";
        if (!(cin >> sueldos)) { cout << "[!] Entrada inválida.\n"; limpiarBufferLocal(); continue; }
        cout << "Otras ayudas/pensiones: ";
        if (!(cin >> ayudas)) { cout << "[!] Entrada inválida.\n"; limpiarBufferLocal(); continue; }
        
        cout << "  > ¿Deseas cambiar algún dato de los ingresos? (1: Sí / 0: No): ";
        cin >> correcto;
        limpiarBufferLocal();

        if (correcto == 1) cout << "[!] Reintentando ingresos...\n";
    } while (correcto != 0);

    // 3. GASTOS
    do {
        cout << "\n> GASTOS\n";
        cout << "Alquiler o hipoteca: ";
        if (!(cin >> alquiler)) { cout << "[!] Entrada inválida.\n"; limpiarBufferLocal(); continue; }
        cout << "Luz, agua y gas: ";
        if (!(cin >> suministros)) { cout << "[!] Entrada inválida.\n"; limpiarBufferLocal(); continue; }
        cout << "Material escolar: ";
        if (!(cin >> material_escolar)) { cout << "[!] Entrada inválida.\n"; limpiarBufferLocal(); continue; }
        cout << "Gastos en estudios: ";
        if (!(cin >> estudios)) { cout << "[!] Entrada inválida.\n"; limpiarBufferLocal(); continue; }
        cout << "Otros gastos: ";
        if (!(cin >> otros)) { cout << "[!] Entrada inválida.\n"; limpiarBufferLocal(); continue; }

        cout << "  > ¿Deseas cambiar algún dato de los gastos? (1: Sí / 0: No): ";
        cin >> correcto;
        limpiarBufferLocal();

        if (correcto == 1) cout << "[!] Reintentando gastos...\n";
    } while (correcto != 0);

    float ingresos_totales = sueldos + ayudas;
    float gastos_totales = alquiler + suministros + material_escolar + estudios + otros;

    // Crear el objeto beneficiario con los datos recopilados (Uso explícito del enum para evitar ambigüedad)
    Beneficiario b(0, "", "", "", "", GestionONG::BENEFICIARIO, adultos, ninos, ingresos_totales, gastos_totales);
    b.evaluarBeneficiario();

    return b;
}

// Envía los nuevos datos al servidor para que actualice la BD remota
int actualizarDatosBeneficiario(int socketServidor, int id_beneficiario, const GestionONG::Beneficiario& b) {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed)); 

    // Configuramos la operación indicando que modificaremos las condiciones de este ID existente
    paquete.tipoOperacion = OP_REGISTRO_BENEFICIARIO; 
    paquete.idUsuario = id_beneficiario;

    // Mapeamos las propiedades del objeto b al paquete de red
    paquete.economia.sueldo = b.getIngresos(); 
    paquete.economia.otros_gastos = b.getGastos();
    paquete.economia.adultos = b.getNumAdultos();
    paquete.economia.ninos = b.getNumNinos();

    // Usamos enviarPeticionServidor que gestiona el ciclo de vida del socket automáticamente
    PaqueteRed respuesta = enviarPeticionServidor(paquete);
    
    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        return 1; // Éxito
    }

    return 0; // Error
}

void verProximoRepartoComida(int socketServidor) {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    paquete.tipoOperacion = OP_CONSULTAR_EVENTOS; 
    paquete.idEvento = 999; // Flag indicador acordado para Comida

    PaqueteRed respuesta = enviarPeticionServidor(paquete);
    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        printf("%s\n", respuesta.mensajeRespuesta);
    } else {
        printf("[!] No se pudo obtener el horario de reparto de comida.\n");
    }
}

void verProximoRepartoRopa(int socketServidor, int id_perfil) {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    paquete.tipoOperacion = OP_CONSULTAR_EVENTOS;
    paquete.idUsuario = id_perfil;
    paquete.idEvento = 888; // Flag indicador acordado para Ropa

    // Corregida la asignación accidental previa
    PaqueteRed respuesta = enviarPeticionServidor(paquete);
    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        printf("%s\n", respuesta.mensajeRespuesta);
    } else {
        printf("[!] No se pudo obtener el horario de reparto de ropa.\n");
    }
}

void verTalleresProximos(int socketServidor) {
    printf("\n--- CONSULTANDO PRÓXIMOS TALLERES Y EVENTOS ---\n");
    
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    paquete.tipoOperacion = OP_VER_EVENTOS_DISPONIBLES; 

    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        // Imprime la tabla de texto construida dinámicamente por la base de datos remota
        printf("%s\n", respuesta.mensajeRespuesta);
    } else {
        printf("[!] Error al recuperar los talleres: %s\n", respuesta.mensajeRespuesta);
    }
}

// ============================================================================
// FUNCIONES AUXILIARES MATEMÁTICAS (Se quedan en el cliente, locales)
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