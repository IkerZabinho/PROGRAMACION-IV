#ifndef INTERFAZADMIN_H
#define INTERFAZADMIN_H

// --- MENÚ PRINCIPAL DEL ADMINISTRADOR ---
// Solo necesita el ID del perfil logueado (igual que los demás roles)
void menuAdministrador(int id_perfil);

// --- OPERACIONES DE ADMINISTRACIÓN ---
void crearEventoCliente();
void borrarEventoCliente();
void listarUsuariosCliente();
void darBajaUsuarioCliente();
void registrarRecogidaRopaAdminCliente(); 
void crearTallerCliente();
#endif // INTERFAZADMIN_H