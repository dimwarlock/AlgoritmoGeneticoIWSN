#include <stdio.h>
#include <libpq-fe.h>
//#include <glib.h>
//#include <glibconfig.h>
//#include <gmodule.h>
#include <uthash.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>


/**********
HARD CODING
***********/

// Definición de Constantes Importantes para la lógica del Código
#define LARGO_CABLE_CONEXION_TRANSMISOR 20
#define LARGO_CONEXION_TRANSMISORES 400
#define NUMERO_SENSORES 10

// Definicion de Constantes Importantes para leer el Archivo TXT
#define MAXIMA_LONGITUD_CADENA 1000
#define NOMBRE_ARCHIVO "PuntosSensores10.txt" 

// Definicion de Numero de Puntos en la Malla Triangular
#define NUMERO_PUNTOS 519433

// Definicion de INF como un valor muy grande para representar el infinito
#define INF 999999999

// Definicion de Constantes Importantes para el funcionamiento del Algoritmo Genetico
#define PROBABILIDAD_MUTACION 0.05
#define TAMAÑO_POBLACION NUMERO_SENSORES*5
#define NUMERO_GENERACIONES NUMERO_SENSORES*5
#define PROBABILIDAD_CRUCE 0.6


/*********
Tabla Hash
**********/

// Struct para una Tabla Hash
typedef struct 
{
    char *Clave;
    double Valor;
    UT_hash_handle hh;
} 
TablaHash;

// Funcion para agregar Elemento a la Tabla Hash
void AgregarElementoTablaHash(TablaHash **TablaH, const char *Clave, float Valor) 
{
    TablaHash *Elemento = (TablaHash *) malloc(sizeof(TablaHash));
    Elemento->Clave = strdup(Clave);
    Elemento->Valor = Valor;
    HASH_ADD_STR(*TablaH, Clave, Elemento);
}

// Funcion para encontrar Elemento en la Tabla Hash
TablaHash *EncontrarElementoTablaHash(TablaHash **TablaH, const char *Clave) 
{
    TablaHash *Elemento;
    HASH_FIND_STR(*TablaH, Clave, Elemento);
    return Elemento;
}


/********************************
Variables Globales
*********************************/

int* NumeroTransmisoresPosiblesPorSensor;

bool HayMejorSolucionEnPoblacionInicial = false;

TablaHash *TablaConexiones;
TablaHash *TablaIndividuos;

char (*Puntos)[MAXIMA_LONGITUD_CADENA];


/******************
Definicion de Grafo
*******************/

// Struct para un Grafos
typedef struct 
{
    int NumeroDeNodos;
    double** MatrizAdyacencia;
} 
Grafo;

// Struct de una Arista para el Algoritmo de Kruskal que calcula el MST
typedef struct
{
    int Origen;
    int Destino;
    double Peso;
}
Arista;

// Struct de un Subconjunto para el Algoritmo de Kruskal que calcula el MST
typedef struct
{
    int Padre;
    int Rango;
}
Subconjunto;


/**********************
Definicion de Individuo
***********************/

// Definir la estructura de un individuo
typedef struct 
{
    char** Genes;
    int IndicesTransmisoresActualesEnIndividuo[NUMERO_SENSORES];
    float Fitness;
}
Individuo;


/******************************************************************************************************************************
Función para calcular la Distancia entre un Punto de la Malla y uno definido como Sensor para saber si es un Posible Transmisor
*******************************************************************************************************************************/

double DistanciaEC(char Sensor[], char Punto[])
{
    // Declarar nuevas variables para no alterar las originales
    char SensorD[strlen(Sensor) + 1];
    char PuntoD[strlen(Punto) + 1];

    strcpy(SensorD, Sensor);
    strcpy(PuntoD, Punto);

    /**************************************
    Obtener los XYZ separados del Sensor 
    ***************************************/

    char *SensorCoordenadas[3];  
    
    int q;

    for (q = 0; q < 3; q++) 
    {
        SensorCoordenadas[q] = (char*)malloc(100 * sizeof(char));
    }

    char *SensorCoordenada = strtok(SensorD, " "); 

    // Ciclo para guardar c/Coordenada en SensorCoordenadas
    int i = 0;
    while (SensorCoordenada != NULL) 
    {
        SensorCoordenadas[i] = SensorCoordenada;  
        i++;

        // Llamada adicional a strtok para continuar dividiendo la cadena
        SensorCoordenada = strtok(NULL, " "); 
    }

    // DEBUG - Imprimir cada SensorCoordenada por separado
    //for (int j = 0; j < i; j++) 
    //{
    //    printf("%s\n", SensorCoordenadas[j]);
    //}


    /***********************************************************
    Obtener los XYZ separados del Punto candidato a Transmisor
    ***********************************************************/

    char *PuntoCoordenadas[3];  

    int w;

    for (w = 0; w < 3; w++) 
    {
        PuntoCoordenadas[w] = (char*)malloc(100 * sizeof(char));
    }

    char *PuntoCoordenada = strtok(PuntoD, " "); 

    // Ciclo para guardar c/Coordenada en PuntoCoordenadas
    int k = 0;
    while (PuntoCoordenada != NULL) 
    {
        PuntoCoordenadas[k] = PuntoCoordenada;  
        k++;

        // Llamada adicional a strtok para continuar dividiendo la cadena
        PuntoCoordenada = strtok(NULL, " "); 
    }

    // DEBUG - Imprimir cada PuntoCoordenada por separado
    //for (int l = 0; l < k; l++) 
    //{
    //    printf("%s\n", PuntoCoordenadas[l]);
    //}


    /***********************************************************
    Castear los XYZ a Float para poder operar la Distancia
    ***********************************************************/
    char* END;

    // XYZ del Sensor
    double X1 = strtod(SensorCoordenadas[0], &END);
    double Y1 = strtod(SensorCoordenadas[1], &END);
    double Z1 = strtod(SensorCoordenadas[2], &END);

    /*
    printf("Sensor Coordenada X: %f\n", X1);
    printf("Sensor Coordenada Y: %f\n", Y1);
    printf("Sensor Coordenada Z: %f\n", Z1);
    */

    // XYZ del candidato a Transmisor
    double X2 = strtod(PuntoCoordenadas[0], &END);
    double Y2 = strtod(PuntoCoordenadas[1], &END);
    double Z2 = strtod(PuntoCoordenadas[2], &END);

    /*
    printf("Transmisor Posible Coordenada X: %f\n", X2);
    printf("Transmisor Posible Coordenada Y: %f\n", Y2);
    printf("Transmisor Posible Coordenada Z: %f\n", Z2);
    */


    /***********************************************************************
    Calcular la Distancia entre el Sensor y el Punto candidato a Transmisor
    ************************************************************************/

    double Distancia = sqrt(pow(X2 - X1, 2) + pow(Y2 - Y1, 2) + pow(Z2 - Z1, 2));

    // DEBUG - Mostrar distancia
    //printf("La Distancia calculada es: %f\n", Distancia);

    return Distancia;
}


/******************************************************************
Función para calcular todos los Transmisores Posibles para c/Sensor
*******************************************************************/

char*** CalcularTransmisoresPosibles(char PosicionesSensores[][1000], char (*Puntos)[1000])
{
    // Asignar memoria al arreglo que guardara la cantidad de Transmisores Posibles para c/Sensor
    NumeroTransmisoresPosiblesPorSensor = (int*) malloc(NUMERO_SENSORES * sizeof(int));

    // Matriz que posee en c/Posición todos los Transmisores Posibles para un Sensor
    char*** TransmisoresPosibles = (char***) malloc(NUMERO_SENSORES * sizeof(char**));


    // Validar si un Punto es un TransmisorPosible o no
    int i;
    int j;


    for (i = 0; i < NUMERO_SENSORES; i++)
    {  
        // Asignar memoria para los posibles transmisores del sensor i
        TransmisoresPosibles[i] = (char**) malloc(NUMERO_PUNTOS * sizeof(char*));

        // Contador para almacenar los transmisores posibles para el sensor i
        int ContadorTransmisores = 0; 

        for (j = 0; j < NUMERO_PUNTOS; j++)
        {
            if (DistanciaEC(PosicionesSensores[i], Puntos[j]) <= LARGO_CABLE_CONEXION_TRANSMISOR && DistanciaEC(PosicionesSensores[i], Puntos[j]) != 0)
            {
                // Almacenar el Puntos[j] en TransmisoresPosibles
                TransmisoresPosibles[i][ContadorTransmisores] = strdup(Puntos[j]);

                // Incrementar el contador de transmisores
                ContadorTransmisores++;
            }
        }
        // Guardar la Cantidad de Transmisores Posibles por Sensor en un Arreglo Global
        NumeroTransmisoresPosiblesPorSensor[i] = ContadorTransmisores;
    }

    // DEBUG - Ver los Transmisores Posibles para c/Sensor
    /*
    int k;
    int p;
    for (int k = 0; k < NUMERO_SENSORES; k++)
    {
        printf("Sensor %d:\n", k+1);
        for (int p = 0; p < NUMERO_PUNTOS; p++)
        {
            if (TransmisoresPosibles[k][p] != NULL)
            {
                printf("Sensor %d Transmisor Posible: %s\n", k+1, TransmisoresPosibles[k][p]);
            }
        }
    }
    */

    return TransmisoresPosibles;
}


/**************************************************************
Funcion para Generar Grafo Fase I con los Transmisores Elegidos
***************************************************************/

Grafo GenerarGrafoFaseI(char** Individuo)
{
    // Matriz de Adyacencia para definir las conexiones entre los Transmisores en el Individuo
    double** MatrizAdyacencia = (double**)malloc(NUMERO_SENSORES * sizeof(double*));

    // Asignar memoria a cada Fila en la Matriz
    for (int i = 0; i < NUMERO_SENSORES; i++) 
    {
        MatrizAdyacencia[i] = (double*)malloc(NUMERO_SENSORES * sizeof(double));
    }

    // Calcular la distancia entre todos los pares de puntos y almacenarla en la Matriz de Adyacencia
    for (int i = 0; i < NUMERO_SENSORES; i++) 
    {
        for (int j = i + 1; j < NUMERO_SENSORES; j++) 
        {
            if (i == j) 
            {
                MatrizAdyacencia[i][j] = 0;
            } 
            else if (DistanciaEC(Individuo[i], Individuo[j]) < LARGO_CONEXION_TRANSMISORES)
            {
                MatrizAdyacencia[i][j] = DistanciaEC(Individuo[i], Individuo[j]);
            }
            else
            {
                MatrizAdyacencia[i][j] = 0;
            }
        }
    }

    Grafo GrafoFaseI;
    GrafoFaseI.NumeroDeNodos = NUMERO_SENSORES;
    GrafoFaseI.MatrizAdyacencia = MatrizAdyacencia;

    return GrafoFaseI;
}


/******************************************************************
Funcion que genera el Grafo Fase II con solo las Conexiones Validas
*******************************************************************/

Grafo GenerarGrafoFaseII(char*** ConexionesValidas, int NumeroConexionesValidas)
{
    // Identificar la cantidad de Transmisores Validos en las Conexiones Validas
    char** TransmisoresValidos = (char**)malloc(NUMERO_SENSORES * sizeof(char*));
    int ContadorTransmisoresValidos = 0;

    // Ciclo que añade c/Transmisor identificado a TransmisoresValidos
    for (int h = 0; h < 2; h++)
    {
        for (int q = 0; q < NumeroConexionesValidas; q++)
        {
            int YaExiste = 0;

            for (int g = 0; g < ContadorTransmisoresValidos; g++)
            {
                if (ConexionesValidas[q][h] == TransmisoresValidos[g])
                {
                    YaExiste = 1;
                    break;
                }
            }

            if (YaExiste == 0)
            {
                TransmisoresValidos[ContadorTransmisoresValidos] = (char*)malloc(100 * sizeof(char));
                TransmisoresValidos[ContadorTransmisoresValidos] = ConexionesValidas[q][h];
                ContadorTransmisoresValidos++;
            }
        }
    }

    // Matriz de Adyacencia para definir las conexiones entre los Transmisores en las Conexiones Validas
    double** MatrizAdyacencia = (double**)malloc(ContadorTransmisoresValidos * sizeof(double*));

    // Asignar memoria a cada Fila en la Matriz
    for (int i = 0; i < ContadorTransmisoresValidos; i++) 
    {
        MatrizAdyacencia[i] = (double*)malloc(ContadorTransmisoresValidos * sizeof(double));
    }

    // Calcular la distancia entre todos los pares de puntos y almacenarla en la Matriz de Adyacencia
    for (int c = 0; c < NumeroConexionesValidas; c++)
    {
        for (int i = 0; i < ContadorTransmisoresValidos; i++) 
        {
            for (int j = i + 1; j < ContadorTransmisoresValidos; j++) 
            {
                if (i == j) 
                {
                    MatrizAdyacencia[i][j] = 0;
                } 
                else if (TransmisoresValidos[i] == ConexionesValidas[c][0] && TransmisoresValidos[j] == ConexionesValidas[c][1] 
                        && DistanciaEC(ConexionesValidas[c][0], ConexionesValidas[c][1]) < LARGO_CONEXION_TRANSMISORES)
                {
                    MatrizAdyacencia[i][j] = DistanciaEC(ConexionesValidas[c][0], ConexionesValidas[c][1]);
                }
            }
        }
    }
 

    Grafo GrafoFaseII;
    GrafoFaseII.NumeroDeNodos = ContadorTransmisoresValidos;
    GrafoFaseII.MatrizAdyacencia = MatrizAdyacencia;

    return GrafoFaseII;
}


/******************************************
Función que obtiene las Aristas de un Grafo
*******************************************/

Arista* ObtenerAristas(Grafo GrafoFaseII) 
{
    Arista* Aristas = (Arista*)malloc(GrafoFaseII.NumeroDeNodos * (GrafoFaseII.NumeroDeNodos - 1) * sizeof(Arista));
    
    int k = 0;

    for (int i = 0; i < GrafoFaseII.NumeroDeNodos; ++i) 
    {
        for (int j = 0; j < GrafoFaseII.NumeroDeNodos; ++j) 
        {
            if (GrafoFaseII.MatrizAdyacencia[i][j] > 0) 
            {
                Aristas[k].Origen = i;
                Aristas[k].Destino = j;
                Aristas[k].Peso = GrafoFaseII.MatrizAdyacencia[i][j];
                k++;
            }
        }
    }

    return Aristas;
}


/**************************************
Función que compara 2 Aristas del Grafo
***************************************/

int Comparar(const void* a, const void* b) 
{
    Arista* A1 = (Arista*)a;
    Arista* B1 = (Arista*)b;

    if (A1->Peso< B1->Peso) 
    {
        return -1;
    } 
    else if (A1->Peso > B1->Peso) 
    {
        return 1;
    } 
    else 
    {
        return 0;
    }
}


/**************************************************************
Función que encuentra el Conjunto al cual pertenece un Elemento
***************************************************************/

int Encontrar(Subconjunto Subconjuntos[], int i) 
{
    //printf("\ni Tiene un valor de: %d\n", i);

    if (Subconjuntos[i].Padre != i) 
    {
        Subconjuntos[i].Padre = Encontrar(Subconjuntos, Subconjuntos[i].Padre);
    }

    return Subconjuntos[i].Padre;
}


/**************************
Función que une 2 Conjuntos
***************************/

void Unir(Subconjunto Subconjuntos[], int x, int y) 
{
    int raizX = Encontrar(Subconjuntos, x);
    int raizY = Encontrar(Subconjuntos, y);

    if (Subconjuntos[raizX].Rango < Subconjuntos[raizY].Rango) 
    {
        Subconjuntos[raizX].Padre = raizY;
    } 
    else if (Subconjuntos[raizX].Rango > Subconjuntos[raizY].Rango) 
    {
        Subconjuntos[raizY].Padre = raizX;
    } 
    else 
    {
        Subconjuntos[raizY].Padre = raizX;
        Subconjuntos[raizX].Rango++;
    }
}


/******************************************************************************
Función que calcula e imprime el Arbol de Cobertura con el Algoritmo de Kruskal
*******************************************************************************/

int KruskalMST(Grafo GrafoFaseII, int NumeroConexionesValidas) 
{
    int V = GrafoFaseII.NumeroDeNodos;
    Arista* Resultado = (Arista*)malloc(NumeroConexionesValidas * sizeof(Arista));
    int e = 0;
    int i = 0;

    Arista* Aristas = ObtenerAristas(GrafoFaseII);

    qsort(Aristas, NumeroConexionesValidas, sizeof(Arista), Comparar);
    
    /*
    printf("\nEl Origen de la Arista 1 es ahora: %d", Aristas[0].Origen);
    printf("\nEl Destino de la Arista 1 es ahora: %d", Aristas[0].Destino);
    printf("\nEl Origen de la Arista 2 es ahora: %d", Aristas[1].Origen);
    printf("\nEl Destino de la Arista 2 es ahora: %d", Aristas[1].Destino);
    */

    Subconjunto* Subconjuntos = (Subconjunto*)malloc(V * sizeof(Subconjunto));

    for (int v = 0; v < V; v++) 
    {
        Subconjuntos[v].Padre = v;
        Subconjuntos[v].Rango = 0;
    }

    while (e < V && i < NumeroConexionesValidas) 
    {
        Arista AristaActual = Aristas[i];

        /*
        printf("\nEl Origen de AristaActual la cual es la Arista %d es: %d", i, AristaActual.Origen);
        printf("\nEl Destino de AristaActual la cual es la Arista %d es: %d", i, AristaActual.Destino);
        */

        int x = Encontrar(Subconjuntos, AristaActual.Origen);
        int y = Encontrar(Subconjuntos, AristaActual.Destino);

        if (x != y) 
        {
            Resultado[e++] = AristaActual;
            Unir(Subconjuntos, x, y);
        }
        i++;
    }

    /*
    printf("\nArista   Peso\n");
    for (i = 0; i < e; ++i) 
    {
        printf("%d - %d    %f\n", Resultado[i].Origen, Resultado[i].Destino, Resultado[i].Peso);
    }
    */

    // Es una Regla absoluta que en un Grafo Conexo el N° de Nodos es el N° de Aristas mas 1
    return e+1;
}


/*****************************************************************************
Función que calcula la Orientación de una Conexión desde la perspectiva Punto1
******************************************************************************/

int* DeterminarOrientacionDesdePunto(char* Punto1, char* Punto2) 
{
    // Arreglo que almacena la Orientación, las posiciones 0 1 2 corresponden a X Y Z respectivamente.
    // En el caso de sus posibles valores, 0 significa Negativo al eje, 1 significa Positivo al eje y 2 significa Paralelo al eje.
    int* Orientacion = (int*) malloc(3 * sizeof(int));

    // Declarar nuevas variables para no alterar las originales
    char Punto1O[strlen(Punto1) + 1];
    char Punto2O[strlen(Punto2) + 1];

    strcpy(Punto1O, Punto1);
    strcpy(Punto2O, Punto2);


    /***********************************
    Obtener los XYZ separados del Punto1
    ************************************/

    char *Punto1Coordenadas[3];  
    
    int q;

    for (q = 0; q < 3; q++) 
    {
        Punto1Coordenadas[q] = (char*)malloc(100 * sizeof(char));
    }

    char *Punto1Coordenada = strtok(Punto1O, " "); 

    // Ciclo para guardar c/Coordenada en Punto1Coordenadas
    int i = 0;
    while (Punto1Coordenada != NULL) 
    {
        Punto1Coordenadas[i] = Punto1Coordenada;  
        i++;

        // Llamada adicional a strtok para continuar dividiendo la cadena
        Punto1Coordenada = strtok(NULL, " "); 
    }


    /***********************************
    Obtener los XYZ separados del Punto2
    ************************************/

    char *Punto2Coordenadas[3];  

    int w;

    for (w = 0; w < 3; w++) 
    {
        Punto2Coordenadas[w] = (char*)malloc(100 * sizeof(char));
    }

    char *Punto2Coordenada = strtok(Punto2O, " "); 

    // Ciclo para guardar c/Coordenada en Punto2Coordenadas
    int k = 0;
    while (Punto2Coordenada != NULL) 
    {
        Punto2Coordenadas[k] = Punto2Coordenada;  
        k++;

        // Llamada adicional a strtok para continuar dividiendo la cadena
        Punto2Coordenada = strtok(NULL, " "); 
    }


    /******************************************************
    Castear los XYZ a Float para poder operar la Distancia
    ******************************************************/
    char* END;

    // XYZ del Punto1
    double X1 = strtod(Punto1Coordenadas[0], &END);
    double Y1 = strtod(Punto1Coordenadas[1], &END);
    double Z1 = strtod(Punto1Coordenadas[2], &END);

    // XYZ del Punto2
    double X2 = strtod(Punto2Coordenadas[0], &END);
    double Y2 = strtod(Punto2Coordenadas[1], &END);
    double Z2 = strtod(Punto2Coordenadas[2], &END);

    // Diferencias de Coordenadas
    double DiferenciaX = X2 - X1;
    double DiferenciaY = Y2 - Y1;
    double DiferenciaZ = Z2 - Z1;


    if (DiferenciaX > 0) 
    {
        Orientacion[0] = 1;
    }
    else if (DiferenciaX < 0) 
    {
        Orientacion[0] = 0;
    }
    else 
    {
        Orientacion[0] = 2;
    }

    if (DiferenciaY > 0) 
    {
        Orientacion[1] = 1;
    }
    else if (DiferenciaY < 0) 
    {
        Orientacion[1] = 0;
    } 
    else
    {
        Orientacion[1] = 2;
    }

    if (DiferenciaZ > 0) 
    {
        Orientacion[2] = 1;
    }
    else if (DiferenciaZ < 0) 
    {
        Orientacion[2] = 0;
    }
    else 
    {
        Orientacion[2] = 2;
    }

    /*
    for(int i = 0; i < 3; i++)
    {
        printf("\n La Orientacion a retornar en indice %d es: %d", i, Orientacion[i]);
    }
    */

    return Orientacion;
}


/***********************************************
Función que redimensiona un Punto en la Conexión
************************************************/

char* RedimensionarPunto (char* Punto, int* OrientacionPerspectivaPunto)
{
    // Declarar nuevas variables para no alterar las originales
    char PuntoO[strlen(Punto) + 1];

    strcpy(PuntoO, Punto);

    // DEBUG - Imprimir punto original
    //printf("\n Asi se ve el Punto Original: %s", PuntoO);

    /**********************************
    Obtener los XYZ separados del Punto
    ***********************************/

    char *PuntoCoordenadas[3];  
    
    int q;

    for (q = 0; q < 3; q++) 
    {
        PuntoCoordenadas[q] = (char*)malloc(100 * sizeof(char));
    }

    char *PuntoCoordenada = strtok(PuntoO, " "); 

    // Ciclo para guardar c/Coordenada en PuntoCoordenadas
    int i = 0;
    while (PuntoCoordenada != NULL) 
    {
        PuntoCoordenadas[i] = PuntoCoordenada;  
        i++;

        // Llamada adicional a strtok para continuar dividiendo la cadena
        PuntoCoordenada = strtok(NULL, " "); 
    }

    // DEBUG - Imprimir cada SensorCoordenada por separado
    //for (int j = 0; j < i; j++) 
    //{
    //    printf("%s\n", SensorCoordenadas[j]);
    //}


    /**********************************************
    Castear los XYZ a doouble para poder operarlos
    **********************************************/
    char* END;

    // XYZ del Punto
    double X = strtod(PuntoCoordenadas[0], &END);
    double Y = strtod(PuntoCoordenadas[1], &END);
    double Z = strtod(PuntoCoordenadas[2], &END);


    if(OrientacionPerspectivaPunto[0] == 0)
    {
        // Restar 0.1 al Punto en X
        X = X - 0.1;
    }
    else if (OrientacionPerspectivaPunto[0] == 1)
    {
        // Sumar 0.1 al Punto en X
        X = X + 0.1;
    }

    if(OrientacionPerspectivaPunto[1] == 0)
    {
        // Restar 0.1 al Punto en Y
        Y = Y - 0.1;
    }
    else if (OrientacionPerspectivaPunto[1] == 1)
    {
        // Sumar 0.1 al Punto en Y
        Y = Y + 0.1;
    }

    if(OrientacionPerspectivaPunto[2] == 0)
    {
        // Restar 0.1 al Punto en Z
        Z = Z - 0.1;
    }
    else if (OrientacionPerspectivaPunto[2] == 1)
    {
        // Sumar 0.1 al Punto en Z
        Z = Z + 0.1;
    }

    char* PuntoRedimensionado = (char*) malloc(100 * sizeof(char));

    sprintf(PuntoRedimensionado, "%f %f %f", X, Y, Z);

    // DEBUG - Imprimir punto redimensionado
    //printf("\n Asi se ve el Punto Redimensionado: %s", PuntoRedimensionado);

    return PuntoRedimensionado;
}


/********************************************************************
Función que evalua la interseccion de cada conexion Transmisor/Sensor
*********************************************************************/

int EvaluarConexionesGrafo(char** Genes, Grafo GrafoPrimigenio, PGconn *conn)
{
    // Arreglo que ha de guardar las Conexiones Validas
    char*** ConexionesValidas = (char***) malloc((NUMERO_SENSORES * NUMERO_SENSORES) * sizeof(char**));
    int ContadorConexionesValidas = 0;

    // Matriz que almacena las Conexiones Existentes en el Grafo
    char*** ConexionesExistentes = (char***) malloc((NUMERO_SENSORES * NUMERO_SENSORES) * sizeof(char**));

    // Variable que tiene el Numero de Conexiones
    int NumeroConexiones = 0;

    for (int i = 0; i < NUMERO_SENSORES; i++)
    {
        for (int j = 0; j < NUMERO_SENSORES; j++)
        {
            if (GrafoPrimigenio.MatrizAdyacencia[i][j] > 0.000001)
            {
                ConexionesExistentes[NumeroConexiones] = (char**)malloc(2 * sizeof(char*));
                ConexionesExistentes[NumeroConexiones][0] = (char*)malloc(100 * sizeof(char*));
                ConexionesExistentes[NumeroConexiones][1] = (char*)malloc(100 * sizeof(char*));
                ConexionesExistentes[NumeroConexiones][0] = Genes[i];
                ConexionesExistentes[NumeroConexiones][1] = Genes[j];
                NumeroConexiones++;
            }
        }
    }

    float InterseccionesReales;

    // Convertir las Conexiones a LINESTRING y evaluar su interseccion
    for (int k = 0; k < NumeroConexiones; k++)
    {
        // Buscar la Conexion en la Tabla Hash
        char Clave[200];
        Clave[0] = '\0';
        strcat(Clave, ConexionesExistentes[k][0]);
        strcat(Clave, ConexionesExistentes[k][1]);

        if (EncontrarElementoTablaHash(&TablaConexiones, Clave))
        {
            InterseccionesReales = EncontrarElementoTablaHash(&TablaConexiones, Clave)->Valor;

            // DEBUG - Realmente funciona la Tabla Hash de Conexiones
            printf("\nYa he visto esta Conexion antes, Intersecta estas veces: %f", InterseccionesReales);
        }
        else
        {
            /*******************
            Intersecciones Linea
            ********************/
            
            // Calcular Orientacion desde Punto I a Punto II
            int* OrientacionDesdePuntoI = (int*) malloc(3 * sizeof(int));
            OrientacionDesdePuntoI = DeterminarOrientacionDesdePunto(ConexionesExistentes[k][0], ConexionesExistentes[k][1]);

            // Calcular Orientacion desde Punto II a Punto I
            int* OrientacionDesdePuntoII = (int*) malloc(3 * sizeof(int));

            for (int g = 0; g < 3; g++)
            {
                if(OrientacionDesdePuntoI[g] == 0)
                {
                    OrientacionDesdePuntoII[g] = 1;
                }
                else if(OrientacionDesdePuntoI[g] == 1)
                {
                    OrientacionDesdePuntoII[g] = 0;
                }
            }

            /*
            for(int g = 0; g < 3; g++)
            {
                printf("\nOrientacion numero %d desde Punto 1: %d", g, OrientacionDesdePuntoI[g]);
            }

            for(int g = 0; g < 3; g++)
            {
                printf("\nOrientacion numero %d desde Punto 2: %d", g, OrientacionDesdePuntoII[g]);
            }
            */
            
            // Redimensionar Punto I
            char* PuntoIRedimensionado = (char*) malloc(100 * sizeof(char));
            PuntoIRedimensionado = RedimensionarPunto(ConexionesExistentes[k][0], OrientacionDesdePuntoI);

            // Redimensionar Punto II
            char* PuntoIIRedimensionado = (char*) malloc(100 * sizeof(char));
            PuntoIIRedimensionado = RedimensionarPunto(ConexionesExistentes[k][1], OrientacionDesdePuntoII);


            char LineaActual[200];
            LineaActual[0] = '\0';
            strcat(LineaActual, "'LINESTRING( ");
            strcat(LineaActual, PuntoIRedimensionado);
            strcat(LineaActual, ", ");
            strcat(LineaActual, PuntoIIRedimensionado);
            strcat(LineaActual, " )'");
            

            char QueryIII[300];
            QueryIII[0] = '\0';
            
            strcat(QueryIII, "SELECT EXISTS (SELECT 1 FROM Triangulos WHERE ST_3DIntersects(triangulo, ");
            strcat(QueryIII, LineaActual);
            strcat(QueryIII, "::geometry) = true);");
            

            PGresult *Res3;
            Res3 = PQexec(conn, QueryIII);
            int NumeroInterseccionesLinea = atoi(PQgetvalue(Res3,0 ,0));

            
            char* TrueAha = (char*) malloc(sizeof(char));
            TrueAha[0] = 't';
            if (PQgetvalue(Res3, 0, 0)[0] == TrueAha[0])
            {
                NumeroInterseccionesLinea = 1;
                //printf("Oh no, colisiones!\n");
            }
            else
            {
                //printf("Efectivamente, soy capaz de detectar cuando no hay colisiones.\n");
                NumeroInterseccionesLinea = 0;
            }
            

            PQclear(Res3);

            // Las Intersecciones Reales son las Intersecciones de la Linea menos las Intersecciones de los Puntos
            InterseccionesReales = (float)NumeroInterseccionesLinea;
            
            //printf("Las Intersecciones Reales son: %f\n", InterseccionesReales);
            //printf("Donde las Intersecciones Reales fueron de: %d. Y las Intersecciones Validas de: %d\n\n", NumeroInterseccionesLinea, InterseccionesValidas);

            /*********
            PEOR CASO
            *********/

            //InterseccionesReales = 1.0F;

            // Añadir la cantidad de Interseciones a la Tabla Hash de Conexiones
            Clave[0] = '\0';
            strcat(Clave, ConexionesExistentes[k][0]);
            strcat(Clave, ConexionesExistentes[k][1]);

            AgregarElementoTablaHash(&TablaConexiones, Clave, InterseccionesReales);
        }

        if (InterseccionesReales == 0.0F)
        {
            // Es una conexión válida pues en realidad no intersecta nada en su camino, se añade la Conexion Valida a Conexiones Validas
            ConexionesValidas[ContadorConexionesValidas] = ConexionesExistentes[k];
            ContadorConexionesValidas++;
            //printf("Efectivamente, he encontrado una Conexión Válida sin intersecciones.");
        }
    }

    if (ContadorConexionesValidas == 0)
    {
        // No se han encontrado conexiones validas en el Grafo actual

        return 0;
    }
    else
    {
        // Generar Grafo Fase II
        Grafo GrafoSinInterseccion = GenerarGrafoFaseII(ConexionesValidas, ContadorConexionesValidas);

        // DEBUG - Imprimir Matriz de Adyacencia del Grafo sin Interseccion
        /*
        for(int u = 0; u < GrafoSinInterseccion.NumeroDeNodos; u++)
        {
            for(int y = 0; y < GrafoSinInterseccion.NumeroDeNodos; y++)
            {
                printf("%-15.6f   |", GrafoSinInterseccion.MatrizAdyacencia[u][y]);
            }
            printf("\n");
        }
        */

        // Generar Arbol de Cobertura Minima
        int NumeroNodosArbolCoberturaMinima = KruskalMST(GrafoSinInterseccion, ContadorConexionesValidas);

        return NumeroNodosArbolCoberturaMinima;
    }
}


/*******************************
Funciones del Algoritmo Genético
********************************/

// Variable que almacena a la Mejor Solucion
Individuo MejorSolucion;


// Funcion que Inicializa la Población
void InicializarPoblacion(Individuo Poblacion[], char*** TransmisoresPosibles) 
{
    for (int i = 0; i < TAMAÑO_POBLACION; i++) 
    {
        // Reservar memoria para los Genes, es decir, los Transmisores
        Poblacion[i].Genes = (char**) malloc(NUMERO_SENSORES * sizeof(char*));

        // Reservar memoria para los Indices de los Transmisores
        //Poblacion[i].IndicesTransmisoresActualesEnIndividuo = (int*) malloc(NUMERO_SENSORES * sizeof(int));

        //printf("\n Generando Individuo numero: %d", i);
        //printf("\n Sus Indices Aleatorios son: ");

        for (int j = 0; j < NUMERO_SENSORES; j++) 
        {     
            Poblacion[i].Genes[j] = (char*) malloc(100 * sizeof(char));

            int IndiceAleatorio = rand() % NumeroTransmisoresPosiblesPorSensor[j];
            Poblacion[i].Genes[j] = TransmisoresPosibles[j][IndiceAleatorio];

            // Añadir el Indice del Transmisor Elegido para el Individuo a IndicesTransmisoresActualesEnIndividuo
            Poblacion[i].IndicesTransmisoresActualesEnIndividuo[j] = IndiceAleatorio;

            // DEBUG - El Indice Aleatorio realmente es correcto
            //printf(" %d", IndiceAleatorio);
            //printf("\n Asi ha quedado el Gen: %s", Poblacion[i].Genes[j]);
        }
    }
}


// Funcion que evalua a un Individuo y retorna su Fitness
float EvaluarIndividuo (Individuo Individuo, PGconn *conn)
{
    float Fitness = 0;

    // Generar Grafo Fase I
    Grafo GrafoPrimigenio = GenerarGrafoFaseI(Individuo.Genes);

    // Evaluar la interseccion de cada conexion Transmisor/Sensor, Generar el Grafo sin Intersecciones y el MST
    int NumeroTransmisoresExitosos = EvaluarConexionesGrafo(Individuo.Genes, GrafoPrimigenio, conn);

    //printf("\n\nEl Individuo en evaluacion ha logrado conectar exitosamente %d Transmisores", NumeroTransmisoresExitosos);

    if (NumeroTransmisoresExitosos == 0)
    {
        Fitness = 0.0F;
        return Fitness;
    }
    
    if (NumeroTransmisoresExitosos == NUMERO_SENSORES)
    {
        // Este Individuo ha conectado todos los Transmisores, se ha encontrado una Solución Ideal
        Fitness = 1.0F;
        return Fitness;
    }
    else
    {
        // No ha conectado todos los Transmisores. Mientras mas Transmisores conectados, mas alto sera el Fitness, mas cerca de 1
        float Division = (float)1/(float)NumeroTransmisoresExitosos;
        Fitness = (1 - Division);
        return Fitness;
    }
}


// Función que evalua el Fitness de c/Individuo en la Poblacion
void EvaluarPoblacion(Individuo Poblacion[], PGconn *conn) 
{
    for (int i = 0; i < TAMAÑO_POBLACION; i++) 
    {
        //printf("\n Individuo %d en la Poblacion Inicial.", i);

        float Fitness = EvaluarIndividuo(Poblacion[i], conn);
        Poblacion[i].Fitness = Fitness;

        if (Poblacion[i].Fitness == 1.0F)
        {
            HayMejorSolucionEnPoblacionInicial = true;
            MejorSolucion = Poblacion[i];
            break;
        }

        char *ClaveIndividuo = (char *)malloc(5 * NUMERO_SENSORES * sizeof(char));
        ClaveIndividuo[0] = '\0';

        // Iterar sobre el arreglo de números y convertir cada número a ClaveIndividuo
        for (int k = 0; k < NUMERO_SENSORES; k++) 
        {
            // Utilizar un arreglo temporal para la conversión
            char Temp[20];
            Temp[0] = '\0';

            // Convertir el número a Char
            sprintf(Temp, "%d", Poblacion[i].IndicesTransmisoresActualesEnIndividuo[k]);

            // Concatenar la ClaveIndividuo convertida al arreglo principal
            strcat(ClaveIndividuo, Temp);
        }
        
        float FitnessActual = Poblacion[i].Fitness;

        AgregarElementoTablaHash(&TablaIndividuos, ClaveIndividuo, FitnessActual);
    }
}


// Selección de individuos para reproducción, con el Método de la Ruleta
int SeleccionRuleta(Individuo Poblacion[]) 
{
    float SumaFitness = 0;

    for (int i = 0; i < TAMAÑO_POBLACION; i++) 
    {
        SumaFitness += Poblacion[i].Fitness;
    }

    float r = fmod(rand(), SumaFitness);

    float Acumulado = 0;

    for (int i = 0; i < TAMAÑO_POBLACION; i++) 
    {
        Acumulado += Poblacion[i].Fitness;

        if (SumaFitness == 0)
        {
            return fmod(rand(), TAMAÑO_POBLACION);
        }
        else if (Acumulado >= r) 
        {
            return i;
        }
    }

    return -1;  
}


// Cruce de individuos, utilizando Cruce de un solo Punto
void CruzarIndividuos(Individuo Padre1, Individuo Padre2, Individuo *Hijo) 
{
    int PuntoCruce = rand() % NUMERO_SENSORES;

    for (int i = 0; i < PuntoCruce; i++) 
    {
        Hijo->Genes[i] = Padre1.Genes[i];
        
        // Añadir el Indice del Transmisor del Padre al Hijo
        Hijo->IndicesTransmisoresActualesEnIndividuo[i] = Padre1.IndicesTransmisoresActualesEnIndividuo[i];
    }

    for (int i = PuntoCruce; i < NUMERO_SENSORES; i++) 
    {
        // Añadir el Indice del Transmisor del Padre al Hijo
        Hijo->IndicesTransmisoresActualesEnIndividuo[i] = Padre2.IndicesTransmisoresActualesEnIndividuo[i];
        Hijo->Genes[i] = Padre2.Genes[i];
    }
}


// Mutación de un individuo
void MutarIndividuo(Individuo *Individuo, char*** TransmisoresPosibles) 
{
    for (int i = 0; i < NUMERO_SENSORES; i++) 
    {
        if ((double)rand() / RAND_MAX < PROBABILIDAD_MUTACION && NumeroTransmisoresPosiblesPorSensor[i] > 1) 
        {
            // Cambiar a otro transmisor posible para el sensor
            int IndiceAleatorio = rand() % NumeroTransmisoresPosiblesPorSensor[i];

            while (strcmp(TransmisoresPosibles[i][IndiceAleatorio], Individuo->Genes[i]) == 0)
            {
                IndiceAleatorio = rand() % NumeroTransmisoresPosiblesPorSensor[i];
            }

            Individuo->Genes[i] = TransmisoresPosibles[i][IndiceAleatorio];
        }
    }
}


// Funcion que encuentra al Peor Individuo en la Poblacion para ver si es reemplazable por el Hijo
int CalcularIndicePeorIndividuo(Individuo Poblacion[]) 
{
    float PeorFitness = INF;
    int IndicePeorIndividuo = -1;

    for (int i = 0; i < TAMAÑO_POBLACION; i++) 
    {
        if (Poblacion[i].Fitness < PeorFitness) 
        {
            PeorFitness = Poblacion[i].Fitness;
            IndicePeorIndividuo = i;
        }
    }

    return IndicePeorIndividuo;
}


/**********************************************
Funcion Principal que ejecuta todo el Algoritmo
***********************************************/

void main()
{
    /**********************************************************************************
    Leer Archivo que tiene las Coordenadas de los Sensores y almacenarlas en un Arreglo
    ***********************************************************************************/

    // Arreglo que ha de almacenar las Posiciones de los Sensores
    char PosicionesSensores[NUMERO_SENSORES][MAXIMA_LONGITUD_CADENA];
    // Útil para leer el archivo
    char buferArchivo[MAXIMA_LONGITUD_CADENA];
    // Abrir el archivo
    FILE *archivo = fopen(NOMBRE_ARCHIVO, "r");
  
    // Indice para saber en que linea se va
    int indice = 0;

    // Mientras se pueda leer una línea del archivo
    while (fgets(buferArchivo, MAXIMA_LONGITUD_CADENA, archivo))
    {
        // Remover salto de línea
        strtok(buferArchivo, "\n");
        // Copiar la línea al Arreglo, utilizando el índice
        memcpy(PosicionesSensores[indice], buferArchivo, MAXIMA_LONGITUD_CADENA);

        indice++;
    }

    // Cerrar el Archivo
    fclose(archivo);
   
    // DEBUG - Imprimir Posiciones de los Sensores
    /*
    int i;
    for (i = 0; i < NUMERO_SENSORES; i++)
    {
        printf("Sensor %d: '%s'\n", i+1, PosicionesSensores[i]);
    }
    */


    /******************************************************************************************
    Leer Archivo que tiene todos los Puntos de la Malla Triangular y almacenarlos en un Arreglo
    *******************************************************************************************/
    
    // Arreglo que ha de almacenar los Puntos de la Malla Triangular
    Puntos = malloc(sizeof(char[NUMERO_PUNTOS][MAXIMA_LONGITUD_CADENA]));
    // Útil para leer el archivo
    char buferArchivo2[MAXIMA_LONGITUD_CADENA];
    // Abrir el archivo
    FILE *archivo2 = fopen("Puntos.txt", "r");
    
    // Indice para saber en que linea se va
    int indice2 = 0;

    // Mientras se pueda leer una línea del archivo
    while (fgets(buferArchivo2, MAXIMA_LONGITUD_CADENA, archivo2))
    {
        // Remover salto de línea
        strtok(buferArchivo2, "\n");
        // Copiar la línea al Arreglo, utilizando el índice
        memcpy(Puntos[indice2], buferArchivo2, MAXIMA_LONGITUD_CADENA);

        indice2++;
    }

    // Cerrar el Archivo
    fclose(archivo2);

    // DEBUG - Imprimir los Puntos
    /*
    int j;
    for (j = 0; j < NUMERO_PUNTOS; j++)
    {
        printf("Punto %d: '%s'\n", j+1, Puntos[j]);
    }
    */


    /************************************************************************************************ 
    Obtener todos los Transmisores posibles para c/Sensor y almacenarlos en una Matriz Tridimensional
    *************************************************************************************************/

    char*** TransmisoresPosibles = CalcularTransmisoresPosibles(PosicionesSensores, Puntos);
    

    /*******************************************
    Iniciar Conexion a la Base de Datos Espacial
    ********************************************/

    PGconn *conn;
    conn = PQconnectdb("dbname=st user=postgres password=1234 host=localhost port=5432");

    if (PQstatus(conn) == CONNECTION_OK)
    {
        printf("Conexion exitosa\n");
    } 
    else 
    {
        fprintf(stderr, "Error de conexión: %s", PQerrorMessage(conn));
    }


    /***********************************
    Inicializar numeros pseudoaleatorios
    ************************************/

    srand(time(NULL));


    /**************************************
    Definiciones para el Algoritmo Genético
    ***************************************/
    
    // Inicialización de las Tablas Hash
    TablaConexiones = NULL;
    TablaIndividuos = NULL;

    /*
    // Crear Cronómetro
    clock_t Inicio, Fin;
    double Tiempo;

    // Iniciar el Cronómetro
    Inicio = clock();

    // DEBUG - El tiempo es correcto
    printf("Ha iniciado el Cronómetro\n");
    */

    struct timespec inicio, fin;
    double tiempo;

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // Definir el Fitness de la Mejor Solucion en 0
    MejorSolucion.Fitness = 0;

    // Variable para quebrar la ejecución del Código si se ha encontrado una Solución Ideal
    bool ExisteIndividuoIdeal = false;

    // Inicializar Población
    Individuo Poblacion[TAMAÑO_POBLACION];
    InicializarPoblacion(Poblacion, TransmisoresPosibles);

    // Evaluar Población inicial
    EvaluarPoblacion(Poblacion, conn);


    /*******************************
    Ejecución del Algoritmo Genético
    ********************************/

    if (HayMejorSolucionEnPoblacionInicial == false)
    {
        // Evolucionar la población durante un número fijo de Generaciones
        for (int Generacion = 0; Generacion < NUMERO_GENERACIONES; Generacion++) 
        {
            // Selección, Cruce y Mutación
            for (int i = 0; i < TAMAÑO_POBLACION; i++) 
            {
                // Preguntar si el Individuo Actual es una Solucion Ideal y conecta Todos los Transmisores
                if (Poblacion[i].Fitness == 1.0F)
                {
                    ExisteIndividuoIdeal = true;
                    MejorSolucion = Poblacion[i];
                    break;
                }
                else if (Poblacion[i].Fitness > MejorSolucion.Fitness)
                {
                    MejorSolucion = Poblacion[i];
                }

                // Selección
                int Padre1 = SeleccionRuleta(Poblacion);
                int Padre2 = SeleccionRuleta(Poblacion);
 
                // Ver si se realizara el Cruce, con un Pseudoaleatorio que puede o no ser menor a la Pobabilidad de Cruce
                double Probabilidad = (double)rand() / RAND_MAX;

                if (Probabilidad < PROBABILIDAD_CRUCE) 
                {
                    // Cruce
                    Individuo Hijo;
                    
                    // Reservar memoria para los genes del Hijo y realizar Cruce para generar el Hijo
                    Hijo.Genes = (char**) malloc(NUMERO_SENSORES * sizeof(char*));

                    CruzarIndividuos(Poblacion[Padre1], Poblacion[Padre2], &Hijo);

                    // Mutación del Hijo, que puede o no ocurrir
                    MutarIndividuo(&Hijo, TransmisoresPosibles);


                    /*********************************************************************************************************
                    Creación de Clave para almacenar Fitness del Hijo en Tabla Hash, la Clave es homologa a los Genes del Hijo
                    **********************************************************************************************************/

                    char *ClaveIndividuo = (char *)malloc(5 * NUMERO_SENSORES * sizeof(char));
                    ClaveIndividuo[0] = '\0';

                    // Iterar sobre el arreglo de números y convertir cada número a ClaveIndividuo
                    for (int k = 0; k < NUMERO_SENSORES; k++) 
                    {
                        // Utilizar un arreglo temporal para la conversión
                        char Temp[20];
                        Temp[0] = '\0';

                        // Convertir el número a Char
                        sprintf(Temp, "%d", Hijo.IndicesTransmisoresActualesEnIndividuo[k]);

                        // Concatenar la ClaveIndividuo convertida al arreglo principal
                        strcat(ClaveIndividuo, Temp);
                    }
                    
                    // DEBUG - Imprimir Clave del Hijo
                    //printf("\nLa Clave de Hijo a ingresar es: %s", ClaveIndividuo);

                    if (EncontrarElementoTablaHash(&TablaIndividuos, ClaveIndividuo))
                    {
                        // DEBUG - Imprimir el Fitness del Individuo ya visto
                        //printf("\nIndividuo ya visto, su Fitness es: %f", EncontrarElementoTablaHash(&TablaIndividuos, ClaveIndividuo)
                        //->Valor);

                        Hijo.Fitness = EncontrarElementoTablaHash(&TablaIndividuos, ClaveIndividuo)->Valor;
                    }
                    else
                    {
                        // Evaluar el hijo
                        Hijo.Fitness = EvaluarIndividuo(Hijo, conn);

                        // Añadir Fitness del Hijo a TablaIndividuos
                        AgregarElementoTablaHash(&TablaIndividuos, ClaveIndividuo, Hijo.Fitness);
                    }

                    // Preguntar si el Hijo recien generado es una Solucion Ideal y conecta Todos los Transmisores
                    if (Hijo.Fitness == 1.0F)
                    {
                        ExisteIndividuoIdeal = true;
                        MejorSolucion = Hijo;
                        break;
                    }

                    // Reemplazar el Peor Individuo con el Hijo si este último tiene un mejor Fitness respecto al Peor Individuo
                    int IndicePeorIndividuo = CalcularIndicePeorIndividuo(Poblacion);

                    if (Hijo.Fitness > Poblacion[IndicePeorIndividuo].Fitness) 
                    {
                        Poblacion[IndicePeorIndividuo] = Hijo;
                    }
                }
            }

            if (ExisteIndividuoIdeal == true)
            {
                printf("\n\n\nSe ha encontrado una Solución que conecta todos los Transmisores.");
                break;
            }

            // DEBUG - Imprimir Generación actual
            printf("\nHa finalizado la Generacion: %d", Generacion);
        }
    }
    else
    {
        ExisteIndividuoIdeal = true;
        printf("\n\n\nSe ha encontrado una Solución que conecta todos los Transmisores.");
    }

    // Cerrar la Conexión a la Base de Datos Espacial
    PQfinish(conn);

    // No se encontro una solucion ideal
    if (ExisteIndividuoIdeal == false)
    {
        printf("\nNo se encontro una Solucion Ideal que conecta todos los Transmisores.");
    }

    printf("\nEl Fitness de la Mejor Solución es: %f", MejorSolucion.Fitness);

    // Imprimir la Mejor Solucion
    printf("\n\nEstas son las ubicaciones de los Transmisores de la Mejor Solucion.");

    for (int i = 0; i < NUMERO_SENSORES; i++)
    {
        printf("\nTransmisor %d: %s", i, MejorSolucion.Genes[i]);
    }

    /*
    // Detener el Cronómetro
    Fin = clock();

    // Imprimir el tiempo de ejecucion
    Tiempo = ((double)(Fin - Inicio)) / CLOCKS_PER_SEC;
    */

    clock_gettime(CLOCK_MONOTONIC, &fin);

    tiempo = (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("\n\nEl tiempo que ha tardado el Algoritmo es: %.9f Segundos\n", tiempo);
}