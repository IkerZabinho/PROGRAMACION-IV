#include "Motor.h"

Motor::Motor(){
  this->potencia=0;
  this->tipo='g';
}

float Motor::getPotencia() const{
  return potencia;
}

char Motor::getTipo() const{
  return this->tipo;
}

void Motor::defineMotor(float p, char tipo){
  potencia = p;
  this->tipo = tipo;
}