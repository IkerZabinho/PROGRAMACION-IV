#ifndef CAMION_H
#define CAMION_H

#include "Vehiculo.h"
#include "Motor.h"

namespace deusto {

class Camion : public Vehiculo {
private:
    double capacidad; // Toneladas
    bool cargado;
    Motor motor;

public:
    Camion(int anyo, const char* modelo, double capacidad);
    
    void setMotor(const Motor& m);
    
    void mostrarInfo() const override;
    void acelerar(int tiempo) override;
    void frenar(int tiempo) override;
    
    // Método específico
    void cargarDescargar();
};

} // namespace deusto

#endif