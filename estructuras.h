#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

// TIPO USUARIO
typedef enum {
    VOLUNTARIO,
    DONANTE,
    BENEFICIARIO,
    ADMINISTRADOR
} TipoUsuario;

// TIPO DONACION
typedef enum {
    COMIDAD,
    ROPAD,
    DINERO
} TipoDonacion;

// TIPO DE EVENTO
typedef enum{
    RECOGIDA,
    REPARTO
} TipoEvento;

typedef enum {
    ROPA,
    COMIDA
} Material;

//TIPO COMIDA

typedef enum{
    CARBOHIDRATOS=1,
    LEGUMBRES,
    CONSERVAS,
    LACTEOS,
} TipoComida;


//---ESTRUCTURAS---
// FECHA
typedef struct{
    int hora;
    int minutos;
    int dia;
    int mes;
    int anyo;
} Fecha;


// USUARIO
typedef struct {
    int id_usuario;
    char nombre[50];
    char apellidos[100];
    char nombre_usuario[30];
    char contrasena[30];
    TipoUsuario tipoUsuario;
} Usuario;

// BENEFICIARIO
typedef struct{
    int id_beneficiario;  //pk en la DB
    int id_usuario;  //FK
    int num_adultos;  
    int num_ninos;
    float ingresos;
    float gastos;
} Beneficiario;

// VOLUNTARIO 
typedef struct {
    int id_voluntario;
    int id_usuario;
    char rol[50];
} Voluntario;

// EVENTO
typedef struct{
    int id_evento;
    Fecha fecha_inicio;
    Fecha fecha_fin;
    TipoEvento tipoEvento;
    char descripcion[200];
    int lim_voluntarios;
    Material material;
} Evento;

// DONACION
typedef struct{
    int id_donacion;
    int id_usuario;
    TipoDonacion tipoDonacion;
    char fecha[20]; //YYYY-MM-DD
} Donacion;

//COMIDA
typedef struct{
    int id_comida;
    TipoComida tipo_comida;
    float kilos;
    int id_donacion;
}Comida;

// DINERO
typedef struct{
    int id_dinero;
    int id_donacion;
    float cantidad;
} Dinero;

//ROPA
typedef struct{
    int id_ropa;
    int id_donacion;
    float kilos;
}Ropa;

// PARTICIPACIÓN 
typedef struct {
    int id_voluntario;
    int id_evento;
} Participacion;

//IMPARTIR
typedef struct {
    int id_voluntario;
    int id_taller;
} Impartir;

//ASISTENCIA
typedef struct {
    int id_beneficiario;
    int id_taller;
} Asistencia;


#endif