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
                        a = row*2 - celdaCentroY;
                    else if(row < celdaCentroY)
                        a = row;
                    
                    float b = 0.0; // Valoracion de la casilla central en el eje X
                    if(col >= celdaCentroX)
                        b = col*2 - celdaCentroX;
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
                  List<Object*> obstacles, List<Object*> defenses, Object* defense = nullptr)
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
        
        if(defense != nullptr)
        {
            for (auto o: obstacles) {
                if(defense == o) continue;
                
                float xObstacle = o->position.x;
                float yObstacle = o->position.y;
                Vector3 po = Vector3(xObstacle, yObstacle, 0);
                
                if(_distance(pf, po) < (defense->radio + o->radio)) return false;
            }
            
            for (auto d: defenses) {
                if(defense == d) continue;
                
                float xDefense = d->position.x;
                float yDefense = d->position.y;
                Vector3 pd = Vector3(xDefense, yDefense, 0);
                
                if(_distance(pf, pd) < (defense->radio + d->radio)) return false;
            }
            
            if(pf.x + defense->radio > mapWidth || pf.x - defense->radio < 0)
                return false;
            
            if(pf.y + defense->radio > mapHeight || pf.y - defense->radio < 0)
                return false;
        } else return false;
        
        factible = true;
    }

    return factible;
}

void DEF_LIB_EXPORTED placeDefenses(bool** freeCells, int nCellsWidth, int nCellsHeight, float mapWidth,
                                    float mapHeight, std::list<Object*> obstacles, std::list<Defense*> defenses)
{
    float cellWidth = mapWidth / nCellsWidth;
    float cellHeight = mapHeight / nCellsHeight;
    
    float tablero[nCellsHeight][nCellsWidth];
    
    float maxValue = 0.0;
    int base_i = 0;
    int base_j = 0;
    
    for (int i = 0; i < nCellsHeight; i++)
    {
        for (int j = 0; j < nCellsWidth; j++)
        {
            tablero[i][j] = cellValue(i, j, freeCells, nCellsWidth, nCellsHeight,
                                      mapWidth, mapHeight, obstacles, defenses,
                                      true);
            
        }
    }
    List<Defense*>::iterator base = defenses.begin();
    
    // Falta meter en un vector una estructura que identifique el valor de la celda, y su x e y coordenada
    // Una vez tenga eso, ordeno el vector y voy entrayendo en orden de mayor a menor valoracion
    // Utilizo la funcion de factibilidad, y si es factible esa posicion, pongo la base.
    // Despues, se vuelve a valorar todas y cada una de las celdas, y sorpresa sorpresa, se colocan las defensas.
    int maxAttemps = 1000;
    
    
    /*
     *while(currentDefense != defenses.end() && maxAttemps > 0) {
     *    (*currentDefense)->position.x = ((int)(_RAND2(nCellsWidth))) * cellWidth + cellWidth * 0.5f;
     *    (*currentDefense)->position.y = ((int)(_RAND2(nCellsHeight))) * cellHeight + cellHeight * 0.5f;
     *    (*currentDefense)->position.z = 0;
     *    ++currentDefense;
     *}
     */
    
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
