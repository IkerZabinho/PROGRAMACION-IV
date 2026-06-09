// interfaz_Admin.cpp
#include <iostream>
#include <string>
#include <cstring>
#include "../Comun/protocolo.h"
#include "RedCliente.h"      
#include "../ComunPrueba/Clases.h"
#include "interfazAdmin.h"

using namespace std;

void InterfazAdmin::ejecutarMenu(int socketServidor, int id_perfil) {
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
        while (cin.get() != '\n'); 

        switch(opcion) {
            case 1: crearEventoCliente(); break;
            case 2: borrarEventoCliente(); break;
            case 3: listarUsuariosCliente(); break;
            case 4: darBajaUsuarioCliente(); break;
            case 5: registrarRecogidaRopaAdminCliente(); break;
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
    int opcionMaterial;
        while (true) {
            cout << "Material del evento (0 = Ropa, 1 = Comida): ";
            if (cin >> opcionMaterial) {
                if (opcionMaterial == 0) {
                    // Copiamos "Ropa" al campo de tu estructura (asumiendo que es un char[])
                    strncpy(paquete.admin.material, "Ropa", sizeof(paquete.admin.material) - 1);
                    break;
                } else if (opcionMaterial == 1) {
                    strncpy(paquete.admin.material, "Comida", sizeof(paquete.admin.material) - 1);
                    break;
                } else {
                    cout << "[!] Error: Opción inválida. Introduce 0 o 1.\n";
                }
            } else {
                // Por si el usuario introduce una letra por error
                cout << "[!] Error: Entrada no válida. Introduce un número (0 o 1).\n";
                cin.clear(); // Limpia el estado de error
                cin.ignore(10000, '\n');        
            }
        }
    cin.ignore(10000, '\n');

    cout << "Descripción del evento: "<<endl;
    cin.getline(paquete.admin.descripcion, 150);

    int opcionTipo;
        while (true) {
            cout << "Tipo de evento (0 = Recogida, 1 = Reparto): ";
            if (cin >> opcionTipo) {
                if (opcionTipo == 0) {
                    strncpy(paquete.perfil.nombre, "Recogida", sizeof(paquete.perfil.nombre) - 1);
                    break;
                } else if (opcionTipo == 1) {
                    strncpy(paquete.perfil.nombre, "Reparto", sizeof(paquete.perfil.nombre) - 1);
                    break;
                } else {
                    cout << "[!] Error: Opción inválida. Introduce 0 o 1.\n";
                }
            } else {
                cout << "[!] Error: Entrada no válida. Introduce un número (0 o 1).\n";
                cin.clear();
                cin.ignore(10000, '\n');
            }
        }
        cin.ignore(10000, '\n'); // Limpia el salto de línea residual
        
    cout << "Límite de voluntarios: ";
    cin >> paquete.admin.cupo_o_limite;
    cin.ignore();

    GestionONG::Fecha f_inicio, f_final;
    while (true) {
        if (f_inicio.leer_y_validar_fecha("Introduce fecha inicio (DD-MM-AAAA HH:MM): ", &f_inicio)) {
            break; // Fecha válida, salimos del bucle
        } else {
            cout << "[!] Error: Formato de fecha de inicio inválido o valores incorrectos.\n";
            // Nota: Si leer_y_validar_fecha no limpia internamente cin en caso de fallo, 
            // descomenta las siguientes dos líneas:
            // cin.clear();
            // cin.ignore(10000, '\n');
        }
    }

    // --- VALIDACIÓN FECHA FINAL ---
    while (true) {
        if (f_final.leer_y_validar_fecha("Introduce fecha final (DD-MM-AAAA HH:MM): ", &f_final)) {
            // Comprobamos que la fecha final sea posterior a la de inicio
            if (f_final.comparar_fechas(f_final, f_inicio) > 0) {
                break; // Fecha válida y posterior, salimos del bucle
            } else {
                cout << "[!] Error: La fecha final debe ser cronológicamente posterior a la de inicio.\n";
            }
        } else {
            cout << "[!] Error: Formato de fecha final inválido o valores incorrectos.\n";
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

   int confirmacion = -1;

    while (true) {
        cout << "\n¿Quieres crear el evento? (Si=1, No=0): ";
        
        
        if (cin >> confirmacion) {
            cin.ignore(10000, '\n');

            if (confirmacion == 1) {
                cout << "\n[Red] Enviando datos de evento...\n";
                PaqueteRed respuesta = enviarPeticionServidor(paquete);
                cout << respuesta.mensajeRespuesta << endl;
                break; 
            } 
            else if (confirmacion == 0) {
                cout << "\n[!] Operación cancelada. Volviendo al menú principal...\n";
                return; 
            } 
            else {
                
                cout << "[!] Error: Opción inválida. Introduce solo 1 para Sí o 0 para No.\n";
            }
        } 
        else {
            
            cout << "[!] Error: Entrada no numérica. Por favor, introduce un número (1 o 0).\n";
            cin.clear();          
            cin.ignore(10000, '\n'); 
        }
    }
}


// ============================================================================
// 2. GESTIONAR EVENTOS (BORRAR)
// ============================================================================
void borrarEventoCliente() {
    PaqueteRed paqueteListar;
    memset(&paqueteListar, 0, sizeof(PaqueteRed));
    paqueteListar.tipoOperacion = OP_LISTAR_EVENTOS;

    PaqueteRed respuestaLista = enviarPeticionServidor(paqueteListar);
    cout << respuestaLista.mensajeRespuesta << endl;
    
    PaqueteRed paqueteBorrar;
    memset(&paqueteBorrar, 0, sizeof(PaqueteRed));
    paqueteBorrar.tipoOperacion = OP_BORRAR_EVENTO;

    cout << "Introduce el ID del evento a eliminar (0 para cancelar): ";
    if (!(cin >> paqueteBorrar.idEvento) || paqueteBorrar.idEvento <= 0) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "[!] Operación cancelada.\n";
        return;
    }
    cin.ignore(10000, '\n'); 

  
    int confirmacion = -1;
    while (true) {
        cout << "¿Seguro que deseas eliminar el evento " << paqueteBorrar.idEvento << "? (Sí=1, No=0): ";
        if (cin >> confirmacion) {
            cin.ignore(10000, '\n'); 

            if (confirmacion == 1) {
                cout << "\n[Red] Enviando solicitud de eliminación para el evento " << paqueteBorrar.idEvento << "...\n";
                PaqueteRed respuestaBorrar = enviarPeticionServidor(paqueteBorrar);
                cout << respuestaBorrar.mensajeRespuesta << endl;
                break;
            } 
            else if (confirmacion == 0) {
                cout << "\n[!] Operación cancelada. Volviendo al menú principal...\n";
                return; 
            } 
            else {
                cout << "[!] Error: Opción inválida. Introduce solo 1 para Sí o 0 para No.\n";
            }
        } 
        else {
            cout << "[!] Error: Entrada no numérica. Por favor, introduce un número (1 o 0).\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
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
    
   
    int volver = -1;
    while (true) {
        cout << "Pulsa 0 para volver al menú: ";
        if (cin >> volver) {
            cin.ignore(10000, '\n');
            if (volver == 0) {
                break; 
            } else {
                cout << "[!] Error: Opción inválida. ";
            }
        } else {
            cout << "[!] Error: Entrada no numérica. ";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}

// ============================================================================
// 4. DAR DE BAJA A UN USUARIO
// ============================================================================
void darBajaUsuarioCliente() {
    PaqueteRed paqueteListar;
    memset(&paqueteListar, 0, sizeof(PaqueteRed));
    paqueteListar.tipoOperacion = OP_LISTAR_USUARIOS; 

    cout << "\n--- LISTA DE USUARIOS REGISTRADOS ---" << endl;
    PaqueteRed respuestaLista = enviarPeticionServidor(paqueteListar);
    
    cout << respuestaLista.mensajeRespuesta << endl;
    cout << "-------------------------------------\n" << endl;

    PaqueteRed paqueteBaja;
    memset(&paqueteBaja, 0, sizeof(PaqueteRed));
    paqueteBaja.tipoOperacion = OP_BAJA_USUARIO;

    cout << "--- DAR DE BAJA USUARIO ---" << endl;
    cout << "Introduce el ID del usuario a eliminar (0 para cancelar): ";
    if (!(cin >> paqueteBaja.idUsuario) || paqueteBaja.idUsuario <= 0) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "[!] Operación cancelada.\n";
        return;
    }
    cin.ignore(10000, '\n'); 

    
    int confirmacion = -1;
    while (true) {
        cout << "¿Seguro que deseas dar de baja al usuario " << paqueteBaja.idUsuario << "? (Sí=1, No=0): ";
        if (cin >> confirmacion) {
            cin.ignore(10000, '\n');

            if (confirmacion == 1) {
                cout << "\n[Red] Dando de baja al usuario " << paqueteBaja.idUsuario << "...\n";
                PaqueteRed respuestaBaja = enviarPeticionServidor(paqueteBaja);
                cout << respuestaBaja.mensajeRespuesta << endl;
                break;
            } 
            else if (confirmacion == 0) {
                cout << "\n[!] Operación cancelada. Volviendo al menú principal...\n";
                return; 
            } 
            else {
                cout << "[!] Error: Opción inválida. Introduce solo 1 para Sí o 0 para No.\n";
            }
        } 
        else {
            cout << "[!] Error: Entrada no numérica. Por favor, introduce un número (1 o 0).\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}
