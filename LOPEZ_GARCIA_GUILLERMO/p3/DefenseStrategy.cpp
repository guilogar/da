// ###### Config options ################



// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"
#include "cronometro.h"

using namespace Asedio;

Vector3 cellCenterToPosition(int i, int j, float cellWidth, float cellHeight)
{
    return Vector3((j * cellWidth) + cellWidth * 0.5f, (i * cellHeight) + cellHeight * 0.5f, 0);
}

struct Valoracion // Estructura para guardar una valoracion, con la fila y columna que se hace
{
    float value_;
    int i_, j_;
    Valoracion(int i = 0, int j = 0, int value = 0.0) : i_(i), j_(j), value_(value) {}
    bool operator < (const Valoracion& v) { return ( value_ < v.value_ ); }
};

float defaultCellValue(int row, int col, bool** freeCells, int nCellsWidth, int nCellsHeight,
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
                    int celdaCentroY = nCellsWidth / 2;
                    int celdaCentroX = nCellsHeight / 2;

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
    if(row >= 0 && row <= nCellsWidth && col >= 0 && col <= nCellsHeight)
    {
        float cellWidth = mapWidth / nCellsWidth;
        float cellHeight = mapHeight / nCellsHeight;

        Vector3 pf = cellCenterToPosition(row, col, cellWidth, cellHeight);

        if(defense != NULL)
        {
            List<Object*>::iterator o = obstacles.begin();
            while (o != obstacles.end())
            {
                float xObstacle = (*o)->position.x;
                float yObstacle = (*o)->position.y;
                Vector3 po = Vector3(xObstacle, yObstacle, 0);

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

                if(_distance(pf, pd) < (defense->radio + (*d)->radio)) return false;
                ++d;
            }

            // Si la defensa se sale por los bordes
            if(pf.x + defense->radio > mapWidth || pf.x - defense->radio < 0) return false;
            if(pf.y + defense->radio > mapHeight || pf.y - defense->radio < 0) return false;

            return true; // Pasa todas las condiciones
        }
    }
    return false;
}

std::vector<Valoracion> ordenacionPorFusion(std::vector<Valoracion>& vector)
{
    if(vector.size() <= 1) return vector;
    else if(vector.size() == 2)
    {
        if(vector[0] < vector[1])
        {
            Valoracion v = vector[0];
            vector[0] = vector[1];
            vector[1] = v;
        }
    }
    else
    {
        int tamanio = vector.size();
        int mitad = tamanio / 2;
        std::vector<Valoracion> primeraMitad;
        std::vector<Valoracion> segundaMitad;
        
        for (int i = 0; i < mitad; i++) { primeraMitad.push_back(vector[i]); }
        for (int i = mitad; i < tamanio; i++) { segundaMitad.push_back(vector[i]); }
        
        primeraMitad = ordenacionPorFusion(primeraMitad);
        segundaMitad = ordenacionPorFusion(segundaMitad);
        vector.clear();
        
        for (int i = 0; i < primeraMitad.size(); i++)
            vector.push_back(primeraMitad[i]);
        
        for (int i = 0; i < segundaMitad.size(); i++)
            vector.push_back(segundaMitad[i]);
    }
    
    return vector;
}

int obtenerPivote(std::vector<Valoracion> vector)
{
    int p = 0;
    
    if(vector.size() > 0)
    {
        Valoracion v = vector[0];
        for (int k = 1; k < vector.size(); k++)
        {
            if(v < vector[k])
            {
                Valoracion va = vector[p];
                vector[p] = vector[k];
                vector[k] = va;
                p++;
            }
        }
        vector[0] = vector[p];
        vector[p] = v;
    }
    return p;
}

std::vector<Valoracion> ordenacionRapida(std::vector<Valoracion>& vector)
{
    if(vector.size() <= 1) return vector;
    else if(vector.size() == 2)
    {
        if(vector[0] < vector[1])
        {
            Valoracion v = vector[0];
            vector[0] = vector[1];
            vector[1] = v;
        }
    }
    else
    {
        int tamanio = vector.size();
        int pivote = obtenerPivote(vector);
        if(pivote == 0) pivote = tamanio / 2;
        
        std::vector<Valoracion> primeraMitad;
        std::vector<Valoracion> segundaMitad;
        
        for (int i = 0; i < pivote; i++) { primeraMitad.push_back(vector[i]); }
        for (int i = pivote; i < tamanio; i++) { segundaMitad.push_back(vector[i]); }
        
        primeraMitad = ordenacionRapida(primeraMitad);
        segundaMitad = ordenacionRapida(segundaMitad);
        vector.clear();
        
        for (int i = 0; i < primeraMitad.size(); i++)
            vector.push_back(primeraMitad[i]);
        
        for (int i = 0; i < segundaMitad.size(); i++)
            vector.push_back(segundaMitad[i]);
    }
    
    return vector;
}

std::vector<Valoracion> obtenerValoraciones(bool** freeCells, int nCellsWidth, int nCellsHeight,
                                            float mapWidth, float mapHeight, std::list<Object*> obstacles,
                                            std::list<Defense*> defenses, bool isBase = false,
                                            unsigned int modOrdenacion = 0)
{
    std::vector<Valoracion> valoracionesCeldas; // Vector de valoraciones
    
    for (int i = 0; i < nCellsHeight; i++)
    {
        for (int j = 0; j < nCellsWidth; j++)
        {
            float v = defaultCellValue(i, j, freeCells, nCellsWidth, nCellsHeight,
                                       mapWidth, mapHeight, obstacles, defenses,
                                       isBase);
            valoracionesCeldas.push_back(Valoracion(i, j, v)); // Se valora una celda
        }
    }
    
    switch (modOrdenacion) {
        case 0: break; // Sin ordenación
        case 1:
            {
                valoracionesCeldas = ordenacionPorFusion(valoracionesCeldas);
            } break;
        case 2:
            {
                valoracionesCeldas = ordenacionRapida(valoracionesCeldas);
            } break;
        case 3:
            {
                std::make_heap(valoracionesCeldas.begin(), valoracionesCeldas.end());
            } break;
        default:
            std::sort(
                    valoracionesCeldas.begin(),
                    valoracionesCeldas.end()
            ); // Se ordena el vector de menor a mayor valoracion
    }
    
    return valoracionesCeldas;
}

void DEF_LIB_EXPORTED placeDefenses3(bool** freeCells, int nCellsWidth, int nCellsHeight,
                                     float mapWidth, float mapHeight, List<Object*> obstacles,
                                     List<Defense*> defenses)
{
    float cellWidth = mapWidth / nCellsWidth;
    float cellHeight = mapHeight / nCellsHeight;
    const double e_abs = 0.01, // Máximo error absoluto cometido.
                 e_rel = 0.001; // Máximo error relativo aceptado.
    
    // modOrdenacion 0: Sin ordenación
    // modOrdenacion 1: Ordenación mediante fusión
    // modOrdenacion 2: Ordenación rápida
    // modOrdenacion 3: Ordenación mediante montículo con la stl.
    
    // Bucle indicar el modo de ordenacion cuando se llame a la funcion "obtenerValoraciones"
    for (int modOrdenacion = 0; modOrdenacion < 4; modOrdenacion++)
    {
        long int r = 0;
        cronometro c;
        c.activar();
        
        do {
            // Empieza la estrategia de colocacion
            // =============================================================================
            // =============================================================================
            
            // Vector de valoraciones para la base
            std::vector<Valoracion> valoracionesCeldas =
                obtenerValoraciones(freeCells, nCellsWidth, nCellsHeight,
                                    mapWidth, mapHeight, obstacles,
                                    defenses, true, modOrdenacion); // i vale 0, 1, 2 o 3
            
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
                int row;
                int col;
                
                if(modOrdenacion == 0) // Para el caso de sin ordenacion previa
                {
                    Valoracion v = valoracionesCeldas[0];
                    for (int i = 1; i < valoracionesCeldas.size(); i++)
                    {
                        if(v < valoracionesCeldas[i])
                        {
                            v = valoracionesCeldas[i];
                        }
                    }
                    row = v.i_;
                    col = v.j_;
                } else if(modOrdenacion == 3) // Ordenacion con monticulo
                {
                    row = valoracionesCeldas.front().i_;
                    col = valoracionesCeldas.front().j_;
                } else // Caso con ordenacion con fusion o rapida
                {
                    row = valoracionesCeldas.rbegin()->i_;
                    col = valoracionesCeldas.rbegin()->j_;
                }
                
                // Se comprueba que esa celda sea factible para colocar la defensa (como es la primera, la base)
                if(factibilidad(row, col, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses, (*currentDefense)))
                {
                    Vector3 v = cellCenterToPosition(row, col, cellWidth, cellHeight);
                    (*currentDefense)->position.x = v.x;
                    (*currentDefense)->position.y = v.y;
                    (*currentDefense)->position.z = 0;
                    break; // Se rompe la iteración del bucle para no continuar, ya que, aquí se inserta la primera defensa (la base)
                }
                
                if(modOrdenacion == 3)
                    std::pop_heap(valoracionesCeldas.begin(), valoracionesCeldas.end());
                
                valoracionesCeldas.pop_back(); // Se actualiza el vector de valoraciones
            }
            
            // Vector de valoraciones para el resto de las defensas
            /*
             *valoracionesCeldas = obtenerValoraciones(freeCells, nCellsWidth, nCellsHeight,
             *                                          mapWidth, mapHeight, obstacles, defenses);
             */
            // No se vuelven a evaluar las defensas, por eso se comenta esta línea anterior.
            
            while(++currentDefense != defenses.end())
            {
                while(!valoracionesCeldas.empty())
                {
                    int row;
                    int col;
                    
                    if(modOrdenacion == 0) // Para el caso de sin ordenacion previa
                    {
                        Valoracion v = valoracionesCeldas[0];
                        for (int i = 1; i < valoracionesCeldas.size(); i++)
                        {
                            if(v < valoracionesCeldas[i])
                            {
                                v = valoracionesCeldas[i];
                            }
                        }
                        row = v.i_;
                        col = v.j_;
                    } else if(modOrdenacion == 3) // Ordenacion con monticulo
                    {
                        row = valoracionesCeldas.front().i_;
                        col = valoracionesCeldas.front().j_;
                    } else // Caso con ordenacion con fusion o rapida
                    {
                        row = valoracionesCeldas.rbegin()->i_;
                        col = valoracionesCeldas.rbegin()->j_;
                    }
                    
                    // Se comprueba que esa celda sea factible para colocar la defensa
                    if(factibilidad(row, col, nCellsWidth, nCellsHeight, mapWidth, mapHeight,
                                    obstacles, defenses, (*currentDefense)))
                    {
                        Vector3 v = cellCenterToPosition(row, col, cellWidth, cellHeight);
                        (*currentDefense)->position.x = v.x;
                        (*currentDefense)->position.y = v.y;
                        (*currentDefense)->position.z = 0;
                        break; // Se rompe esta iteración porque ya se ha conseguido colocar la defensa con una valoracion
                    }
                    
                    if(modOrdenacion == 3)
                        std::pop_heap(valoracionesCeldas.begin(), valoracionesCeldas.end());
                    
                    valoracionesCeldas.pop_back(); // Se actualiza el vector de valoraciones
                }
            }
            
            // =============================================================================
            // =============================================================================
            // Termina la estrategia de colocacion
            
            ++r;
        } while(c.tiempo() < e_abs / e_rel + e_abs);
        
        c.parar();
        
        std::cout << (nCellsWidth * nCellsHeight) << '\t' << c.tiempo() / r << '\t';
        std::cout << c.tiempo()*2 / r << '\t' << c.tiempo()*3 / r << '\t' << c.tiempo()*4 / r << std::endl;
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
