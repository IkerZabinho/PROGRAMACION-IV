#ifndef FUNCIONES_H
#define FUNCIONES_H
#include "estructuras.h"
#include "sqlite3.h"
/*
int insertarDatosBeneficiario(sqlite3 *db, long long id, float ing, float gas, int adu, int nin);
int insertarDatosVoluntario(sqlite3 *db, long long id, const char *rol);
int insertarUsuario(sqlite3 *db, Usuario u);

int callbackLogin(void *data, int argc, char **argv, char **colName);
int comprobarLogin(sqlite3 *db, char *user, char *pass, int *tipo, int *id_res);
int callbackMostrar(void *data, int argc, char **argv, char **colName);
void mostrarUsuarios(sqlite3 *db);

void mostrarProximaRecogida(sqlite3 *db, int material);
void donarDinero(sqlite3 *db, int id_usuario);
void donarComida(sqlite3 *db, int id_usuario);
void donarRopa(sqlite3 *db, int id_usuario);

void crearEvento(sqlite3 *db);

void iniciarSesion(sqlite3 *db);
void registrarUsuario(sqlite3 *db);

void menuPrincipal(sqlite3 *db, int tipo, int id_usuario);

int callbackCheckCupo(void *data, int argc, char **argv, char **colName);
int callbackMostrarEventos(void *data, int argc, char **argv, char **colName);
int callbackCheckFecha(void *data, int argc, char **argv, char **colName);
int callbackGetID(void *data, int argc, char **argv, char **colName);
void apuntarseEvento(sqlite3 *db, int id_usuario);

void evaluarBeneficiario(Beneficiario b);
void mostrarAyudaComida(Beneficiario b);
void mostrarAyudaRopa(Beneficiario b);
void verProximoRepartoComida(sqlite3 *db);
void verProximoRepartoRopa(sqlite3 *db, int id_beneficiario);
float calcularAyudaDinero(Beneficiario b);
void verTalleresProximos(sqlite3 *db);

// Funciones auxiliares para beneficiario
Beneficiario guardarCondicionesBeneficiario(void);
int actualizarDatosBeneficiario(sqlite3 *db, int id_usuario, Beneficiario b);
int buscarIdEspecifico(sqlite3 *db, int id_usuario, int tipo);
void consultarMisEventos(sqlite3 *db, int id_voluntario);
void consultarHistorialEventos(sqlite3 *db, int id_usuario);
void listarDonaciones(sqlite3 *db, int id_usuario);

// Funciones para registro de recogida de ropa
void listarBeneficiarios(sqlite3 *db);
void listarEventosRopaFuturos(sqlite3 *db);
void registrarRecogidaRopa(sqlite3 *db, int id_beneficiario, int id_evento);
void registrarRecogidaRopaInterfaz(sqlite3 *db);
void registrarRecogidaRopaAdmin(sqlite3 *db);

// Administrador
int es_bisiesto(int a);
int comparar_fechas(Fecha f1, Fecha f2);
int leer_y_validar_fecha(const char *mensaje, Fecha *f);

void menuAdministrador(sqlite3 *db);
void borrarEvento(sqlite3 *db);
void darBajaUsuario(sqlite3 *db);
void listarUsuarios(sqlite3 *db);
void crearEventoMartesAutomatico(sqlite3 *db);
void asegurarEventoRopa(sqlite3 *db);





*/















// --- GESTIÓN DE USUARIOS ---
int insertarUsuario(sqlite3 *db, Usuario u, void *datosEspecificos);
int insertarDatosBeneficiario(sqlite3 *db, Beneficiario b);
int insertarDatosVoluntario(sqlite3 *db, Voluntario v);
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