//int a = 5;
//int b = 7;

//int &r = a;
//r = b;

//esto hace que se copie el valor de b en a.

/*
    referencias_simple.cpp
*/

#include <iostream>
#include <cstring>

using namespace std;

/* REFERENCIAS BASICAS */

void ejemploReferencia()
{
    int a = 5;
    int &r = a;

    r = 10;

    cout << a << endl; // 10
}

/* PUNTEROS VS REFERENCIAS */

void sumar(int a, int b)
{
    a = a + b;
}

void sumarPuntero(int *a, int *b)
{
    *a = *a + *b;
}

void sumarReferencia(int &a, int &b)
{
    a = a + b;
}

void ejemploSumar()
{
    int a = 5;
    int b = 7;

    sumarPuntero(&a, &b);
    cout << a << endl; // 12

    a = 5;
    b = 7;

    sumarReferencia(a, b);
    cout << a << endl; // 12

    a = 5;
    b = 7;

    sumar(a, b);
    cout << a << endl; // 5
}

/* PROBLEMA: COPIA DE OBJETOS */

class Cadena
{
private:
    char* texto;

public:
    Cadena(const char* t)
    {
        texto = new char[strlen(t) + 1];//cuando se hace-> new poner new tipo(int,char..)[strlen(el dato)+1]
        strcpy(texto, t);//Copia el contenido del texto recibido al espacio de memoria que acabamos de reservar.
    }
//DESTRUCTOR
    ~Cadena()
    {
        delete [] texto;//Libera la memoria reservada
    }


    bool igualA(Cadena &c)//& hace una copia con ello para no cambiar el original
    {
        return strcmp(texto, c.texto) == 0;//si texto y c.texto=0 iguales
    }

    void mostrar()
    {
        cout << texto << endl;
    }
};

//si hacemos bool igualA(Cadena c), pasamos una copia de c que es un puntero,
//por lo que pasamos la direccion del puntero. Al acabar la funcion, c se borra,
//para eso, se llama al destructor y se libera la memoria a la que apunta, esto es, c2. 

void ejemploCadena()
{
    Cadena c1("Una cadena");
    Cadena c2("Otra cadena");
    c1.mostrar();
    c2.mostrar();
    cout << (c1.igualA(c2)?"Igual":"Distinta") << endl;
    c1.mostrar();
    c2.mostrar();
}

/* DEVOLVER REFERENCIAS */

int& referenciaCorrecta(int &x)
{
    return x;
}

/*
    MAL:

    int& referenciaIncorrecta()
    {
        int x = 5;
        return x;
    }

    x desaparece al terminar la funcion.
*/

void ejemploRetorno()
{
    int a = 5;

    referenciaCorrecta(a) = 20;

    cout << a << endl; // 20
}

int main()
{
    ejemploReferencia();
    ejemploSumar();
    ejemploCadena();
    ejemploRetorno();

    return 0;
}