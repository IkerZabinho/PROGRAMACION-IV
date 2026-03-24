#include <stdio.h>
#include "estructuras.h"

int main(){
    char frase[100];  

    printf("Introduce una frase: ");
    scanf(" %s", frase); // lee hasta ENTER

    int longitud = strlen(frase);
    printf("La longitud de la frase es: %d\n", longitud);


    char frase2[]={"Hola"};
    
    if (strcmp(frase, frase2)==0){
        printf("Las dos palabras son iguales.\n");
    
    }else{
        printf("Las dos palabras no son iguales.\n");
    }
    return 0;
}