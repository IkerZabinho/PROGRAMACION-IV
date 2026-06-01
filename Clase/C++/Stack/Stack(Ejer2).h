#ifndef STACK(EJER2)_H
#define STACK(EJER2)_H
#include "Element(Ejer2).h"

class Stack
{
private:
    Element *first;
    unsigned int size;

public:
    Stack();
    ~Stack();
    void push(int dato);
    int pop();
    int poll();
    void clear();
    unsigned int getSize();
};

#endif