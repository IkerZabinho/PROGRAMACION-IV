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

using namespace std;

// Prototipos de las funciones adaptadas
void procesarLoginCliente();
void procesarRegistroCliente();
void ejecutarFormularioRegistroCliente();
int main() {
    // Configuración para que se vean bien las tildes y la Ñ en Windows (Tus líneas originales)
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

        // Tu misma validación de entrada numérica pero en estilo C++
        if (!(cin >> opcion)) {
            printf("\n[!] Ups, parece que no has introducido un número.");
            printf("\nPor favor, elige una opción del 0 al 2.\n");
            cin.clear(); // Limpia el error de cin
            while (cin.get() != '\n'); // Limpia el buffer del teclado
            opcion = -1; // Valor neutro para repetir el bucle
            continue;
        }
        
        // Limpiamos el 'Enter' sobrante
        while (cin.get() != '\n');

        switch (opcion) {
            case 1:
                // Antes llamabas a iniciarSesion(db). 
                // Ahora llamas a la función que pide los datos y los manda al servidor.
                procesarLoginCliente(); 
                break;
            case 2:
                // Antes llamabas a registrarUsuario(db).
                ejecutarFormularioRegistroCliente();
                break;
            case 0:
                // Como el reporte lo genera el servidor con la base de datos, 
                // el cliente simplemente se despide.
                printf("\nGracias por usar el sistema. ¡Hasta pronto!\n");
                break;
            default:
                printf("\n[?] La opción '%d' no existe en el menú. Inténtalo de nuevo.\n", opcion);
                break;
        }

    } while (opcion != 0);

    return 0;
}

// Esta función sustituye a tu antiguo "iniciarSesion(db)"
void procesarLoginCliente() {
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed)); // Limpiamos la estructura por seguridad
    
    paquete.tipoOperacion = OP_LOGIN; // Indicamos que queremos loguearnos

    string usuario, contrasena;
    cout << "\n--- INICIO DE SESIÓN ---\n";
    cout << "Usuario: ";
    getline(cin, usuario);
    cout << "Contraseña: ";
    getline(cin, contrasena);

    // Metemos los datos dentro del paquete de red
    strncpy(paquete.perfil.usuario, usuario.c_str(), sizeof(paquete.perfil.usuario) - 1);
    strncpy(paquete.perfil.contrasena, contrasena.c_str(), sizeof(paquete.perfil.contrasena) - 1);

    cout << "\n[Conectando] Enviando credenciales al servidor...\n";

    // LLAMADA REAL A LA RED:
    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    // ANALIZAR LA RESPUESTA DEL SERVIDOR
    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        cout << "\n>>> " << respuesta.mensajeRespuesta << " <<<\n";
        cout << "ID Usuario: " << respuesta.idUsuario << "\n";
        
        // REQUERIMIENTO 2: Si es Beneficiario (Rol 4 por ejemplo), guardamos su caché local
        if (respuesta.tipoUsuario == 3) { 
            cout << "--- CACHÉ LOCAL (Datos Económicos Guardados) ---\n";
            cout << "Sueldo: " << respuesta.economia.sueldo << "€\n";
            cout << "Alquiler: " << respuesta.economia.alquiler << "€\n";
            cout << "------------------------------------------------\n";
        }
        
        // Aquí podrías saltar a un "menu_interno_ong()" según el tipoUsuario...
        if (respuesta.tipoUsuario == 4) { // ADMINISTRADOR
            //menuAdministrador(NULL); //hay que traerlo desde el interfaz.cpp aquí o poner en otro cpp como el de voluntario y así
        } 
        else if (respuesta.tipoUsuario >= 1 && respuesta.tipoUsuario <= 3) { // VOLUNTARIO(1), DONANTE(2) o BENEFICIARIO(3)
            menuPrincipal(NULL, respuesta.tipoUsuario, respuesta.idUsuario);
        }
    } else {
        // Muestra el error que venga de la base de datos (Ej: "Contraseña incorrecta")
        cout << "\n[!] ERROR: " << respuesta.mensajeRespuesta << "\n";
    }
    cout << "\nPresione Enter para continuar...";
    cin.get();
}

void procesarRegistroUsuario() {
    cout << "\n--- REGISTRO DE NUEVO USUARIO ---\n";
    // Aquí pedirás si es Voluntario, Donante, etc., rellenarás el PaqueteRed
    // y lo enviarás al servidor de la misma manera.
    
    cout << "[Próximamente] Formulario de registro por red...\n";
    PaqueteRed paquete;
    memset(&paquete, 0, sizeof(PaqueteRed));

    std::cout << "\nSelecciona perfil: 1.Voluntario | 2.Donante | 3.Beneficiario: ";
    int eleccion;
    std::cin >> eleccion;
    std::cin.ignore();

    if (eleccion == 1) paquete.tipoOperacion = OP_REGISTRO_VOLUNTARIO;
    else if (eleccion == 2) paquete.tipoOperacion = OP_REGISTRO_DONANTE;
    else if (eleccion == 3) paquete.tipoOperacion = OP_REGISTRO_BENEFICIARIO;
    else { std::cout << "Opción inválida.\n"; return; }

    std::cout << "Nombre: "; std::cin.getline(paquete.perfil.nombre, 50);
    std::cout << "Apellidos: "; std::cin.getline(paquete.perfil.apellidos, 100);
    std::cout << "Usuario: "; std::cin.getline(paquete.perfil.usuario, 50);
    std::cout << "Contraseña: "; std::cin.getline(paquete.perfil.contrasena, 50);

    if (paquete.tipoOperacion == OP_REGISTRO_BENEFICIARIO) {
        std::cout << "Adultos en casa: "; std::cin >> paquete.economia.adultos;
        std::cout << "Niños en casa: "; std::cin >> paquete.economia.ninos;
        std::cout << "Sueldo mensual (€): "; std::cin >> paquete.economia.sueldo;
        std::cin.ignore();
    }

    std::cout << "\n[Red] Enviando registro al servidor...\n";
    // Aquí usas la función con la que envías los sockets en tu cliente (ej: enviarPeticion o enviarPaquete)
    PaqueteRed respuesta = enviarPeticionServidor(paquete); 

    std::cout << "\n>>> RESPUESTA SERVIDOR: " << respuesta.mensajeRespuesta << "\n";

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
    cin.ignore(); // Limpiar el buffer de entrada

    if (eleccion == 1) {
        paquete.tipoOperacion = OP_REGISTRO_VOLUNTARIO;
        paquete.tipoUsuario = 1; // Rol Voluntario
    } else if (eleccion == 2) {
        paquete.tipoOperacion = OP_REGISTRO_DONANTE;
        paquete.tipoUsuario = 2; // Rol Donante
    } else if (eleccion == 3) {
        paquete.tipoOperacion = OP_REGISTRO_BENEFICIARIO;
        paquete.tipoUsuario = 3; // Rol Beneficiario
    } else {
        cout << "[!] Opción inválida.\n";
        return;
    }

    // 1. Datos comunes (Estructura DatosPersonales perfil)
    cout << "\n--- DATOS PERSONALES ---\n";
    cout << "Nombre: "; cin.getline(paquete.perfil.nombre, 50);
    cout << "Apellidos: "; cin.getline(paquete.perfil.apellidos, 100);
    cout << "Nombre de usuario (Login): "; cin.getline(paquete.perfil.usuario, 50);
    cout << "Contraseña: "; cin.getline(paquete.perfil.contrasena, 50);

    // 2. Si eligió beneficiario, pedimos los detalles de vuestro Requerimiento 2
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
    
    // Llamas a tu función de envío por sockets
    PaqueteRed respuesta = enviarPeticionServidor(paquete);

    if (respuesta.tipoOperacion == OP_RESPUESTA_OK) {
        cout << "\n>>> ¡ÉXITO! " << respuesta.mensajeRespuesta << " <<<\n";
    } else {
        cout << "\n[!] ERROR EN REGISTRO: " << respuesta.mensajeRespuesta << "\n";
    }
}

// En el CLIENTE (por ejemplo, Cliente/interfaz_cliente.cpp o main_cliente.cpp)

void menuPrincipal(int socketServidor, int tipo, int id_perfil) {
    int opcion;
    do {
        printf("\n======= MENU PRINCIPAL =======");
        if (tipo == GestionONG::VOLUNTARIO) {
            menuVoluntario(socketServidor,id_perfil);
        }
        else if (tipo == GestionONG::DONANTE) {
            menuDonante( socketServidor,  id_perfil);
        }
        else if (tipo == GestionONG::BENEFICIARIO) {
            printf("\n1. Cambiar condiciones");
            printf("\n2. Consultar horarios para recoger ayudas");
            printf("\n3. Ver proximos talleres");
        }
       
        printf("\n0. Cerrar sesion");
        printf("\nSeleccione una opcion: ");
        scanf("%d", &opcion);
        fflush(stdin); // Limpiar buffer de entrada en C

        switch(opcion) {
            case 1:
                // Pasamos 'socketServidor' en lugar de 'db'
                if(tipo == GestionONG::VOLUNTARIO) apuntarseEvento(socketServidor, id_perfil);
                else if (tipo == GestionONG::DONANTE) donarDinero(socketServidor, id_perfil);
                else if (tipo == GestionONG::BENEFICIARIO) {
                    // El formulario de rellenar datos se queda local en el cliente
                    //GestionONG::Beneficiario b_actualizada = guardarCondicionesBeneficiario(); poner el metodo en la interfaz_ben
                    
                    // Esta función ahora enviará los datos de 'b_actualizada' por red usando el socket
                   /*  if (actualizarDatosBeneficiario(socketServidor, id_perfil, b_actualizada)) {
                        printf("\n---------------------------------------------------------");
                        printf("\n[SISTEMA] Tus condiciones se han actualizado correctamente.\n");
                    } */
                }
                break;

            case 2:
                // Tu nueva función ya adaptada a red que hicimos antes
                if(tipo == GestionONG::VOLUNTARIO) consultarMisEventos(socketServidor, id_perfil); 
                else if(tipo == GestionONG::DONANTE) donarComida(socketServidor, id_perfil);
                else if(tipo == GestionONG::BENEFICIARIO) {
                    printf("\nCONSULTAR HORARIOS DE AYUDAS\n");
                    // Estas funciones también se cambian para pedir los datos al servidor por red
                    //verProximoRepartoComida(socketServidor);
                    //verProximoRepartoRopa(socketServidor, id_perfil);
                }
                break;

            case 3:
                if(tipo == GestionONG::VOLUNTARIO) consultarHistorialEventos(socketServidor, id_perfil);
                else if(tipo == GestionONG::DONANTE) donarRopa(socketServidor, id_perfil);
                //else if(tipo == GestionONG::BENEFICIARIO) verTalleresProximos(socketServidor);
                break;
           
            case 4:
                //if (tipo == GestionONG::DONANTE) listarDonaciones(socketServidor, id_perfil);
                break;
        }
    } while (opcion != 0);
}



