#ifndef ELEMENT(EJER2)_H
#define ELEMENT(EJER2)_H

class Element
{
private:
    int data;
    Element *next;

public:
    Element(int data);
    int getData();
    void setNext(Element *e);
    Element *getNext() const;
};

#endif