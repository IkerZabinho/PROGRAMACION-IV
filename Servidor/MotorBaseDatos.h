// MotorBaseDatos.h
#pragma once
#include "../sqlite3.h"
#include "../Comun/protocolo.h"

bool autenticarUsuarioSQL(sqlite3* db, PaqueteRed& paqueteIn, PaqueteRed& paqueteOut);

bool registrarUsuarioSQL(sqlite3* db, PaqueteRed& paqueteIn, void* datosPerfilEspecifico);