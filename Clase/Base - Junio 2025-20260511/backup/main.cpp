#include <iostream>
#include "Motor.h"
#include "Vehiculo.h"
#include "Coche.h"
#include "Camion.h"

using namespace std;
using namespace deusto; // Traemos el namespace global del examen

int main() {
    // ---------------------------------------------------
    // 1. TEST VEHICULO
    // ---------------------------------------------------
    cout << "-- test vehiculo --" << endl;
    Vehiculo v1(2024, "Toyota Corolla");
    v1.mostrarInfo();
    
    cout << "vel mediante setter:" << endl;
    v1.setVelocidad(20);
    cout << "(velocidad cambiada a: " << v1.getVelocidad() << ")" << endl;
    
    v1.acelerar(1);
    v1.frenar(1);
    cout << "------" << endl;

    // ---------------------------------------------------
    // 2. TEST COCHE
    // ---------------------------------------------------
    cout << "-- test coche --" << endl;
    Motor m1;
    m1.defineMotor(125, 'g');

    Coche coche1(2024, "Toyota Corolla sedan", 4, "4 puertas");
    coche1.setMotor(m1);
    coche1.mostrarInfo();
    
    coche1.acelerar(10);
    coche1.frenar(10);
    cout << "------" << endl;

    // ---------------------------------------------------
    // 3. TEST CAMION
    // ---------------------------------------------------
    cout << "-- test camion --" << endl;
    Motor m2;
    m2.defineMotor(330, 'g');

    Camion camion1(2019, "Volvo FM", 15);
    camion1.setMotor(m2);
    camion1.mostrarInfo();
    
    camion1.acelerar(12);
    camion1.frenar(12);
    camion1.cargarDescargar();
    cout << "------" << endl;

    // ---------------------------------------------------
    // 4. TEST VARIADOS (OTROS)
    // ---------------------------------------------------
    cout << "-- test static var --" << endl;
    cout << "Numero de vehiculos creados: " << Vehiculo::getNumVehiculos() << endl;
    cout << "------" << endl;

    cout << "-- test operator --" << endl;
    coche1.setVelocidad(0); // Forzamos a 0 para el test del operador
    coche1 += 5;
    coche1 -= 2;
    cout << "------" << endl;

    return 0;
}