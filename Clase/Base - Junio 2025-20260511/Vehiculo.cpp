#include "Vehiculo.h"
#include <cstring>

namespace deusto {

// Inicialización de la variable estática
int Vehiculo::numVehiculos = 0;

Vehiculo::Vehiculo(int anyo, const char* modelo) {
    this->anyo = anyo;
    this->velocidad = 0.0; // "La velocidad se ha de iniciar a cero"
    
    if (modelo != nullptr) {
        this->modelo = new char[strlen(modelo) + 1];
        strcpy(this->modelo, modelo);
    } else {
        this->modelo = new char[1];
        this->modelo[0] = '\0';
    }
    numVehiculos++; // Incrementa el contador (No para el copy constructor según el enunciado)
}

// Constructor de copia
Vehiculo::Vehiculo(const Vehiculo& v) {
    this->anyo = v.anyo;
    this->velocidad = v.velocidad;
    this->modelo = new char[strlen(v.modelo) + 1];
    strcpy(this->modelo, v.modelo);
    // El enunciado dice explicitly: "que se crean (no para el copy constructor)" -> NO incrementamos aquí
}

// Operador de asignación
Vehiculo& Vehiculo::operator=(const Vehiculo& v) {
    if (this != &v) {
        delete[] this->modelo;
        this->anyo = v.anyo;
        this->velocidad = v.velocidad;
        this->modelo = new char[strlen(v.modelo) + 1];
        strcpy(this->modelo, v.modelo);
    }
    return *this;
}

// Destructor
Vehiculo::~Vehiculo() {
    delete[] modelo;
}

// Getters y Setters
const char* Vehiculo::getModelo() const { return modelo; }
void Vehiculo::setModelo(const char* modelo) {
    if (modelo != nullptr) {
        delete[] this->modelo;
        this->modelo = new char[strlen(modelo) + 1];
        strcpy(this->modelo, modelo);
    }
}
int Vehiculo::getAnyo() const { return anyo; }
void Vehiculo::setAnyo(int anyo) { this->anyo = anyo; }
double Vehiculo::getVelocidad() const { return velocidad; }
void Vehiculo::setVelocidad(double velocidad) { this->velocidad = velocidad; }

int Vehiculo::getNumVehiculos() { return numVehiculos; }

// Mostrar Info genérica (excepto velocidad)
void Vehiculo::mostrarInfo() const {
    std::cout << "(mostrar info de vehiculo:)" << std::endl;
    std::cout << modelo << ", " << anyo << std::endl;
}

// Implementación base de acelerar y frenar en 5 unidades
void Vehiculo::acelerar(int tiempo) {
    velocidad += 5;
    std::cout << "(acelerar desde vehiculo)" << std::endl;
    std::cout << "(velocidad cambiada a: " << velocidad << ")" << std::endl;
}

void Vehiculo::frenar(int tiempo) {
    velocidad -= 5;
    if (velocidad < 0) velocidad = 0; // Control de seguridad
    std::cout << "(frenar desde vehiculo)" << std::endl;
    std::cout << "(velocidad cambiada a: " << velocidad << ")" << std::endl;
}

} // namespace deusto