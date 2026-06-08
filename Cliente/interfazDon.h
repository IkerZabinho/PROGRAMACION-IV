// interfazDon.h
#ifndef INTERFAZDON_H
#define INTERFAZDON_H

#include "InterfazUsuario.h"


class InterfazDonante : public InterfazUsuario {
public:
    InterfazDonante() = default;

    void ejecutarMenu(int socketServidor, int id_perfil) override;
};

void donarDinero(int socketServidor, int id_donante);
void consultarHistorialDonaciones(int socketServidor, int id_donante);
void donarComida(int socketServidor, int id_perfil);
void donarRopa(int socketServidor, int id_perfil);

#endif 