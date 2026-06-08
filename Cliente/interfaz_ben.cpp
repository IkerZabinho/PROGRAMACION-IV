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


// ============================================================================
// MENU PRINCIPAL DEL BENEFICIARIO (Estructura emparejada con Voluntario)
// ============================================================================


void menuBeneficiario(int socketServidor, int id_perfil, const PaqueteRed& datosSesion) {
    int opBen;
    
    // Creamos el objeto local cargando los datos desde la sesión que nos pasa el main
    GestionONG::Beneficiario b;
    b.setIngresos(datosSesion.economia.sueldo);
    b.setNumAdultos(datosSesion.economia.adultos);
    b.setNumNinos(datosSesion.economia.ninos);
    b.setGastos(datosSesion.economia.otros_gastos); 

    do {
        printf("\n======= MENU PRINCIPAL BENEFICIARIO =======");
        printf("\n1. Cambiar condiciones económicas");
        printf("\n2. Consultar horarios para recoger ayudas");
        printf("\n3. Ver próximos talleres");
        printf("\n0. Volver al menú anterior / Cerrar sesión");
        printf("\n===========================================");
        printf("\nSeleccione una opción: ");
        
        if (!(cin >> opBen)) {
            printf("\n[!] Entrada inválida. Introduce un número.\n");
            limpiarBufferLocal();
            opBen = -1;
            continue;
        }
        limpiarBufferLocal();

        switch (opBen) {
            case 1:
                // 1. Modificamos los datos localmente con el formulario
                b = guardarCondicionesBeneficiario(b); 
                
                // 2. Enviamos estos nuevos datos al servidor para actualizar la BD remota
                printf("\n[Red] Actualizando condiciones en el servidor...\n");
                if (actualizarDatosBeneficiario(socketServidor, id_perfil, b)) {
                    extern PaqueteRed datosLoginGlobal; 
                    datosLoginGlobal.economia.sueldo = b.getIngresos();
                    datosLoginGlobal.economia.otros_gastos = b.getGastos();
                    datosLoginGlobal.economia.adultos = b.getNumAdultos();
                    datosLoginGlobal.economia.ninos = b.getNumNinos();

                    b.setIngresos(b.getIngresos());
                    b.setGastos(b.getGastos());
                    b.setNumAdultos(b.getNumAdultos());
                    b.setNumNinos(b.getNumNinos());
                    printf(">>> [OK] Datos económicos actualizados con éxito en el servidor. <<<\n");
                } else {
                    printf("[!] ERROR: No se pudieron guardar los cambios en el servidor.\n");
                }
                break;

            case 2:
                printf("\n--- CONSULTAR HORARIOS DE AYUDAS ---\n");
                {
                    PaqueteRed paquete;
                    memset(&paquete, 0, sizeof(PaqueteRed));
                    paquete.tipoOperacion = OP_CONSULTAR_EVENTOS; 
                    paquete.idUsuario = id_perfil;                
                    paquete.tipoUsuario = 3; // Rol de Beneficiario

                    printf("[Red] Enviando consulta de horarios de ayuda al servidor...\n");
                    PaqueteRed respuestaEventos = enviarPeticionServidor(paquete);

                    if (respuestaEventos.tipoOperacion == OP_RESPUESTA_OK) {
                        printf("%s\n", respuestaEventos.mensajeRespuesta);
                    } else {
                        printf("[ERROR] %s\n", respuestaEventos.mensajeRespuesta);
                    }
                }
                break;

            case 3:
                // Llamamos a la función de este mismo archivo pasándole el socket
                verTalleresProximos(socketServidor);
                break;
           
            case 0:
                printf("\nSaliendo del módulo de beneficiario...\n");
                break;
                
            default:
                printf("\n[?] Opción no válida.\n");
                break;
        }
    } while (opBen != 0);
}


// Función auxiliar local para evitar bloqueos del cin
void limpiarBufferLocal() {
    cin.clear();
    while (cin.get() != '\n');
}



// ============================================================================
// 1. GESTIÓN ECONÓMICA DE BENEFICIARIOS
// ============================================================================

// Cambia la cabecera en interfaz_ben.cpp e interfazBen.h para que reciba el objeto actual
Beneficiario guardarCondicionesBeneficiario(const GestionONG::Beneficiario& bActual) {
    int adultos = bActual.getNumAdultos();
    int ninos = bActual.getNumNinos();
    float sueldos = bActual.getIngresos(); 
    float gastos = bActual.getGastos();

    cout << "\n--- DETALLES ECONÓMICOS DEL BENEFICIARIO ---\n";


    int opcionInicial = -1;
    while (true) {
        cout << "¿Qué deseas hacer? (1: Modificar datos / 0: Volver al menú sin cambiar nada): ";
        if (cin >> opcionInicial) {
            cin.ignore(10000, '\n'); 

            if (opcionInicial == 0) {
                cout << "\n[!] Operación cancelada. Volviendo al menú principal...\n";
                
                GestionONG::Beneficiario bCancelado;
                bCancelado.setIngresos(-1.0f); 
                return bCancelado; 
            }
            else if (opcionInicial == 1) {
                break; 
            }
            else {
                cout << "[!] Error: Opción inválida. Introduce 1 o 0.\n";
            }
        } else {
            cout << "[!] Error: Entrada no numérica. Por favor, introduce un número (1 o 0).\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }


    int cambiar = -1;
    cout << "\n> INTEGRANTES DE LA FAMILIA\n";
    cout << "  Número de adultos actual: " << adultos << "\n";
    cout << "  Número de niños actual: " << ninos << "\n";
    
    while (true) {
        cout << "  > ¿Deseas cambiar algún dato de los integrantes? (Sí=1, No=0): ";
        if (cin >> cambiar) {
            cin.ignore(10000, '\n');
            if (cambiar == 1) {
                cout << "  Nuevo número de adultos en casa: "; cin >> adultos;
                cout << "  Nuevo número de niños en casa: "; cin >> ninos;
                cin.ignore(10000, '\n');
                break;
            } else if (cambiar == 0) {
                break; 
            } else {
                cout << "  [!] Error: Opción inválida. Introduce 1 o 0.\n";
            }
        } else {
            cout << "  [!] Error: Entrada no numérica.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }


    cout << "\n> INGRESOS Y GASTOS\n";
    cout << "  Ingresos actuales: " << sueldos << "€\n";
    cout << "  Gastos actuales: " << gastos << "€\n";
    
    while (true) {
        cout << "  > ¿Deseas cambiar los balances financieros? (Sí=1, No=0): ";
        if (cin >> cambiar) {
            cin.ignore(10000, '\n');
            if (cambiar == 1) {
                cout << "  Introduce nuevos ingresos mensuales totales (€): "; cin >> sueldos;
                cout << "  Introduce nuevos gastos mensuales totales (€): "; cin >> gastos;
                cin.ignore(10000, '\n');
                break;
            } else if (cambiar == 0) {
                break; 
            } else {
                cout << "  [!] Error: Opción inválida. Introduce 1 o 0.\n";
            }
        } else {
            cout << "  [!] Error: Entrada no numérica.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }

    GestionONG::Beneficiario b;
    b.setNumAdultos(adultos);
    b.setNumNinos(ninos);
    b.setIngresos(sueldos);
    b.setGastos(gastos);

    evaluarBeneficiario(b);

    return b;
}
// Envía los nuevos datos al servidor para que actualice la BD remota
int actualizarDatosBeneficiario(int socketServidor, int id_perfil, const GestionONG::Beneficiario& b) {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));
    
    paquete.tipoOperacion = OP_ACTUALIZAR_PERFIL; 
    paquete.idUsuario = id_perfil;
    paquete.tipoUsuario = 3; 

    paquete.economia.sueldo = b.getIngresos();
    paquete.economia.adultos = b.getNumAdultos();
    paquete.economia.ninos = b.getNumNinos();
    paquete.economia.otros_gastos = b.getGastos();

    // Aquí lo llamamos 'respuesta' porque es lo que nos devuelve el servidor
    PaqueteRed respuesta = enviarPeticionServidor(paquete); 
    
    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        return 1; 
    }
    return 0; 
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

void evaluarBeneficiario(const GestionONG::Beneficiario &b) {
    float renta = b.getIngresos() - b.getGastos();
   
    // Cálculos de umbrales mensuales inteligentes basados en los integrantes
    float gastoComidaMensual = (b.getNumAdultos() * 150.0f) + (b.getNumNinos() * 140.0f);
    float gastoRopaMensual = (b.getNumAdultos() * 5.50f) + (b.getNumNinos() * 9.0f);
    float umbralTotal = gastoComidaMensual + gastoRopaMensual;

    printf("\n===========================================");
    printf("\n       RESULTADO DEL ANÁLISIS SOCIAL");
    printf("\n===========================================");

    // Lógica avanzada de Escenarios
    if (renta > umbralTotal) {
        printf("\nESTADO: Evaluación Finalizada -> Autosuficiente");
        printf("\nTras analizar tu renta disponible, el sistema indica que puedes cubrir");
        printf("\nlas necesidades básicas de alimentación y vestimenta por tu cuenta.");
        printf("\nPriorizamos nuestros recursos para casos en situación de mayor urgencia.");
        printf("\n-------------------------------------------");
        printf("\nSi tu situación económica cambia, puedes solicitar una nueva evaluación.");
    }
    else if (renta >= gastoComidaMensual && renta <= umbralTotal) {
        printf("\nESTADO: Evaluación Finalizada -> Escenario A");
        printf("\nTras analizar tu renta disponible, consideramos que cubres la alimentación");
        printf("\nbásica, por lo tanto, recibirás apoyo específico en vestimenta.");
        printf("\n-------------------------------------------");
        mostrarAyudaRopa(b);
    }
    else if (renta > 0 && renta < gastoComidaMensual) {
        printf("\nESTADO: Evaluación Finalizada -> Escenario B");
        printf("\nTras analizar tu renta disponible, el sistema indica que necesitas apoyo");
        printf("\ntanto en alimentación semanal como en vestimenta semestral.");
        printf("\n-------------------------------------------");
        mostrarAyudaComida(b);
        mostrarAyudaRopa(b);
    }
    else {
        printf("\nESTADO: Evaluación Finalizada -> Escenario C");
        printf("\nTras analizar tu renta disponible, el sistema detecta una situación de");
        printf("\nemergencia. Recibirás ayuda económica, alimentación y vestimenta.");
        printf("\n-------------------------------------------");
        float dinero = calcularAyudaDinero(b);
        printf("\n > AYUDA ECONÓMICA: %.2f euros/mes", dinero);
        mostrarAyudaComida(b);
        mostrarAyudaRopa(b);
    }
    printf("\n===========================================\n");
}