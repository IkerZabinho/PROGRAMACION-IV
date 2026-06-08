#ifndef INTERFAZ_USUARIO_H
#define INTERFAZ_USUARIO_H

#include "../Comun/protocolo.h"

class InterfazUsuario {
public:
    virtual ~InterfazUsuario() {}

    virtual void ejecutarMenu(int socketServidor, int id_perfil) = 0;
};

#endif 