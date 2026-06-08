// Cliente/RedCliente.h
#pragma once
#include "../Comun/protocolo.h"

// Función que abre el socket, envía un paquete, recibe la respuesta y cierra el socket
PaqueteRed enviarPeticionServidor(PaqueteRed paqueteAEnviar);