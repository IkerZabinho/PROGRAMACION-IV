#ifndef INTERFAZ_H
#define INTERFAZ_H

#include "sqlite3.h"
#include "Clases.h"
#include <string>       

using std::string;     

namespace GestionONG {

        // Declaraciones avanzadas de menús y funciones de base de datos
    void menuAdministrador(sqlite3 *db);
    void menuPrincipal(sqlite3 *db, int tipoUsuario, int id_perfil);
    void listarUsuarios(sqlite3 *db);
    int eliminarUsuarioDB(sqlite3 *db, int id);
    int buscarIdEspecifico(sqlite3 *db, int id_usuario, int tipoUsuario);
    int insertarEvento(sqlite3 *db, const Evento& e);
    int insertarDonacionRopa(sqlite3 *db, const Ropa& r, int id_usuario);
    int insertarDonacionDinero(sqlite3 *db, const Dinero& d, int id_donante);
    int insertarDonacionComidaDB(sqlite3 *db, const Donacion& d, const Comida& c);
    void mostrarProximaRecogida(sqlite3 *db, int tipo);
    int es_bisiesto(int anyo);
    int comparar_fechas(const Fecha& f1, const Fecha& f2);
    void crearEventoJuevesRopaAutomatico(sqlite3 *db);
    void verProximoRepartoComida(sqlite3 *db);
    void verProximoRepartoRopa(sqlite3 *db, int id_beneficiario);
    void verTalleresProximos(sqlite3 *db);
    float calcularAyudaDinero(Beneficiario b);
    void mostrarAyudaComida(Beneficiario b);
    void mostrarAyudaRopa(Beneficiario b);
    int actualizarDatosBeneficiario(sqlite3 *db, int id_beneficiario, Beneficiario b);
    void verProximaRecogidaRopa(sqlite3 *db);
    void iniciarSesion(sqlite3 *db);
    void crearEventoMartesAutomatico(sqlite3 * db);
    void listarEventos(sqlite3 *db);
    void borrarEvento(sqlite3 *db);

    // Prototipo local
    int leer_y_validar_fecha(const string& mensaje, Fecha *f);

    // ============================================================================
    // 1. GESTIÓN ECONÓMICA DE BENEFICIARIOS
    // ============================================================================
    // Captura los datos económicos de un beneficiario desde la consola
    Beneficiario guardarCondicionesBeneficiario();

    // ============================================================================
    // 2. SISTEMA DE AUTENTICACIÓN Y REGISTRO
    // ============================================================================
    // Formulario interactivo para dar de alta a un Voluntario, Donante o Beneficiario
    void registrarUsuario(sqlite3 *db);

    // Formulario de login que conecta con la lógica de verificación
    void IniciarSesion(sqlite3 *db);

    // ============================================================================
    // 3. CAPA DE INTERFAZ PARA DONACIONES
    // ============================================================================
    void donarRopa(sqlite3 *db, int id_usuario);
    void donarDinero(sqlite3 *db, int id_donante);
    void donarComida(sqlite3 *db, int id_donante);

    // ============================================================================
    // 4. PARTICIPACIÓN EN EVENTOS Y TALLERES
    // ============================================================================
    // Muestra los eventos activos y permite al voluntario inscribirse
    void apuntarseEvento(sqlite3 *db, int id_voluntario);

    // Muestra los talleres y permite al beneficiario apuntarse
    void apuntarseTaller(sqlite3 *db, int id_beneficiario);

    // ============================================================================
    // 5. SECCIÓN DE ADMINISTRACIÓN (CREACIÓN)
    // ============================================================================
    void crearEvento(sqlite3 *db);
    void crearTaller(sqlite3 *db);

} // namespace GestionONG

#endif // INTERFAZ_H