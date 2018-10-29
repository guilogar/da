// ###### Config options ################

//#define PRINT_DEFENSE_STRATEGY 1    // generate map images

// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"

#ifdef PRINT_DEFENSE_STRATEGY
#include "ppm.h"
#endif

#ifdef CUSTOM_RAND_GENERATOR
RAND_TYPE SimpleRandomGenerator::a;
#endif

#include <exception>

using namespace Asedio;
using namespace std;

struct Valoracion // Estructura para guardar una valoracion, con la fila y columna que se hace
{
    float value_;
    int i_, j_;
    Valoracion(int i = 0, int j = 0, int value = 0.0) : i_(i), j_(j), value_(value) {}
    bool operator < (const Valoracion& v) { return ( value_ < v.value_ ); }
};

float cellValue(int row, int col, bool** freeCells, int nCellsWidth, int nCellsHeight,
                float mapWidth, float mapHeight, List<Object*> obstacles, List<Defense*> defenses,
                bool isBase = false)
{
    try // Por si acaso freeCells no es del tamaño esperado, se recoje la excepcion y se devuelve una valoracion negativa
    {
        if (row >= 0 && col >= 0) // Comprueba si la fila y columna es mayor que cero
        {
            if(row < nCellsWidth && col < nCellsHeight) // Comprueba que la fila y columna este dentro de los margenes del trablero
            {
                if(freeCells[row][col]) // Comprobacion si el centro de la celda esta disponible
                {
                    int celdaCentroY = nCellsWidth;
                    int celdaCentroX = nCellsHeight;
                    
                    float a = 0.0; // Valoracion de la casilla central en el eje Y
                    if(row >= celdaCentroY)
                        a = celdaCentroY*2 - row;
                    else if(row < celdaCentroY)
                        a = row;
                    
                    float b = 0.0; // Valoracion de la casilla central en el eje X
                    if(col >= celdaCentroX)
                        b = celdaCentroX*2 - col;
                    else if(col < celdaCentroX)
                        b = col;
                    
                    return a * b; // La mejor valoracion es el centro del tablero
                } else return 0.0;
            }
        }
    } catch(std::exception &e) {}
    return -1; // Por si se manda a valorar una celda que no esta dentro del tablero
}

// Esta funcion comprueba si una defensa se puede colocar
bool factibilidad(int row, int col, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
                  List<Object*> obstacles, List<Defense*> defenses, Object* defense = NULL)
{
    /*
     * 1. que no se salga del tablero
     * 2. que no colisione con un obstaculo
     * 3. que no colisione con las defensas que estan colocadas
     */
    bool factible = false;
    
    if(row >= 0 && row <= nCellsWidth && col >= 0 && col <= nCellsHeight)
    {
        float cellWidth = mapWidth / nCellsWidth;
        float cellHeight = mapHeight / nCellsHeight;
        
        float posicionCentroCeldaX = row + cellWidth;
        float posicionCentroCeldaY = col + cellHeight;
        Vector3 pf = Vector3(posicionCentroCeldaX, posicionCentroCeldaY, 0);
        
        if(defense != NULL)
        {
            List<Object*>::iterator o = obstacles.begin();
            while (o != obstacles.end())
            {
                float xObstacle = (*o)->position.x;
                float yObstacle = (*o)->position.y;
                Vector3 po = Vector3(xObstacle, yObstacle, 0);
                
                /*
                 *std::cout << "Vector3 pf = " << pf.x << ", " << pf.y << ", " << pf.z << std::endl;
                 *std::cout << "Vector3 po = " << po.x << ", " << po.y << ", " << po.z << std::endl;
                 */
                
                if(_distance(pf, po) < (defense->radio + (*o)->radio)) return false;
                ++o;
            }
            
            List<Defense*>::iterator d = defenses.begin();
            while (d != defenses.end())
            {
                if(defense == (*d)) { ++d; continue; } // Si estamos comparando una defensa con ella misma
                
                float xDefense = (*d)->position.x;
                float yDefense = (*d)->position.y;
                
                if(xDefense < 0 || yDefense < 0) { ++d; continue; } // Si la defensa no esta colocada, no se compara con ella
                
                Vector3 pd = Vector3(xDefense, yDefense, 0);
                
                /*
                 *std::cout << "Vector3 pf = " << pf.x << ", " << pf.y << ", " << pf.z << std::endl;
                 *std::cout << "Vector3 pd = " << pd.x << ", " << pd.y << ", " << pd.z << std::endl;
                 */
                if(_distance(pf, pd) < (defense->radio + (*d)->radio)) return false;
                /*
                 *std::cout << "Distancia => " << _distance(pf, pd) << std::endl;
                 *std::cout << "Suma de radios => " << ((defense->radio + (*d)->radio)) << std::endl;
                 */
                ++d;
            }
            
            // Si la defensa se sale por los bordes
            if(pf.x + defense->radio > mapWidth || pf.x - defense->radio < 0) return false;
            if(pf.y + defense->radio > mapHeight || pf.y - defense->radio < 0) return false;
            
            return true; // Para todas las condiciones
        }
    }

    return factible;
}

std::vector<Valoracion> obtenerValoraciones(bool** freeCells, int nCellsWidth, int nCellsHeight,
                                            float mapWidth, float mapHeight, std::list<Object*> obstacles,
                                            std::list<Defense*> defenses, bool isBase = false)
{
    std::vector<Valoracion> valoracionesCeldas; // Vector de valoraciones
    
    for (int i = 0; i < nCellsHeight; i++)
    {
        for (int j = 0; j < nCellsWidth; j++)
        {
            float v = cellValue(i, j, freeCells, nCellsWidth, nCellsHeight,
                                mapWidth, mapHeight, obstacles, defenses,
                                isBase);
            valoracionesCeldas.push_back(Valoracion(i, j, v)); // Se valora una celda
        }
    }
    
    std::sort(valoracionesCeldas.begin(), valoracionesCeldas.end()); // Se ordena el vector de mayor a menor valoracion
    
    return valoracionesCeldas;
}

void DEF_LIB_EXPORTED placeDefenses(bool** freeCells, int nCellsWidth, int nCellsHeight, float mapWidth,
                                    float mapHeight, std::list<Object*> obstacles, std::list<Defense*> defenses)
{
    float cellWidth = mapWidth / nCellsWidth;
    float cellHeight = mapHeight / nCellsHeight;
    
    // Vector de valoraciones para la base
    std::vector<Valoracion> valoracionesCeldas =
        obtenerValoraciones(freeCells, nCellsWidth, nCellsHeight,
                            mapWidth, mapHeight, obstacles, defenses, true);
    
    //if(!defenses.empty())
    List<Defense*>::iterator currentDefense = defenses.begin(); // Se instancia todas las posiciones de las defensas a un
                                                                // valor negativo para indicar que no estan colocadas.
    while(currentDefense != defenses.end())
    {
        (*currentDefense)->position.x = -1;
        (*currentDefense)->position.y = -1;
        (*currentDefense)->position.z = -1;
        ++currentDefense;
    }
    
    currentDefense = defenses.begin(); // Se actualiza el iterador al principio
    while(!valoracionesCeldas.empty()) // Mientras que hayas celdas disponibles
    {
        int row = valoracionesCeldas.rbegin()->i_;
        int col = valoracionesCeldas.rbegin()->j_;
        
        // Se comprueba que esa celda sea factible para colocar la defensa (como es la primera, la base)
        if(factibilidad(row, col, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses, (*currentDefense)))
        {
            //std::cout << "Es valido para => " << row << ", " << col << std::endl;
            (*currentDefense)->position.x = row;
            (*currentDefense)->position.y = col;
            (*currentDefense)->position.z = 0;
            break;
        }
        valoracionesCeldas.pop_back(); // Se actualiza el vector de valoraciones
    }
    
    // Vector de valoraciones para el resto de las defensas
    valoracionesCeldas = obtenerValoraciones(freeCells, nCellsWidth, nCellsHeight,
                                              mapWidth, mapHeight, obstacles, defenses);
    
    while(++currentDefense != defenses.end() && !valoracionesCeldas.empty())
    {
        int row = valoracionesCeldas.rbegin()->i_;
        int col = valoracionesCeldas.rbegin()->j_;
        
        // Se comprueba que esa celda sea factible para colocar la defensa (como es la primera, la base)
        if(factibilidad(row, col, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses, (*currentDefense)))
        {
            //std::cout << "Es valido para => " << row << ", " << col << std::endl;
            (*currentDefense)->position.x = row;
            (*currentDefense)->position.y = col;
            (*currentDefense)->position.z = 0;
            //std::cout << (*currentDefense)->position.x << std::endl;
        }
        valoracionesCeldas.pop_back(); // Se actualiza el vector de valoraciones
    }
    
    #ifdef PRINT_DEFENSE_STRATEGY
    
        float** cellValues = new float* [nCellsHeight];
        for(int i = 0; i < nCellsHeight; ++i) {
           cellValues[i] = new float[nCellsWidth];
           for(int j = 0; j < nCellsWidth; ++j) {
               cellValues[i][j] = ((int)(cellValue(i, j))) % 256;
           }
        }
        
        dPrintMap("strategy.ppm", nCellsHeight, nCellsWidth, cellHeight, cellWidth,
                  freeCells , cellValues, std::list<Defense*>(), true);
        
        for(int i = 0; i < nCellsHeight; ++i) delete [] cellValues[i];
        delete [] cellValues;
        cellValues = NULL;
    
    #endif
}
