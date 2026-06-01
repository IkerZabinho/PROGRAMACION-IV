#ifndef EJER1_H
#define EJER1_H

//como nos dice declarar dentro de namespace container



class ArrayInt
{
private:
    unsigned int capacity;
    int *array;

public:
    ArrayInt();
    ArrayInt(unsigned int capacity);
    ~ArrayInt();

    void setValue(unsigned int index, int data);
    
    int getValue(unsigned int index);
    void setCapacity(unsigned int capacity);
    unsigned int getCapacity();
};

#endif;