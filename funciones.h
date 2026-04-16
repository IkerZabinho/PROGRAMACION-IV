#ifndef FUNCIONES_H
#define FUNCIONES_H
#include "estructuras.h"
#include "sqlite3.h"


int cargar_configuracion(const char *filename, Config *conf) ;
void generarReporteResumen(sqlite3 *db, const char *nombreArchivo);
int callback_escribir_fichero(void *data, int argc, char **argv, char **azColName);

// --- GESTIÓN DE USUARIOS ---
int insertarUsuario(sqlite3 *db, Usuario u, void *datosEspecificos);
int insertarDatosBeneficiario(sqlite3 *db, Beneficiario b);
int insertarDatosVoluntario(sqlite3 *db, Voluntario v);
int insertarDatosDonante(sqlite3 *db, Donante d);
int comprobarLogin(sqlite3 *db, char *user, char *pass, Usuario *u_sesion);
void registrarUsuario(sqlite3 *db);
void iniciarSesion(sqlite3 *db);
void darBajaUsuario(sqlite3 *db);
void listarUsuarios(sqlite3 *db);

// --- EVENTOS Y TALLERES ---
void crearEvento(sqlite3 *db);
void borrarEvento(sqlite3 *db);
void listarEventos(sqlite3 *db);
void verProximoRepartoComida(sqlite3 *db);
void verProximoRepartoRopa(sqlite3 *db, int id_beneficiario);
void verTalleresProximos(sqlite3 *db);
void apuntarseTaller(sqlite3 *db, int id_beneficiario);
void asignarVoluntarioTaller(sqlite3 *db);
void asegurarEventoRopa(sqlite3 *db);
void crearEventoMartesAutomatico(sqlite3 *db);

// --- DONACIONES ---
void donarDinero(sqlite3 *db, int id_donante);
void donarComida(sqlite3 *db, int id_donante);
void donarRopa(sqlite3 *db, int id_donante);
void listarDonaciones(sqlite3 *db, int id_donante);

// --- AYUDAS Y CÁLCULOS ---
float calcularAyudaDinero(Beneficiario b);
void mostrarAyudaComida(Beneficiario b);
void mostrarAyudaRopa(Beneficiario b);

// --- MENÚS ---
void menuPrincipal(sqlite3 *db, int tipo, int id_perfil);
void menuAdministrador(sqlite3 *db);

// --- AUXILIARES ---
int buscarIdEspecifico(sqlite3 *db, int id_usuario, int tipo);
void registrarRecogidaRopa(sqlite3 *db, int id_beneficiario, int id_evento);
void registrarRecogidaRopaAdmin(sqlite3 *db);


void mostrarProximaRecogida(sqlite3 *db, int material);
int tieneChoqueDeFechas(sqlite3 *db, int id_v, int id_e);
int leer_y_validar_fecha(const char *mensaje, Fecha *f);
void listarEventosRopaFuturos(sqlite3 *db);
int estaEventoLleno(sqlite3 *db, int id_e);
int comparar_fechas(Fecha f1, Fecha f2);

void registrarRecogidaRopa(sqlite3 *db, int id_beneficiario, int id_evento);
void registrarRecogidaRopaInterfaz(sqlite3 *db);
void registrarRecogidaRopaAdmin(sqlite3 *db);
void listarBeneficiarios(sqlite3 *db);
int es_bisiesto(int a);
void verProximaRecogidaRopa(sqlite3 *db, int id_voluntario);


#endif