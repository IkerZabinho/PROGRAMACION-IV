#include "Coche.h"
#include <cstring>
#include <cmath>   // Necesario para sqrt()


using namespace std;
namespace deusto {

Coche::Coche(int anyo, const char* modelo, int numPuertas, const char* tipoCarroceria) 
    : Vehiculo(anyo, modelo) {
    this->numPuertas = numPuertas;
    
    if (tipoCarroceria != nullptr) {
        this->tipoCarroceria = new char[strlen(tipoCarroceria) + 1];
        strcpy(this->tipoCarroceria, tipoCarroceria);
    } else {
        this->tipoCarroceria = new char[1];
        this->tipoCarroceria[0] = '\0';
    }
}

Coche::Coche(const Coche& c) : Vehiculo(c) {
    this->numPuertas = c.numPuertas;
    this->motor = c.motor;
    this->tipoCarroceria = new char[strlen(c.tipoCarroceria) + 1];
    strcpy(this->tipoCarroceria, c.tipoCarroceria);
}

Coche& Coche::operator=(const Coche& c) {
    if (this != &c) {
        Vehiculo::operator=(c);
        delete[] this->tipoCarroceria;
        this->numPuertas = c.numPuertas;
        this->motor = c.motor;
        this->tipoCarroceria = new char[strlen(c.tipoCarroceria) + 1];
        strcpy(this->tipoCarroceria, c.tipoCarroceria);
    }
    return *this;
}

Coche::~Coche() {
    delete[] tipoCarroceria;
}

void Coche::setMotor(const Motor& m) {
    this->motor = m;
}

Motor Coche::getMotor() const { return motor; }

void Coche::mostrarInfo() const {
 cout << "(mostrar info de coche:)" << std::endl;
    cout << getModelo() << " " << tipoCarroceria << ", " << getAnyo() << ", " << numPuertas << " puertas" << std::endl;
    cout << "Potencia: " << motor.getPotencia() << std::endl;
    cout << "Tipo motor: " << motor.getTipo() << std::endl;
}

// Fórmula exigida: v = v + 0.8 * sqrt(tiempo * potencia)
void Coche::acelerar(int tiempo) {
    cout << "(acelerar desde coche)" << std::endl;
    double nuevaVel = getVelocidad() + 0.8 * sqrt(tiempo * motor.getPotencia());
    setVelocidad(nuevaVel);
    cout << "(velocidad cambiada a: " << getVelocidad() << ")" << std::endl;
}

// Fórmula exigida: v = v - 0.8 * sqrt(tiempo * potencia)
void Coche::frenar(int tiempo) {
    cout << "(frenar desde coche)" << std::endl;
    double nuevaVel = getVelocidad() - 0.8 * sqrt(tiempo * motor.getPotencia());
    if (nuevaVel < 0) nuevaVel = 0;
    setVelocidad(nuevaVel);
    cout << "(velocidad cambiada a: " << getVelocidad() << ")" << std::endl;
}

// Apartado 4.3: Sobrecarga += y -=
Coche& Coche::operator+=(double incremento) {
    setVelocidad(getVelocidad() + incremento);
    cout << "(velocidad cambiada a: " << getVelocidad() << ")" << std::endl;
    return *this;
}

Coche& Coche::operator-=(double decremento) {
    double nuevaVel = getVelocidad() - decremento;
    if (nuevaVel < 0) nuevaVel = 0;
    setVelocidad(nuevaVel);
    cout << "(velocidad cambiada a: " << getVelocidad() << ")" << std::endl;
    return *this;
}

} // namespace deusto