// interfazDon.h
#ifndef INTERFAZDON_H
#define INTERFAZDON_H

void menuDonante(int socketServidor, int id_perfil);
void donarDinero(int socketServidor, int id_donante);
void consultarHistorialDonaciones(int socketServidor, int id_donante);
void donarComida( int socketServidor, int id_perfil);
void donarRopa( int socketServidor, int id_perfil);

#endif // INTERFAZDON_H