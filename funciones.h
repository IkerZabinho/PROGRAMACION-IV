#ifndef FUNCIONES_H
#define FUNCIONES_H
#include "estructuras.h"
#include "sqlite3.h"


int insertarDatosBeneficiario(sqlite3 *db, long long id, float ing, float gas, int adu, int nin);
int insertarDatosVoluntario(sqlite3 *db, long long id, const char *rol);
int insertarUsuario(sqlite3 *db, Usuario u);

int callbackLogin(void *data, int argc, char **argv, char **colName);
int comprobarLogin(sqlite3 *db, char *user, char *pass, int *tipo, int *id_res);
int callbackMostrar(void *data, int argc, char **argv, char **colName);
void mostrarUsuarios(sqlite3 *db);

void donarDinero(sqlite3 *db, int id_usuario);
void crearEvento(sqlite3 *db);

void iniciarSesion(sqlite3 *db);
void registrarUsuario(sqlite3 *db);

void menuPrincipal(sqlite3 *db, int tipo, int id_usuario);

int callbackMostrarEventos(void *data, int argc, char **argv, char **colName);
int callbackCheckFecha(void *data, int argc, char **argv, char **colName);
void apuntarseEvento(sqlite3 *db, int id_usuario);



#endif