// interfazVol.h
#ifndef INTERFAZVOL_H
#define INTERFAZVOL_H

#include "InterfazUsuario.h" 

class InterfazVoluntario : public InterfazUsuario {
public:
    InterfazVoluntario() = default;

    void ejecutarMenu(int socketServidor, int id_perfil) override;
};

void apuntarseEvento(int socketServidor, int id_perfil);
void consultarMisEventos(int socketServidor, int id_perfil);
void consultarHistorialEventos(int socketServidor, int id_perfil);

#endif // INTERFAZVOL_H