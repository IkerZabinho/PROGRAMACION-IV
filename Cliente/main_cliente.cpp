#include <iostream>
#include <string>
#include <cstring>
#include <windows.h>
#include "../Comun/protocolo.h" // Para usar PaqueteRed y los enums
#include "RedCliente.h"      // Descomentar cuando implementen los sockets del cliente
//#include "../ComunPrueba/interfaz.h" //Para poder llamar a los menús de la interfaz
#include "../ComunPrueba/Clases.h"
#include "interfazVol.h"
#include "interfazDon.h"
#include "interfazBen.h"


using namespace std;

// Prototipos de las funciones adaptadas

void procesarLoginCliente();
void procesarRegistroCliente();
void ejecutarFormularioRegistroCliente();
void menuPrincipal(int tipo, int id_perfil); 

PaqueteRed datosLoginGlobal;

void limpiarBuffer() {
    cin.clear();
    while (cin.get() != '\n');
}

int main() {
    // Configuración para que se vean bien las tildes y la Ñ en Windows
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

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

        if (!(cin >> opcion)) {
            printf("\n[!] Ups, parece que no has introducido un número.\n");
            limpiarBuffer();
            opcion = -1; 
            continue;
        }
        limpiarBuffer();

        switch (opcion) {
            case 1:
                procesarLoginCliente();
                break;
            case 2:
                ejecutarFormularioRegistroCliente();
                break;
            case 0:
                printf("\nGracias por usar el sistema. ¡Hasta pronto!\n");
                break;
            default:
                printf("\n[?] La opción '%d' no existe.\n", opcion);
                break;
        }

    } while (opcion != 0);

    return 0;
}

void procesarLoginCliente() {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed)); 
    
    paquete.tipoOperacion = OP_LOGIN; 

    string usuario, contrasena;
    cout << "\n--- INICIO DE SESIÓN ---\n";
    cout << "Usuario: ";
    getline(cin, usuario);
    cout << "Contraseña: ";
    getline(cin, contrasena);

    strncpy(paquete.perfil.usuario, usuario.c_str(), sizeof(paquete.perfil.usuario) - 1);
    strncpy(paquete.perfil.contrasena, contrasena.c_str(), sizeof(paquete.perfil.contrasena) - 1);

    cout << "\n[Conectando] Enviando credenciales al servidor...\n";

    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        cout << "\n>>> " << respuesta.mensajeRespuesta << " <<<\n";
        cout << "ID Usuario: " << respuesta.idUsuario << "\n";
        
        
        // GUARDAMOS LOS DATOS REALES EN LA VARIABLE GLOBAL
        datosLoginGlobal = respuesta; 

        if (respuesta.tipoUsuario == 3) { 
            cout << "--- CACHÉ LOCAL (Datos Económicos Guardados) ---\n";
            // 'sueldo' contiene los ingresos totales calculados por el servidor
            cout << "Sueldo: " << respuesta.economia.sueldo << "€\n";
            // 'otros_gastos' contiene los gastos totales agregados calculados por el servidor
            cout << "Gastos: " << respuesta.economia.otros_gastos << "€\n"; 
            cout << "------------------------------------------------\n";
        }
        
        if (respuesta.tipoUsuario == 4) { 
            
            menuAdministrador(NULL); 
            return;
        } 
        else if (respuesta.tipoUsuario >= 1 && respuesta.tipoUsuario <= 3) { 
            menuPrincipal(respuesta.tipoUsuario, respuesta.idUsuario);
            return;
        }
    } else {
        cout << "\n[!] ERROR: " << respuesta.mensajeRespuesta << "\n";
        return;
    }
    cout << "\nPresione Enter para continuar...";
    cin.get();
}

void ejecutarFormularioRegistroCliente() {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));

    cout << "\n==========================================";
    cout << "\n           FORMULARIO DE REGISTRO";
    cout << "\n==========================================";
    cout << "\nSelecciona tu perfil:";
    cout << "\n1. Voluntario";
    cout << "\n2. Donante";
    cout << "\n3. Beneficiario (Solicitante de ayuda)";
    cout << "\n------------------------------------------";
    cout << "\nSelecciona una opción: ";
    
    int eleccion;
    cin >> eleccion;
    cin.ignore(); 

    if (eleccion == 1) {
        paquete.tipoOperacion = OP_REGISTRO_VOLUNTARIO;
        paquete.tipoUsuario = 1; 
    } else if (eleccion == 2) {
        paquete.tipoOperacion = OP_REGISTRO_DONANTE;
        paquete.tipoUsuario = 2; 
    } else if (eleccion == 3) {
        paquete.tipoOperacion = OP_REGISTRO_BENEFICIARIO;
        paquete.tipoUsuario = 3; 
    } else {
        cout << "[!] Opción inválida.\n";
        return;
    }

    cout << "\n--- DATOS PERSONALES ---\n";
    cout << "Nombre: "; cin.getline(paquete.perfil.nombre, 50);
    cout << "Apellidos: "; cin.getline(paquete.perfil.apellidos, 100);
    cout << "Nombre de usuario (Login): "; cin.getline(paquete.perfil.usuario, 50);
    cout << "Contraseña: "; cin.getline(paquete.perfil.contrasena, 50);

    if (paquete.tipoOperacion == OP_REGISTRO_BENEFICIARIO) {
        cout << "\n--- ESTUDIO ECONÓMICO (REQUERIMIENTO 2) ---\n";
        cout << "Número de adultos en el hogar: "; cin >> paquete.economia.adultos;
        cout << "Número de niños en el hogar: "; cin >> paquete.economia.ninos;
        cout << "Sueldo o ingresos mensuales (€): "; cin >> paquete.economia.sueldo;
        cout << "Otras ayudas mensuales (€): "; cin >> paquete.economia.otras_ayudas;
        cout << "Gasto mensual de alquiler/hipoteca (€): "; cin >> paquete.economia.alquiler;
        cout << "Gasto mensual de suministros (Luz, agua) (€): "; cin >> paquete.economia.suministros;
        cout << "Gasto mensual en estudios/colegio (€): "; cin >> paquete.economia.estudios;
        cout << "Otros gastos mensuales (€): "; cin >> paquete.economia.otros_gastos;
        cin.ignore();
    }

    cout << "\n[Red] Enviando paquete de registro seguro al servidor por TCP...\n";
    
    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        cout << "\n>>> ¡ÉXITO! " << respuesta.mensajeRespuesta << " <<<\n";
        GestionONG::Beneficiario bNuevo;
        bNuevo.setNumAdultos(paquete.economia.adultos);
        bNuevo.setNumNinos(paquete.economia.ninos);
        bNuevo.setIngresos(paquete.economia.sueldo);
        bNuevo.setGastos(paquete.economia.otros_gastos);

        evaluarBeneficiario(bNuevo);
    } else {
        cout << "\n[!] ERROR EN REGISTRO: " << respuesta.mensajeRespuesta << "\n";
    }
}

void menuPrincipal(int tipo, int id_perfil) {
    if (tipo == GestionONG::VOLUNTARIO) {
        menuVoluntario(0, id_perfil); 
        return;
    }
    else if (tipo == GestionONG::DONANTE) {
        menuDonante(0, id_perfil);    
        return;
    }
    
    if (tipo == GestionONG::BENEFICIARIO) {
        menuBeneficiario(0, id_perfil, datosLoginGlobal);
        return;
    }
}