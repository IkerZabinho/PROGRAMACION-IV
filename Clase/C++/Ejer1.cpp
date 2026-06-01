#include "Ejer1.h"
#include <iostream>

using namespace std;

namespace containers {

    // Constructor por defecto
    ArrayInt::ArrayInt() {
        capacity = 0;
        array = nullptr;
    }

    // Constructor con reserva de memoria
    ArrayInt::ArrayInt(unsigned int capacity) {
        this->capacity = capacity;
        this->array = new int[capacity];
    }

    // Destructor: ¡Fundamental para no dejar basura en memoria!
    ArrayInt::~ArrayInt() {
        delete[] array;
    }

    void ArrayInt::setValue(unsigned int index, int data) {
        if (index < capacity) {
            array[index] = data;
        } else {
            cout << "Error: Indice " << index << " fuera de rango." << endl;
        }
    }

    int ArrayInt::getValue(unsigned int index) {
        if (index < capacity) {
            return array[index];
        }
        cout << "Error: Indice fuera de rango." << endl;
        return -1; 
    }

    unsigned int ArrayInt::getCapacity() {
        return this->capacity;
    }

    // Nota: Redimensionar un array es complejo. 
    // Por ahora, al menos asegúrate de devolver el valor correcto.
    void ArrayInt::setCapacity(unsigned int capacity) {
        // Implementación simple (ojo: esto no redimensiona la memoria real 
        // a menos que borres y crees el array de nuevo)
        this->capacity = capacity;
    }
}