// protocolo.h
#pragma once

enum TipoOperacion {
    OP_LOGIN = 1,
    OP_REGISTRO_VOLUNTARIO,
    OP_REGISTRO_DONANTE,
    OP_REGISTRO_BENEFICIARIO,
    OP_CONSULTAR_EVENTOS,
    OP_DONACION_DINERO,
    OP_CONSULTAR_MIS_EVENTOS,
    OP_DESAPUNTAR_EVENTO,
    OP_VER_EVENTOS_DISPONIBLES,
    OP_INSCRIBIR_EN_EVENTO,
    OP_RESPUESTA_OK,
    OP_RESPUESTA_ERROR,
    
    OP_DONACION_COMIDA,
    OP_DONACION_ROPA,         
    OP_CONSULTAR_DONACIONES,
    OP_ACTUALIZAR_PERFIL,

    OP_CREAR_EVENTO,
    OP_BORRAR_EVENTO,
    OP_LISTAR_EVENTOS,
    OP_LISTAR_USUARIOS,
    OP_BAJA_USUARIO,
    OP_LISTAR_BENEFICIARIOS,
    OP_LISTAR_EVENTOS_ROPA
};

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

struct EstructuraFechaRed {
    int anyo;
    int mes;
    int dia;
    int hora;
};

// Estructura unificada para todos los datos del Panel de Administración
struct DatosAdmin {
    char material[50]; 
    char descripcion[150];      
    int cupo_o_limite;          
    int cantidad_ropa;          
    EstructuraFechaRed f_inicio;
    EstructuraFechaRed f_final;
};

// TU PAQUETE ÚNICO DE RED SIN DUPLICADOS
struct PaqueteRed {
    int tipoOperacion;          
    int idUsuario;              // Carga: id_usuario (Baja), id_beneficiario (Ropa) o id_profesor (Taller)
    int idEvento;               // Carga: id_evento (Borrar)
    int tipoUsuario;            
    
    float cantidadDonada;
    int tipoDonacion;
    
    DatosPersonales perfil;     
    DatosEconomicos economia;   
    DatosAdmin admin;           // Bloque limpio para tus 6 operaciones admin

    char mensajeRespuesta[4096]; 
};