#ifndef VEHICULO_H
#define VEHICULO_H

#include <iostream>

namespace deusto { // Apartado 4.1: Integrar todo en un namespace

class Vehiculo {
private:
    char* modelo;  
    int anyo;  
    double velocidad; // Se usa double o float para albergar decimales

    // Variable estática para contar vehículos (Apartado 4.2)
    static int numVehiculos;

public:
    // El enunciado pide: "Se considera un constructor con los argumentos de anyo y modelo (NO UNO POR DEFECTO)"
    Vehiculo(int anyo, const char* modelo);
    
    // El enunciado dice: "se valorará el correcto uso de... copy constructor y destructor"
    Vehiculo(const Vehiculo& v);
    Vehiculo& operator=(const Vehiculo& v);
    virtual ~Vehiculo(); // Siempre virtual en clases base

    // Getters y Setters
    const char* getModelo() const;
    void setModelo(const char* modelo);
    int getAnyo() const;
    void setAnyo(int anyo);
    double getVelocidad() const;
    void setVelocidad(double velocidad);

    // Métodos virtuales para Polimorfismo
    virtual void mostrarInfo() const;
    virtual void acelerar(int tiempo);
    virtual void frenar(int tiempo);

    // Método estático para recuperar el contador (Apartado 4.2)
    static int getNumVehiculos();
};

} // namespace deusto

#endif