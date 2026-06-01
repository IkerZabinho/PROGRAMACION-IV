#ifndef COCHE_H
#define COCHE_H

#include "Vehiculo.h"
#include "Motor.h"

namespace deusto {

class Coche : public Vehiculo {
private:
    int numPuertas;
    char* tipoCarroceria;
    Motor motor; // Atributo tipo Motor (Composición)

public:
    Coche(int anyo, const char* modelo, int numPuertas, const char* tipoCarroceria);
    Coche(const Coche& c);
    Coche& operator=(const Coche& c);
    ~Coche();

    // Set para el motor externo
    void setMotor(const Motor& m);
    Motor getMotor() const;

    // Redefinición de métodos virtuales
    void mostrarInfo() const override;
    void acelerar(int tiempo) override;
    void frenar(int tiempo) override;

    // Apartado 4.3: Sobrecarga de operadores += y -=
    Coche& operator+=(double incremento);
    Coche& operator-=(double decremento);
};

} // namespace deusto

#endif