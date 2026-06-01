#include "Ejer1.h"
#include <iostream>
using namespace std;

namespace containers {


    ArrayInt::ArrayInt(){
        capacity=0;
        array = nullptr;
    }


    ArrayInt:: ArrayInt(unsigned int capacity){
        this->capacity = capacity;
        array = new int[capacity];


    }


    ArrayInt::~ArrayInt(){
        delete[]array;
    }

    void ArrayInt::setValue(unsigned int index, int data){
        if (index< capacity)
        {

            array[index]= data;
        }else{
            //cout<<
        }
        
    }
unsigned int ArrayInt::getCapacity()
{
return 0;
}
}