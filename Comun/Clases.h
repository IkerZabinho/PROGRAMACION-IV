#ifndef CLASES_H
#define CLASES_H

#include <string>
#include <vector>

namespace GestionONG {

    // --- ENUMERACIONES ---
    enum TipoUsuario { VOLUNTARIO = 1, DONANTE, BENEFICIARIO, ADMINISTRADOR };
    enum TipoDonacion { COMIDAD = 1, ROPAD, DINERO };
    enum TipoEvento { RECOGIDA = 1, REPARTO };
    enum Material { ROPA_MAT = 1, COMIDA_MAT }; 
    enum TipoComida { CARBOHIDRATOS = 1, LEGUMBRES, CONSERVAS, LACTEOS };
    enum TipoTaller { COCINA = 1, APRENDIZAJE, DEPORTES };

    // --- CLASE FECHA ---
    class Fecha {
    public:
        int hora, minutos, dia, mes, anyo;
        
        // Solo declaración del constructor
        Fecha(int h=0, int min=0, int d=1, int m=1, int a=2024);
    };

    // --- JERARQUÍA DE USUARIOS ---
    class Usuario {
    protected:
        int id_usuario;
        std::string nombre;
        std::string apellidos;
        std::string nombre_usuario;
        std::string contrasena;
        TipoUsuario tipo;

    public:
        Usuario(int id, std::string nom, std::string ape, std::string user, std::string pass, TipoUsuario t);
        virtual ~Usuario(); 
        
        int getId() const;
        TipoUsuario getTipo() const;
    };

    class Beneficiario : public Usuario {
    private:
        int id_beneficiario;
        int num_adultos;
        int num_ninos;
        float ingresos;
        float gastos;
    public:
        Beneficiario(int idU, std::string nom, std::string ape, std::string user, std::string pass, 
                     int idB, int adultos, int ninos, float ing, float gas);
    };

    class Voluntario : public Usuario {
    private:
        int id_voluntario;
        std::string rol;
    public:
        Voluntario(int idU, std::string nom, std::string ape, std::string user, std::string pass, 
                   int idV, std::string r);
    };

    class Donante : public Usuario {
    private:
        int id_donante;
    public:
        Donante(int idU, std::string nom, std::string ape, std::string user, std::string pass, int idD);
    };

    // --- CLASES DE GESTIÓN ---
    class Taller {
    public:
        int id_taller;
        TipoTaller tipo;
        Fecha fecha_ini;
        Fecha fecha_fin;
        std::string descripcion;
        int id_voluntario;

        Taller(int id, TipoTaller t, Fecha ini, Fecha fin, std::string desc, int idV);
    };

    class Evento {
    public:
        int id_evento;
        Fecha fecha_inicio;
        Fecha fecha_fin;
        TipoEvento tipoEvento;
        std::string descripcion;
        int lim_voluntarios;
        Material material;

        Evento(int id, Fecha ini, Fecha fin, TipoEvento t, std::string desc, int lim, Material mat);
    };

    class Donacion {
    public:
        int id_donacion;
        int id_usuario;
        TipoDonacion tipoDonacion;
        std::string fecha; 

        Donacion(int id, int idU, TipoDonacion t, std::string f);
    };

    // --- CLASES DE DETALLE ---
    class Comida {
    public:
        int id_comida;
        TipoComida tipo_comida;
        float kilos;
        int id_donacion;

        Comida(int id, TipoComida t, float k, int idD);
    };

    class Dinero {
    public:
        int id_dinero;
        int id_donacion;
        float cantidad;

        Dinero(int id, int idD, float cant);
    };

    class Ropa {
    public:
        int id_ropa;
        int id_donacion;
        float kilos;

        Ropa(int id, int idD, float k);
    };

    // --- CONFIGURACIÓN ---
    class Config {
    public:
        std::string admin_user;
        std::string admin_pass;
        std::string db_path;
        std::string report_name;
    };

} // namespace GestionONG

// ponemos atributos private cuando son datos que guardar sobre usuarios

// faltan las funciones que luego pondremos de funciones.c
#endif