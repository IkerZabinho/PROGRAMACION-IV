#include <iostream>
#include <string>
#include <cstring>
#include "../Comun/protocolo.h"
#include "RedCliente.h"      
#include "../ComunPrueba/Clases.h"
#include "interfazAdmin.h"

using namespace std;

// --- MENU ADMINISTRADOR EN EL CLIENTE ---
void menuAdministrador(int id_perfil) {
    int opcion;
    do {
        cout << "\n==================================";
        cout << "\n   PANEL DE ADMINISTRACIÓN (CLIENTE)";
        cout << "\n==================================";
        cout << "\n1. Crear nuevo evento";
        cout << "\n2. Gestionar eventos (Borrar)";
        cout << "\n3. Listar usuarios registrados";
        cout << "\n4. Dar de baja a un usuario";
        cout << "\n5. Registrar recogida de ropa"; 
        cout << "\n6. Asignar voluntario a taller";
        cout << "\n0. Cerrar sesión";
        cout << "\n----------------------------------";
        cout << "\nSeleccione una opción: ";

        if (!(cin >> opcion)) {
            cout << "\n[!] Por favor, introduce un número válido.\n";
            cin.clear();
            while (cin.get() != '\n');
            opcion = -1;
            continue;
        }
        while (cin.get() != '\n'); // Limpiar buffer

        switch(opcion) {
            case 1: crearEventoCliente(); break;
            case 2: borrarEventoCliente(); break;
            case 3: listarUsuariosCliente(); break;
            case 4: darBajaUsuarioCliente(); break;
            case 5: registrarRecogidaRopaAdminCliente(); break; // 🆕 Llamada red
            case 6: crearTallerCliente(); break;
            case 0: cout << "\nCerrando sesión administrativa.\n"; break;
            default: cout << "\nOpción no válida.\n"; break;
        }
    } while (opcion != 0);
}


// ============================================================================
// 1. CREAR NUEVO EVENTO
// ============================================================================
void crearEventoCliente() {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    paquete.tipoOperacion = OP_CREAR_EVENTO;

    cout << "\n--- CREAR EVENTO ---" << endl;
    cout << "Material / Nombre del evento: ";
    cin.getline(paquete.admin.nombre_taller_o_material, 50);

    cout << "Descripción del evento: ";
    cin.getline(paquete.admin.descripcion, 150);

    cout << "Tipo de evento (ej: Entrega, Recogida): ";
    string tipoAux;
    getline(cin, tipoAux);
    strncpy(paquete.perfil.nombre, tipoAux.c_str(), 49);

    cout << "Límite de voluntarios: ";
    cin >> paquete.admin.cupo_o_limite;
    cin.ignore();

    GestionONG::Fecha f_inicio, f_final;
    while (true) {
        if (f_inicio.leer_y_validar_fecha("Introduce fecha inicio (DD-MM-AAAA HH:MM): ", &f_inicio)) break;
        cout << "[!] Fecha de inicio inválida.\n";
    }

    while (true) {
        if (f_final.leer_y_validar_fecha("Introduce fecha final (DD-MM-AAAA HH:MM): ", &f_final)) {
            if (f_final.comparar_fechas(f_final, f_inicio) > 0) break;
            cout << "[!] Error: La fecha final debe ser posterior a la de inicio.\n";
        }
    }

    paquete.admin.f_inicio.dia  = f_inicio.dia;
    paquete.admin.f_inicio.mes  = f_inicio.mes;
    paquete.admin.f_inicio.anyo = f_inicio.anyo;
    paquete.admin.f_inicio.hora = f_inicio.hora;

    paquete.admin.f_final.dia  = f_final.dia;
    paquete.admin.f_final.mes  = f_final.mes;
    paquete.admin.f_final.anyo = f_final.anyo;
    paquete.admin.f_final.hora = f_final.hora;

    cout << "\n[Red] Enviando datos de evento...\n";
    PaqueteRed respuesta = enviarPeticionServidor(paquete);
    cout << respuesta.mensajeRespuesta << endl;
}

// 2. GESTIONAR EVENTOS (BORRAR)
// ============================================================================
void borrarEventoCliente() {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    paquete.tipoOperacion = OP_BORRAR_EVENTO;

    cout << "\n--- ELIMINAR EVENTO ---" << endl;
    cout << "Introduce el ID del evento a borrar: ";
    cin >> paquete.idEvento;
    cin.ignore();

    cout << "\n[Red] Eliminando evento " << paquete.idEvento << "...\n";
    PaqueteRed respuesta = enviarPeticionServidor(paquete);
    cout << respuesta.mensajeRespuesta << endl;
}

// ============================================================================
// 3. LISTAR USUARIOS REGISTRADOS
// ============================================================================
void listarUsuariosCliente() {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    paquete.tipoOperacion = OP_LISTAR_USUARIOS;

    cout << "\n--- LISTADO DE USUARIOS ---" << endl;
    PaqueteRed respuesta = enviarPeticionServidor(paquete);
    cout << respuesta.mensajeRespuesta << endl;
}

// ============================================================================
// 4. DAR DE BAJA A UN USUARIO
// ============================================================================
void darBajaUsuarioCliente() {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    paquete.tipoOperacion = OP_BAJA_USUARIO;

    cout << "\n--- DAR DE BAJA USUARIO ---" << endl;
    cout << "Introduce el ID del usuario a eliminar: ";
    cin >> paquete.idUsuario;
    cin.ignore();

    cout << "\n[Red] Dando de baja al usuario " << paquete.idUsuario << "...\n";
    PaqueteRed respuesta = enviarPeticionServidor(paquete);
    cout << respuesta.mensajeRespuesta << endl;
}

// ============================================================================
// 5. REGISTRAR RECOGIDA DE ROPA
// ============================================================================
void registrarRecogidaRopaAdminCliente() {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    paquete.tipoOperacion = OP_REGISTRAR_ROPA;

    cout << "\n--- REGISTRAR RECOGIDA DE ROPA ---\n";
    cout << "Introduce el ID del Beneficiario: ";
    cin >> paquete.idUsuario; 
    cout << "Introduce la cantidad de prendas: ";
    cin >> paquete.admin.cantidad_ropa; 
    cin.ignore();

    cout << "\n[Red] Registrando entrega de ropa...\n";
    PaqueteRed respuesta = enviarPeticionServidor(paquete);
    cout << respuesta.mensajeRespuesta << endl;
}

// ============================================================================
// 6. ASIGNAR VOLUNTARIO A TALLER / CREAR TALLER
// ============================================================================
void crearTallerCliente() {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    paquete.tipoOperacion = OP_CREAR_TALLER;

    cout << "\n--- CREAR NUEVO TALLER ---\n";
    cout << "Nombre del Taller: ";
    cin.getline(paquete.admin.nombre_taller_o_material, 50);
    cout << "Descripción detallada: ";
    cin.getline(paquete.admin.descripcion, 150);
    
    cout << "Introduce el ID del Voluntario (Profesor): ";
    cin >> paquete.idUsuario; 
    cin.ignore();

    GestionONG::Fecha fechaAux;
    while (true) {
        if (fechaAux.leer_y_validar_fecha("Introduce la fecha del taller (DD-MM-AAAA HH:MM): ", &fechaAux)) break;
        cout << "[!] Fecha inválida.\n";
    }

    paquete.admin.f_inicio.dia  = fechaAux.dia;
    paquete.admin.f_inicio.mes  = fechaAux.mes;
    paquete.admin.f_inicio.anyo = fechaAux.anyo;
    paquete.admin.f_inicio.hora = fechaAux.hora;

    cout << "\n[Red] Creando taller en el servidor...\n";
    PaqueteRed respuesta = enviarPeticionServidor(paquete);
    cout << respuesta.mensajeRespuesta << endl;
}
