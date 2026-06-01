#include "Camion.h"
#include <cmath>

using namespace std;

namespace deusto {

Camion::Camion(int anyo, const char* modelo, double capacidad) 
    : Vehiculo(anyo, modelo) {
    this->capacidad = capacidad;
    this->cargado = false; // "(cargado se definirá en false al comienzo por defecto)"
}

void Camion::setMotor(const Motor& m) {
    this->motor = m;
}


//E
  //S
   //C
    //A
      //L
       //E
        //R
         //A
void Camion:: mostrarInfo()const{
    cout <<"mostrar info de camion"<< endl;
    cout << getModelo()<<", " <<getAnyo()<< endl;
    cout << "Capacidad: "<< capacidad << "tonas "<<endl;
    cout << "Potencia: "<< motor.getPotencia()<<endl;
    cout<<"Tipo motor: "<< motor.getTipo()<<endl; 
}


// Fórmula exigida para camión: v = v + 0.5 * sqrt(tiempo * potencia)
void Camion::acelerar(int tiempo) {
    cout << "(acelerar desde camion)" <<endl;
    double nuevaVel = getVelocidad() + 0.5 * sqrt(tiempo * motor.getPotencia());
    setVelocidad(nuevaVel);
    cout << "(velocidad cambiada a: " << getVelocidad() << ")" << endl;
}

void Camion::frenar(int tiempo) {
    cout << "(frenar desde camion)" << endl;
    double nuevaVel = getVelocidad() - 0.5 * sqrt(tiempo * motor.getPotencia());
    if (nuevaVel < 0) nuevaVel = 0;
    setVelocidad(nuevaVel);
    cout << "(velocidad cambiada a: " << getVelocidad() << ")" << endl;
}

void Camion::cargarDescargar() {
    if (!cargado) {
        cargado = true;
        cout << "(Cargando camion...)" << endl;
    } else {
        cargado = false;
        cout << "(Descargando camion...)" << endl;
    }
}

} // namespace deusto