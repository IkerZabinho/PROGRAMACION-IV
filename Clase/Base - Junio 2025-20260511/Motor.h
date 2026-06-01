#ifndef MOTOR_H
#define MOTOR_H

class Motor {
  private:
    float potencia;  // caballos (CV)
    char tipo;  // e-electrico, g-gasolina, d-diesel
  
  public:
    Motor();
    void defineMotor(float potencia, char tipo);
    float getPotencia() const;
    char getTipo() const;
};

#endif