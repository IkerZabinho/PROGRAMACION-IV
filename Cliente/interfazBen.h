//interfazBen.h

#ifndef INTERFAZBEN_H
#define INTERFAZBEN_H
#include "interfaz.h"
#include "../Comun/protocolo.h" // Asegura que conoce la estructura PaqueteRed

using namespace GestionONG;

void menuBeneficiario(int socketServidor, int id_perfil, const PaqueteRed& datosSesion);
Beneficiario guardarCondicionesBeneficiario(const GestionONG::Beneficiario& bActual);
int actualizarDatosBeneficiario(int socketServidor, int id_perfil, const GestionONG::Beneficiario& b);
void verProximoRepartoComida(int socketServidor);
void verProximoRepartoRopa(int socketServidor, int id_perfil);
void verTalleresProximos(int socketServidor);
void limpiarBufferLocal();
float calcularAyudaDinero(GestionONG::Beneficiario b);
void mostrarAyudaComida(GestionONG::Beneficiario b);
void mostrarAyudaRopa(GestionONG::Beneficiario b);
void evaluarBeneficiario(const GestionONG::Beneficiario& b);

#endif // INTERFAZBEN_H