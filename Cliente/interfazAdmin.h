// interfazAdmin.h
#ifndef INTERFAZADMIN_H
#define INTERFAZADMIN_H

#include "InterfazUsuario.h"

class InterfazAdmin : public InterfazUsuario {
public:
    InterfazAdmin() = default;

    void ejecutarMenu(int socketServidor, int id_perfil) override;
};

void crearEventoCliente();
void borrarEventoCliente();
void listarUsuariosCliente();
void darBajaUsuarioCliente();
void registrarRecogidaRopaAdminCliente(); 
void crearTallerCliente();

#endif 