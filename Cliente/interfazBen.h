//interfazBen.h

#ifndef INTERFAZBEN_H
#define INTERFAZBEN_H

#include "interfazUsuario.h"
#include "../Comun/protocolo.h" // Asegura que conoce la estructura PaqueteRed

using namespace GestionONG;

class InterfazBeneficiario : public InterfazUsuario {
private:
    PaqueteRed datosSesion;

public:
    InterfazBeneficiario(const PaqueteRed& datos) : datosSesion(datos) {}
    void ejecutarMenu(int socketServidor, int id_perfil) override;
};

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