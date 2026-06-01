#include "Clases.h"
#include <iostream>
#include <cstdio>   
#include <cstring>  
#include <cmath>    
#include <sstream>  

using namespace std;
using namespace GestionONG;

// ============================================================================
// 1. CONSTRUCTORES Y DESTRUCTORES
// ============================================================================

Usuario::Usuario(int id, string nom, string ape, string user, string pass, TipoUsuario t) {
    id_usuario = id;
    nombre = nom;
    apellidos = ape;
    nombre_usuario = user;
    contrasena = pass;
    tipo = t;
}

Usuario::~Usuario() {
    // Vacío de momento, necesario para el polimorfismo
}

int Usuario::getId() const { 
    return id_usuario; 
}

TipoUsuario Usuario::getTipo() const { 
    return tipo; 
}

// ============================================================================
// IMPLEMENTACIÓN DE GETTERS
// ============================================================================
std::string Usuario::getNombre() const {
    return nombre;
}

std::string Usuario::getApellidos() const {
    return apellidos;
}

std::string Usuario::getNombreUsuario() const {
    return nombre_usuario;
}

std::string Usuario::getContrasena() const {
    return contrasena;
}

// ============================================================================
// IMPLEMENTACIÓN DE SETTERS
// ============================================================================
void Usuario::setNombre(const std::string& nom) {
    nombre = nom;
}

void Usuario::setApellidos(const std::string& ape) {
    apellidos = ape;
}

void Usuario::setNombreUsuario(const std::string& user) {
    nombre_usuario = user;
}

void Usuario::setContrasena(const std::string& pass) {
    contrasena = pass;
}


Beneficiario::Beneficiario(int idU, string nom, string ape, string user, string pass, 
                           int idB, int adultos, int ninos, float ing, float gas)
    : Usuario(idU, nom, ape, user, pass, BENEFICIARIO) 
{
    id_beneficiario = idB;
    num_adultos = adultos;
    num_ninos = ninos;
    ingresos = ing;
    gastos = gas;
}

// --- DEFINICIÓN DE GETTERS ---
float Beneficiario::getIngresos() const { 
    return ingresos; 
}
float Beneficiario::getGastos() const { 
    return gastos; 
}
int Beneficiario::getNumAdultos() const { 
    return num_adultos; 
}
int Beneficiario::getNumNinos() const { 
    return num_ninos; 
}

// --- DEFINICIÓN DE SETTERS ---
void Beneficiario::setIngresos(float i) { 
    value_type: ingresos = i; 
}
void Beneficiario::setGastos(float g) { 
    gastos = g; 
}
void Beneficiario::setNumAdultos(int a) { 
    num_adultos = a; 
}
void Beneficiario::setNumNinos(int n) { 
    num_ninos = n; 
}

Voluntario::Voluntario(int idU, string nom, string ape, string user, string pass, int idV, string r)
    : Usuario(idU, nom, ape, user, pass, VOLUNTARIO) 
{
    id_voluntario = idV;
    rol = r;
}

Donante::Donante(int idU, string nom, string ape, string user, string pass, int idD)
    : Usuario(idU, nom, ape, user, pass, DONANTE) 
{
    id_donante = idD;
}


// ============================================================================
// 2. MÉTODOS DE INSERCIÓN ESPECÍFICOS (Uso de ostringstream para seguridad)
// ============================================================================

int Beneficiario::insertarDatosBeneficiario(sqlite3 *db) {
    char *error = 0;
    ostringstream sql;

    // Construcción segura de la query
    sql << "INSERT INTO Beneficiario (id_usuario, ingresos, gastos, num_adultos, num_nino) "
        << "VALUES (" << this->id_usuario << ", " << this->ingresos << ", " << this->gastos << ", " 
        << this->num_adultos << ", " << this->num_ninos << ");";

    if (sqlite3_exec(db, sql.str().c_str(), 0, 0, &error) == SQLITE_OK) {
        return (int)sqlite3_last_insert_rowid(db); 
    } else {
        cout << "[ERROR SQL] No se pudo insertar datos de beneficiario: " << error << endl;
        sqlite3_free(error);
        return -1;
    }
}

int Voluntario::insertarDatosVoluntario(sqlite3 *db) {
    char *error = 0;
    ostringstream sql;

    sql << "INSERT INTO Voluntarios (id_usuario) VALUES (" << this->id_usuario << ");";

    if (sqlite3_exec(db, sql.str().c_str(), 0, 0, &error) == SQLITE_OK) {
        return (int)sqlite3_last_insert_rowid(db); 
    } else {
        cout << "[ERROR SQL] No se pudo insertar datos de voluntario: " << error << endl;
        sqlite3_free(error);
        return -1;
    }
}

int Donante::insertarDatosDonante(sqlite3 *db) {
    char *error = 0;
    ostringstream sql;

    sql << "INSERT INTO Donantes (id_usuario) VALUES (" << this->id_usuario << ");";

    if (sqlite3_exec(db, sql.str().c_str(), 0, 0, &error) == SQLITE_OK) {
        return (int)sqlite3_last_insert_rowid(db); 
    } else {
        cout << "[ERROR SQL] No se pudo insertar datos de donante: " << error << endl;
        sqlite3_free(error);
        return -1;
    }
}


// ============================================================================
// 3. FUNCIÓN MADRE DE INSERCIÓN CONTROLADORA (Método Estático)
// ============================================================================

int Usuario::insertarUsuario(sqlite3 *db, const Usuario& u, void* datosEspecificos) {
    char *error = 0;
    int id_usuario_gen = -1;
    int id_perfil_especifico = -1;

    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    // Evitamos desbordamientos concatenando los std::string nativos de C++
    ostringstream sql;
    sql << "INSERT INTO Usuarios (nombre, apellidos, nombre_usuario, contrasena, tipo) VALUES ('"
        << u.nombre << "', '" << u.apellidos << "', '" << u.nombre_usuario << "', '" 
        << u.contrasena << "', " << (int)u.tipo << ");";

    if (sqlite3_exec(db, sql.str().c_str(), 0, 0, &error) != SQLITE_OK) {
        cout << "[ERROR SQL Usuarios] " << error << endl;
        sqlite3_free(error);
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        return -1;
    }

    id_usuario_gen = (int)sqlite3_last_insert_rowid(db);

    // 2. Insertar en las tablas secundarias según el tipo de usuario
    if (u.tipo == BENEFICIARIO) { 
        Beneficiario *b = (Beneficiario *)datosEspecificos;
        b->id_usuario = id_usuario_gen; 
        id_perfil_especifico = b->insertarDatosBeneficiario(db);
    } 
    else if (u.tipo == VOLUNTARIO) { 
        Voluntario *v = (Voluntario *)datosEspecificos;
        v->id_usuario = id_usuario_gen;
        id_perfil_especifico = v->insertarDatosVoluntario(db);
    } 
    else if (u.tipo == DONANTE) { 
        Donante *d = (Donante *)datosEspecificos;
        d->id_usuario = id_usuario_gen;
        id_perfil_especifico = d->insertarDatosDonante(db);
    }

    // Si la inserción del hijo falló, tiramos todo para atrás
    if (id_perfil_especifico == -1) {
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        return -1;
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    return id_perfil_especifico;
}


// ============================================================================
// 4. MÉTODOS DE GESTIÓN DE USUARIOS (Métodos Estáticos)
// ============================================================================

int Usuario::eliminarUsuarioDB(sqlite3 *db, int id) {
    char *error = 0;
    ostringstream sql;
    
    sql << "DELETE FROM Usuarios WHERE id_usuario = " << id << ";";
    
    if (sqlite3_exec(db, sql.str().c_str(), 0, 0, &error) != SQLITE_OK) {
        cout << "Error SQL al eliminar: " << error << endl;
        sqlite3_free(error);
        return -1; 
    }
    return 0; 
}

void Usuario::listarUsuarios(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id_usuario, nombre, apellidos, nombre_usuario FROM Usuarios;";
   
    cout << "\n--- LISTA ACTUAL DE USUARIOS ---\n";
    printf("%-10s %-20s %-20s %-20s\n", "ID", "NOMBRE", "APELLIDOS", "USUARIO");
    cout << "-----------------------------------------------------------------------\n";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            
            // Tratamiento seguro de cadenas devueltas por SQLite
            const unsigned char *txtNombre = sqlite3_column_text(stmt, 1);
            const unsigned char *txtApellidos = sqlite3_column_text(stmt, 2);
            const unsigned char *txtUser = sqlite3_column_text(stmt, 3);

            printf("%-10d %-20s %-20s %-20s\n", id, 
                   txtNombre ? (const char*)txtNombre : "", 
                   txtApellidos ? (const char*)txtApellidos : "", 
                   txtUser ? (const char*)txtUser : "");
        }
    }
    sqlite3_finalize(stmt);
    cout << "-----------------------------------------------------------------------\n";
}


// ============================================================================
// LÓGICA DE LOGÍSTICA DE USUARIOS
// ============================================================================

int Usuario::comprobarLogin(sqlite3 *db, const string& user, const string& pass, Usuario *u_sesion) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id_usuario, nombre, apellidos, nombre_usuario, tipo FROM Usuarios WHERE nombre_usuario = ? AND contrasena = ?;";
    int encontrado = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, pass.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            u_sesion->id_usuario = sqlite3_column_int(stmt, 0);
            
            // CORREGIDO: Verificación de nulos antes de asignar al std::string para evitar crashes
            const unsigned char* n = sqlite3_column_text(stmt, 1);
            const unsigned char* a = sqlite3_column_text(stmt, 2);
            const unsigned char* u = sqlite3_column_text(stmt, 3);

            u_sesion->nombre = n ? (const char*)n : "";
            u_sesion->apellidos = a ? (const char*)a : "";
            u_sesion->nombre_usuario = u ? (const char*)u : "";
            u_sesion->tipo = (TipoUsuario)sqlite3_column_int(stmt, 4);
            
            encontrado = 1;
        }
    }
    sqlite3_finalize(stmt);
    return encontrado;
}

// ============================================================================
// LÓGICA PROPIA DE BENEFICIARIO
// ============================================================================
void Beneficiario::mostrarAyudaRopa() const {
    int camNinos = this->num_ninos * 3;
    int panNinos = this->num_ninos * 2;
    int sudNinos = this->num_ninos * 1;

    int camAdultos = this->num_adultos * 2;
    int panAdultos = this->num_adultos * 1;

    cout << "\n[VESTIMENTA SEMESTRAL]"; 
    if (this->num_ninos > 0) {
        cout << "\n > NIÑOS/AS: " << camNinos << " camisetas, " << panNinos << " pantalones, " << sudNinos << " sudaderas";
    }
    if (this->num_adultos > 0) {
        cout << "\n > ADULTOS: " << camAdultos << " camisetas, " << panAdultos << " pantalones";
    }
}

void Beneficiario::mostrarAyudaComida() const {
    float totalArrozPasta = (this->num_adultos * 1.0f) + (this->num_ninos * 0.75f);
    float totalLegumbres = (this->num_adultos + this->num_ninos) * 0.5f;
    float totalLeche = (this->num_adultos * 2.0f) + (this->num_ninos * 4.0f);
    int totalConservas = (this->num_adultos * 3) + (this->num_ninos * 2);

    cout << "\n[ALIMENTACIÓN SEMANAL]";  
    cout << "\n > Arroz/Pasta:        " << totalArrozPasta << " kg";
    cout << "\n > Legumbres:          " << totalLegumbres << " kg";
    cout << "\n > Leche:              " << totalLeche << " litros";
    cout << "\n > Conservas:          " << totalConservas << " latas";
}

float Beneficiario::calcularAyudaDinero() const {
    float renta = this->ingresos - this->gastos;
    // std::abs requiere de <cmath>
    return std::abs(int(renta)) + 50.0f; 
}


// ============================================================================
// MÉTODO PRINCIPAL: evaluarBeneficiario
// ============================================================================

void Beneficiario::evaluarBeneficiario() const {
    float renta = this->ingresos - this->gastos;
   
    float gastoComidaMensual = (this->num_adultos * 150.0f) + (this->num_ninos * 140.0f);
    float gastoRopaMensual = (this->num_adultos * 5.50f) + (this->num_ninos * 9.0f);
    float umbralTotal = gastoComidaMensual + gastoRopaMensual;

    cout << "\n===========================================";
    cout << "\n       RESULTADO DEL ANÁLISIS SOCIAL";
    cout << "\n===========================================";

    if (renta > umbralTotal) {
        cout << "\nESTADO: Evaluación Finalizada -> Autosuficiente";
        cout << "\nTras analizar tu renta disponible, el sistema indica que puedes cubrir";
        cout << "\nlas necesidades básicas de alimentación y vestimenta por tu cuenta.";
        cout << "\nPriorizamos nuestros recursos para casos en situación de mayor urgencia.";
        cout << "\n-------------------------------------------";
        cout << "\nSi tu situación económica cambia, puedes solicitar una nueva evaluación.";
    }
    else if (renta >= gastoComidaMensual && renta <= umbralTotal) {
        cout << "\nESTADO: Evaluación Finalizada -> Escenario A";
        cout << "\nTras analizar tu renta disponible, consideramos que cubres la alimentación";
        cout << "\nbásica, por lo tanto, recibirás apoyo específico en vestimenta.";
        cout << "\n-------------------------------------------";
        this->mostrarAyudaRopa(); 
    }
    else if (renta > 0 && renta < gastoComidaMensual) {
        cout << "\nESTADO: Evaluación Finalizada -> Escenario B";
        cout << "\nTras analizar tu renta disponible, el sistema indica que necesitas apoyo";
        cout << "\ntanto en alimentación semanal como en vestimenta semestral.";
        cout << "\n-------------------------------------------";
        this->mostrarAyudaComida();
        this->mostrarAyudaRopa();
    }
    else {
        cout << "\nESTADO: Evaluación Finalizada -> Escenario C";
        cout << "\nTras analizar tu renta disponible, el sistema detecta una situación de";
        cout << "\nemergencia. Recibirás ayuda económica, alimentación y vestimenta.";
        cout << "\n-------------------------------------------";
        
        float dinero = this->calcularAyudaDinero();
        printf("\n > AYUDA ECONÓMICA: %.2f euros/mes", dinero);
        
        this->mostrarAyudaComida();
        this->mostrarAyudaRopa();
    }
    cout << "\n===========================================\n" << endl;
}

int Beneficiario::actualizarDatosBeneficiario(sqlite3 *db, int id_perfil) {
    char *error = 0;
    ostringstream sql;

    sql << "UPDATE Beneficiario SET ingresos = " << this->ingresos << ", gastos = " << this->gastos 
        << ", num_adultos = " << this->num_adultos << ", num_nino = " << this->num_ninos 
        << " WHERE id_beneficiario = " << id_perfil << ";";

    if (sqlite3_exec(db, sql.str().c_str(), 0, 0, &error) != SQLITE_OK) {
        cout << "Error al actualizar base de datos: " << error << endl;
        sqlite3_free(error);
        return 0;
    }
    
    this->evaluarBeneficiario();
    return 1;
}