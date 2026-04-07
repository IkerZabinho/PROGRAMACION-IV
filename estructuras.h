#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

// TIPO USUARIO
typedef enum {
    VOLUNTARIO,
    DONANTE,
    BENEFICIARIO
} TipoUsuario;

// USUARIO
typedef struct{
    int id_usuario;
    char* nombre;
    char* apellidos;
    char* nombre_usuario;
    char* contrasena;
    TipoUsuario tipoUsuario;
} Usuario;

// TIPO DONACION
typedef enum {
    DONACION_VOLUNTARIO,
    DONACION_DONANTE,
    DONACION_BENEFICIARIO
} TipoDonacion;

// DONACION
typedef struct{
    int id_donacion;
    int id_usuario;
    TipoDonacion tipoDonacion;
} Donacion;

// DINERO
typedef struct{
    int id_dinero;
    int id_donacion;
    double cantidad;
} Dinero;

// BENEFICIARIO
typedef struct{
    int id_beneficiario;
    int id_usuario;
    int num_adultos;
    int num_ninos;
    double ingresos;
    double gastos;
} Beneficiario;

// FECHA
typedef struct{
    int hora;
    int minutos;
    int dia;
    int mes;
    int anyo;
} Fecha;

// TIPO DE EVENTO
typedef enum{
    ROPA,
    COMIDA
} TipoEvento;

// EVENTO
typedef struct{
    Fecha fecha_inicio;
    Fecha fecha_fin;
    int id_evento;
    TipoEvento tipoEvento;
    char* descripcion;
    int lim_voluntarios;
} Evento;

// PARTICIPACIÓN (para guardar qué usuarios van a qué eventos)
//FALTA AÑADIR ESTA TABLA EN SQL!!!!!!!!
typedef struct {
    int id_usuario;
    int id_evento;
} Participacion;


/*CREATE TABLE Participaciones (
    id_usuario INTEGER,
    id_evento INTEGER,
    PRIMARY KEY (id_usuario, id_evento),
    FOREIGN KEY(id_usuario) REFERENCES Usuarios(id_usuario),
    FOREIGN KEY(id_evento) REFERENCES Eventos(id_evento)
);*/

//ME DICE QUE GUARDEMOS FEHCA COMO TEXTO ???????
//si no para buscar eventos proximos 3 meses es una pesadilla de IFs

#endif