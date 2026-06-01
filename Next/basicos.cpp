#include <iostream>
using namespace std;

/* AURKIBIDEA

    1) Clase básica
    2) Puntero this
    3) cout y cin
    4) Encapsulamiento: private / public
    5) get y set
    6) Constructor
    7) Destructor
    8) Implementación fuera de la clase con ::
    9) Miembros estáticos
    10) Namespaces
    11) Gestión de memoria con new / delete
    12) Arrays dinámicos con new[] / delete[]

    El objetivo es que lo leas de arriba abajo.
*/

// ============================================================
// 1) CLASE BÁSICA
// ============================================================

class FechaBasica {
    // Si no ponemos public/private/protected, en una class es private por defecto.
    unsigned int anyo;
    unsigned int mes;
    unsigned int dia;

    bool esBisiestoInterno() {
        return ((this->anyo % 4 == 0) && (this->anyo % 100 != 0)) || (this->anyo % 400 == 0);
    }

public:
    void demo() {
        anyo = 2024;
        mes = 2;
        dia = 29;

        cout << "[1] Clase basica" << endl;
        cout << "Fecha guardada dentro del objeto: " << dia << "/" << mes << "/" << anyo << endl;
        cout << "Es bisiesto? " << (esBisiestoInterno() ? "Si" : "No") << endl;
        cout << endl;
    }
};

// ============================================================
// 2) PUNTERO THIS
// ============================================================

// C++ añade automáticamente un primer parámetro a cada función: un puntero constante al objeto.
// no está disponible para funciones estáticas (spoiler apartado 9)

class FechaThis {
private:
    unsigned int anyo;
    unsigned int mes;
    unsigned int dia;

public:
    void setAnyo(unsigned int anyo) {
        // El parametro se llama igual que el atributo.
        // this->anyo es el atributo del objeto.
        // anyo es el parametro recibido.
        this->anyo = anyo;
    }

    void setMes(unsigned int mes) {
        this->mes = mes;
    }

    void setDia(unsigned int dia) {
        this->dia = dia;
    }

    void mostrar() {
        cout << "[2] Uso de this" << endl;
        cout << "Fecha: " << this->dia << "/" << this->mes << "/" << this->anyo << endl;
        cout << endl;
    }
};

// ============================================================
// 3) COUT Y CIN
// ============================================================

//#INCLUDE <IOSTREAM>,  La escritura en consola en C++ se hace utilizando el operador <<, 
//cout para salida y endl para salto de línea.
// pondríamos std::cout pero gracias al punto 8 solo ponemos cout.

void ejemploEntradaSalida() {
    cout << "[3] cout y cin" << endl;
    cout << "En C++ escribimos con cout y leemos con cin." << endl;

    int numero;
    cout << "Introduce un numero entero: ";
    cin >> numero;

    cout << "Has escrito: " << numero << endl;
    cout << endl;
}

// ============================================================
// 4) ENCAPSULAMIENTO: PRIVATE / PUBLIC
// ============================================================

class FechaEncapsulada {
private:
    unsigned int anyo;
    unsigned int mes;
    unsigned int dia;

public:
    void asignarValores(unsigned int a, unsigned int m, unsigned int d) {
        anyo = a;
        mes = m;
        dia = d;
    }

    bool esBisiesto() {
        return ((anyo % 4 == 0) && (anyo % 100 != 0)) || (anyo % 400 == 0);
    }

    void mostrar() {
        cout << "[4] Encapsulamiento" << endl;
        cout << "Fecha: " << dia << "/" << mes << "/" << anyo << endl;
        cout << "Es bisiesto? " << (esBisiesto() ? "Si" : "No") << endl;
        cout << endl;
    }
};

//voidmain()
//{
//FechaEncapsulada f;
//f.anyo= 2006; // Error de compilación. El atributo no es público
//f.mes= 11; // Error de compilación. El atributo no es público
//f.dia= 12; // Error de compilación. El atributo no es público
//cout<< f.esBisiesto() << endl; // Compilación correcta
//}

// ============================================================
// 5) GETTERS Y SETTERS
// ============================================================

//Es muy recomendable que los atributos nunca sean públicos, Set y get!

class FechaGetSet {
private:
    unsigned int anyo;
    unsigned int mes;
    unsigned int dia;

public:
    void setAnyo(unsigned int anyo) {
        this->anyo = anyo;
    }

    void setMes(unsigned int mes) {
        this->mes = mes;
    }

    void setDia(unsigned int dia) {
        this->dia = dia;
    }

    unsigned int getAnyo() {
        return anyo;
    }

    unsigned int getMes() {
        return mes;
    }

    unsigned int getDia() {
        return dia;
    }
};

//private + getter/setter → acceso indirecto y controlado, podemos poner condiciones para cambiar o.

// ============================================================
// 6) CONSTRUCTOR    
// ============================================================


class FechaConConstructor {
private:
    unsigned int anyo;
    unsigned int mes;
    unsigned int dia;

public:
    FechaConConstructor() {
        anyo = 2006;
        mes = 1;
        dia = 1;
    }

    void mostrar() {
        cout << "[6] Constructor" << endl;
        cout << "Fecha inicial por defecto: " << dia << "/" << mes << "/" << anyo << endl;
        cout << endl;
    }
};

// ============================================================
// 7) DESTRUCTOR
// ============================================================ 

class RecursoDemo {
private:
    int* dato;

public:
    RecursoDemo() {
        cout << "[7] Constructor de RecursoDemo" << endl;
        dato = new int;
        *dato = 123;
        cout << "Memoria reservada y valor guardado: " << *dato << endl;
    }

    ~RecursoDemo() {
        cout << "Destructor de RecursoDemo: liberando memoria" << endl;
        delete dato;
        dato = nullptr;
        cout << endl;
    }
};

// En principio, todos los constructores y destructores serán públicos

// ============================================================
// 8) IMPLEMENTACION FUERA DE LA CLASE CON el OPERADOR DE RESOLUCIÓN DE ÁMBITO ::
// ============================================================

//Declararíamos la clase en un fichero de cabecera .h, e implementaríamos las funciones en 
//un fichero fuente .cpp con ::

//código en un .h :

class FechaSeparada {
private:
    unsigned int anyo;
    unsigned int mes;
    unsigned int dia;

public:
    FechaSeparada();
    void setAnyo(unsigned int anyo);
    void mostrar();
};

//código en un .cpp:

FechaSeparada::FechaSeparada() {
    anyo = 2006;
    mes = 1;
    dia = 1;
}

void FechaSeparada::setAnyo(unsigned int anyo) {
    this->anyo = anyo;
}

void FechaSeparada::mostrar() {
    cout << "[8] Operador ::" << endl;
    cout << "Fecha: " << dia << "/" << mes << "/" << anyo << endl;
    cout << endl;
}

// ============================================================
// 9) MIEMBROS ESTATICOS
// ============================================================

//Indica que ese miembro es único para todas las instancias (objetos) que 
//se creen a partir de esa clase, aquí no hay puntero this, pueden llamarse sin haber creado un 
//objeto, con ::

class FechaEstatica {
public:
    static const unsigned int MESES;

    static bool esBisiesto(unsigned int anyo) {
        return ((anyo % 4 == 0) && (anyo % 100 != 0)) || (anyo % 400 == 0);
    }
};

const unsigned int FechaEstatica::MESES = 12;

//Para saber si un año es bisiesto no necesitas un objeto fecha ya creado.Solo necesitas el número del año.
//en vez de 
//FechaEstatica f;
//f.esBisiesto(2024);
//hacemos
//FechaEstatica::esBisiesto(2024);

//en los objetos estáticos no podemos meter miembros no estáticos, como this, porque no sabría de qué miembro coger.

// ============================================================
// 10) NAMESPACES
// ============================================================

namespace geom {
    class Point {
    private:
        int x;
        int y;

    public:
        Point() {
            x = 0;
            y = 0;
        }

        Point(int x, int y) {
            this->x = x;
            this->y = y;
        }

        void mostrar() {
            cout << "Point(" << x << ", " << y << ")" << endl;
        }
    };
}

namespace otra {
    class Point {
    public:
        void mostrar() {
            cout << "Soy otro Point, de otro namespace" << endl;
        }
    };
}

// ============================================================
// 11) GESTION DE MEMORIA CON NEW / DELETE
// ============================================================

class PointDinamico {
private:
    int x;
    int y;

public:
    PointDinamico(int x, int y) {
        this->x = x;
        this->y = y;
    }

    void mostrar() {
        cout << "PointDinamico(" << x << ", " << y << ")" << endl;
    }
};

// ============================================================
// 12) ARRAYS DINAMICOS CON NEW[] / DELETE[]
// ============================================================

class PointArray {
private:
    int x;
    int y;

public:
    PointArray() {
        this->x = 0;
        this->y = 0;
    }

    void set(int x, int y) {
        this->x = x;
        this->y = y;
    }

    void mostrar() {
        cout << "(" << x << ", " << y << ")";
    }
};

int main() {
    cout << "===== C++ =====" << endl;
    cout << endl;

    // 1) Clase basica
    FechaBasica f1;
    f1.demo();

    // 2) this
    FechaThis f2;
    f2.setAnyo(2025);
    f2.setMes(4);
    f2.setDia(22);
    f2.mostrar();

    // 3) cout y cin
    ejemploEntradaSalida();

    // 4) encapsulamiento
    FechaEncapsulada f3;
    f3.asignarValores(2020, 2, 29);
    f3.mostrar();

    // 5) getters y setters
    FechaGetSet f4;
    f4.setAnyo(2012);
    f4.setMes(6);
    f4.setDia(15);

    cout << "[5] Getters y setters" << endl;
    cout << "Fecha: " << f4.getDia() << "/" << f4.getMes() << "/" << f4.getAnyo() << endl;
    cout << endl;

    // 6) constructor
    FechaConConstructor f5;
    f5.mostrar();

    // 7) destructor
    {
        RecursoDemo r;
        cout << "Estamos dentro del bloque donde existe el objeto r" << endl;
    }
    cout << "Hemos salido del bloque: el destructor ya se ha ejecutado" << endl;
    cout << endl;

    // 8) implementacion separada con ::
    FechaSeparada f6;
    f6.setAnyo(2030);
    f6.mostrar();

    // 9) miembros estaticos
    cout << "[9] Miembros estaticos" << endl;
    cout << "FechaEstatica::MESES = " << FechaEstatica::MESES << endl;
    cout << "2024 es bisiesto? " << (FechaEstatica::esBisiesto(2024) ? "Si" : "No") << endl;
    cout << endl;

    // 10) namespaces
    cout << "[10] Namespaces" << endl;
    geom::Point p1(2, 3);
    otra::Point p2;
    p1.mostrar();
    p2.mostrar();
    cout << endl;

    // 11) new / delete
    cout << "[11] Memoria dinamica con new y delete" << endl;
    PointDinamico* p = new PointDinamico(7, 9);
    p->mostrar();
    delete p;
    p = nullptr;
    cout << "Objeto dinamico liberado con delete" << endl;
    cout << endl;

    // 12) new[] / delete[]
    cout << "[12] Arrays dinamicos con new[] y delete[]" << endl;
    PointArray* vectorPuntos = new PointArray[5];

    for (int i = 0; i < 5; i++) {
        vectorPuntos[i].set(i, i * 10);
    }

    for (int i = 0; i < 5; i++) {
        vectorPuntos[i].mostrar();
        cout << endl;
    }

    

    
    delete[] vectorPuntos;
    vectorPuntos = nullptr;
    cout << "Array dinamico liberado con delete[]" << endl;
    cout << endl;

    cout << "===== FIN DEL RECORRIDO =====" << endl;
    return 0;
}