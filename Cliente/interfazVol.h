#ifndef INTERFAZVOL_H
#define INTERFAZVOL_H

void menuVoluntario(int socketServidor, int id_perfil);
void apuntarseEvento(int socketServidor, int id_perfil);
void consultarMisEventos(int socketServidor, int id_perfil);
void consultarHistorialEventos(int socketServidor, int id_perfil);

#endif