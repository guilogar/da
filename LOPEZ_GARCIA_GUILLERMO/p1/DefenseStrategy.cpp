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

using namespace Asedio;

float cellValue(int row, int col, bool** freeCells, int nCellsWidth, int nCellsHeight,
                float mapWidth, float mapHeight, List<Object*> obstacles, List<Defense*> defenses,
                int ce_row = -1, int ce_col = -1)
{
    
    return 0; // implemente aqui la función que asigna valores a las celdas
}

bool factibilidad(int row, int col, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
                  List<Object*> obstacles, List<Object*> defenses, Object* base)
{
    /*
     * 1. que no se salga del tablero
     * 2. que no colisione con un obstaculo
     * 3. que no colisione con las defensas que estan colocadas
     */
    bool factible = true;
    
    if(row >= 0 && row <= nCellsWidth && col >= 0 && col <= nCellsHeight)
    {
        float cellWidth = mapWidth / nCellsWidth;
        float cellHeight = mapHeight / nCellsHeight;
        
        float posicionCentroCeldaX = row + cellWidth;
        float posicionCentroCeldaY = col + cellHeight;
        Vector3 pf = Vector3(posicionCentroCeldaX, posicionCentroCeldaY, 0);
        
        for (auto o: obstacles) {
            float xObstacle = o->position.x;
            float yObstacle = o->position.y;
            Vector3 po = Vector3(xObstacle, yObstacle, 0);
            
            _distance(pf, po);
        }
    }
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
