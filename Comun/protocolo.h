// Protocolo.h
#pragma once

// Enumerado para identificar la acción que solicita el cliente
enum TipoOperacion {
    OP_LOGIN = 1,
    OP_REGISTRO_VOLUNTARIO,
    OP_REGISTRO_DONANTE,
    OP_REGISTRO_BENEFICIARIO,
    OP_CONSULTAR_EVENTOS,
    OP_DONACION_DINERO,
    OP_RESPUESTA_OK,
    OP_RESPUESTA_ERROR
};

// Estructuras de datos específicas (Reutilizadas de tus structs de la Fase 1)
struct DatosEconomicos {
    int adultos;
    int ninos;
    float sueldo;
    float otras_ayudas;
    float alquiler;
    float suministros;
    float estudios;
    float otros_gastos;
};

struct DatosPersonales {
    char nombre[50];
    char apellidos[100];
    char usuario[50];
    char contrasena[50];
};

// El paquete principal que viaja por el socket (Red)
struct PaqueteRed {
    int tipoOperacion;          // Almacena un valor del enum TipoOperacion
    int idUsuario;              // ID devuelto o solicitado
    int tipoUsuario;            // Rol del usuario (1: Voluntario, 2: Donante, etc.)
    
    DatosPersonales perfil;     // Estructura anidada con datos de login/registro
    DatosEconomicos economia;   // REQUERIMIENTO 2: Datos para la Caché del Beneficiario
    
    char mensajeRespuesta[256]; // Mensaje de éxito/error del servidor (ej: "[OK] Sesión iniciada")
};