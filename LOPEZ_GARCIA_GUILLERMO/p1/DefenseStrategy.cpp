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
                int ce_row = -1, int ce_col = -1)
{
    try // Por si acaso freeCells no es del tamaño esperado, se recoje la excepcion y se devuelve una valoracion negativa
    {
        if (ce_row >= 0 && ce_col >= 0) // Comprueba si la fila y columna es mayor que cero
        {
            if(ce_row < nCellsWidth && ce_col < nCellsHeight) // Comprueba que la fila y columna este dentro de los margenes del trablero
            {
                if(freeCells[ce_row][ce_col]) // Comprobacion si el centro de la celda esta disponible
                {
                    int celdaCentroY = nCellsWidth;
                    int celdaCentroX = nCellsHeight;
                    
                    float a = 0.0; // Valoracion de la casilla central en el eje Y
                    if(ce_row >= celdaCentroY)
                        a = ce_row*2 - celdaCentroY;
                    else if(ce_row < celdaCentroY)
                        a = ce_row;
                    
                    float b = 0.0; // Valoracion de la casilla central en el eje X
                    if(ce_col >= celdaCentroX)
                        b = ce_col*2 - celdaCentroX;
                    else if(ce_col < celdaCentroX)
                        b = ce_col;
                    
                    return a * b; // La mejor valoracion es el centro del tablero
                } else return 0.0;
            }
        }
    } catch(std::exception &e) {}
    return -1; // Por si se manda a valorar una celda que no esta dentro del tablero
}

// Esta funcion comprueba si una defensa se puede colocar
bool factibilidad(int row, int col, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
                  List<Object*> obstacles, List<Object*> defenses, Object* base = nullptr)
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
        
        if(base != nullptr)
        {
            for (auto o: obstacles) {
                if(base == o) continue;
                
                float xObstacle = o->position.x;
                float yObstacle = o->position.y;
                Vector3 po = Vector3(xObstacle, yObstacle, 0);
                
                if(_distance(pf, po) < (base->radio + o->radio)) return false;
            }
            
            for (auto d: defenses) {
                if(base == d) continue;
                
                float xDefense = d->position.x;
                float yDefense = d->position.y;
                Vector3 pd = Vector3(xDefense, yDefense, 0);
                
                if(_distance(pf, pd) < (base->radio + d->radio)) return false;
            }
            
            if(pf.x + base->radio > mapWidth || pf.x - base->radio < 0)
                return false;
            
            if(pf.y + base->radio > mapHeight || pf.y - base->radio < 0)
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
    
    int maxAttemps = 1000;
    List<Defense*>::iterator currentDefense = defenses.begin();
    
    while(currentDefense != defenses.end() && maxAttemps > 0) {
        (*currentDefense)->position.x = ((int)(_RAND2(nCellsWidth))) * cellWidth + cellWidth * 0.5f;
        (*currentDefense)->position.y = ((int)(_RAND2(nCellsHeight))) * cellHeight + cellHeight * 0.5f;
        (*currentDefense)->position.z = 0;
        ++currentDefense;
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
