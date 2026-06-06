// interfazDon.h
#ifndef INTERFAZBEN_H
#define INTERFAZBEN_H
#include "interfaz.h"
using namespace GestionONG;

Beneficiario guardarCondicionesBeneficiario();
int actualizarDatosBeneficiario(int socketServidor, int id_perfil, const GestionONG::Beneficiario& b);
void verProximoRepartoComida(int socketServidor);
void verProximoRepartoRopa(int socketServidor,int id_perfil);
void verTalleresProximos(int socketServidor);



#endif // INTERFAZBEN_H