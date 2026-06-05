#ifndef CLASES_H
#define CLASES_H

#include <string>
#include <vector>
#include "sqlite3.h"

namespace GestionONG {

    // --- DECLARACIONES ADELANTADAS ---
    class Ropa;
    class Dinero;
    class Comida;

    // --- ENUMERACIONES ---
    enum TipoUsuario { VOLUNTARIO = 1, DONANTE, BENEFICIARIO, ADMINISTRADOR };
    enum TipoDonacion { COMIDAD = 1, ROPAD, DINERO};
    enum TipoEvento { RECOGIDA = 1, REPARTO };
    enum Material { ROPA_MAT = 1, COMIDA_MAT }; 
    enum TipoComida { CARBOHIDRATOS = 1, LEGUMBRES, CONSERVAS, LACTEOS };
    enum TipoTaller { COCINA = 1, APRENDIZAJE, DEPORTES };

    // --- CLASE FECHA ---
    class Fecha {
    public:
        int hora, minutos, dia, mes, anyo;
        
        Fecha() = default;
        Fecha(int h, int min=0, int d=1, int m=1, int a=2024);

        int es_bisiesto(int anyo);
        int comparar_fechas(const Fecha& f1, const Fecha& f2);
        int leer_y_validar_fecha(const std::string& mensaje, Fecha *f);
    };

    // --- JERARQUÍA DE USUARIOS ---
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
        Usuario() = default;
        Usuario(int id, std::string nom, std::string ape, std::string user, std::string pass, TipoUsuario t);
        virtual ~Usuario(); 
        
        int getId() const;
        TipoUsuario getTipo() const;

        // --- 🆕 DECLARACIÓN DE GETTERS ---
        std::string getNombre() const;
        std::string getApellidos() const;
        std::string getNombreUsuario() const;
        std::string getContrasena() const;

        // --- 🆕 DECLARACIÓN DE SETTERS ---
        void setNombre(const std::string& nom);
        void setApellidos(const std::string& ape);
        void setNombreUsuario(const std::string& user);
        void setContrasena(const std::string& pass);

        // Métodos estáticos de BD (se quedan exactamente igual)
        static int eliminarUsuarioDB(sqlite3 *db, int id);
        static void listarUsuarios(sqlite3 *db);
        static int insertarUsuario(sqlite3 *db, const Usuario& u, void* datosEspecificos);
        static int comprobarLogin(sqlite3 *db, const std::string& user, const std::string& pass, Usuario *u_sesion);
    };

    class Beneficiario : public Usuario {
    private:
        int id_beneficiario;
        int num_adultos;
        int num_ninos;
        float ingresos;
        float gastos;
        
        void mostrarAyudaRopa() const;
        void mostrarAyudaComida() const;
        float calcularAyudaDinero() const;

    public:
        Beneficiario() = default;
        Beneficiario(int idU, std::string nom, std::string ape, std::string user, std::string pass, 
                     int idB, int adultos, int ninos, float ing, float gas);

        int insertarDatosBeneficiario(sqlite3 *db);
        void evaluarBeneficiario() const;
        int actualizarDatosBeneficiario(sqlite3 *db, int id_perfil);

        // --- DECLARACIÓN DE GETTERS (Terminan en ;) ---
        float getIngresos() const;
        float getGastos() const;
        int getNumAdultos() const;
        int getNumNinos() const;

        // --- DECLARACIÓN DE SETTERS (Terminan en ;) ---
        void setIngresos(float i);
        void setGastos(float g);
        void setNumAdultos(int a);
        void setNumNinos(int n);

        friend class Usuario;
    };

    class Voluntario : public Usuario {
    private:
        int id_voluntario;
        std::string rol;
    public:
        Voluntario() = default;
        Voluntario(int idU, std::string nom, std::string ape, std::string user, std::string pass, 
                   int idV, std::string r);

        int insertarDatosVoluntario(sqlite3 *db);
        friend class Usuario;
    };

    class Donante : public Usuario {
    private:
        int id_donante;
    public:
        Donante() = default;
        Donante(int idU, std::string nom, std::string ape, std::string user, std::string pass, int idD);

        int insertarDatosDonante(sqlite3 *db);
        friend class Usuario;
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

        Taller() = default;
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

        Evento() = default;
        Evento(int id, Fecha ini, Fecha fin, TipoEvento t, std::string desc, int lim, Material mat);

        int insertarEvento(sqlite3 *db);
        static int tieneChoqueDeFechas(sqlite3 *db, int id_voluntario, int id_evento_nuevo);
        static int estaEventoLleno(sqlite3 *db, int id_e);
    };

    class Donacion {
    public:
        int id_donacion;
        int id_usuario;
        TipoDonacion tipoDonacion;
        std::string fecha; 

        Donacion() = default;
        Donacion(int id, int idU, TipoDonacion t, std::string f);

        static int insertarDonacionRopa(sqlite3 *db, const Ropa& r, int id_donante);
        static int insertarDonacionDinero(sqlite3 *db, const Dinero& d, int id_donante);
        static int insertarDonacionComidaDB(sqlite3 *db, const Donacion& d, const Comida& c);
    };

    // --- CLASES DE DETALLE ---
    class Comida {
    public:
        int id_comida;
        TipoComida tipo_comida;
        float kilos;
        int id_donacion;

        Comida() = default;
        Comida(int id, TipoComida t, float k, int idD);
    };

    class Dinero {
    public:
        int id_dinero;
        int id_donacion;
        float cantidad;

        Dinero() = default;
        Dinero(int id, int idD, float cant);
    };

    class Ropa {
    public:
        int id_ropa;
        int id_donacion;
        float kilos;

        Ropa() = default;
        Ropa(int id, int idD, float k);
    };

    // ============================================================================
    // 🆕 NUEVO: CLASES DE RELACIÓN INTERMEDIA (FALTABAN AQUÍ)
    // ============================================================================
    class Participacion {
    public:
        int id_voluntario;
        int id_evento;

        Participacion() = default;
        Participacion(int idV, int idE) : id_voluntario(idV), id_evento(idE) {}
    };

    class Impartir {
    public:
        int id_voluntario;
        int id_taller;

        Impartir() = default;
        Impartir(int idV, int idT) : id_voluntario(idV), id_taller(idT) {}
    };

    class Asistencia {
    public:
        int id_beneficiario;
        int id_taller;

        Asistencia() = default;
        Asistencia(int idB, int idT) : id_beneficiario(idB), id_taller(idT) {}
    };
    // ============================================================================

    // --- CONFIGURACIÓN ---
    class Config {
    public:
        std::string admin_user;
        std::string admin_pass;
        std::string db_path;
        std::string report_name;

        Config() = default;
        int cargar_configuracion(const char *filename, Config *conf);
        static void generarReporteResumen(sqlite3 *db, const char *nombreArchivo);
        static int callback_escribir_fichero(void *data, int argc, char **argv, char **azColName);
    };

} // namespace GestionONG

#endif // CLASES_H