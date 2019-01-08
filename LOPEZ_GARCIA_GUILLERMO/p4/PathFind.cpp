// ###### Config options ################

#define PRINT_PATHS 1

// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"

#ifdef PRINT_PATHS
#include "ppm.h"
#endif

#include <algorithm>
#include <vector>
#include <iostream>

using namespace Asedio;

Vector3 cellCenterToPosition(int i, int j, float cellWidth, float cellHeight){ 
    return Vector3((j * cellWidth) + cellWidth * 0.5f, (i * cellHeight) + cellHeight * 0.5f, 0); 
}

void DEF_LIB_EXPORTED calculateAdditionalCost(float** additionalCost, int cellsWidth,
                                              int cellsHeight, float mapWidth,
                                              float mapHeight, List<Object*> obstacles,
                                              List<Defense*> defenses)
{
    float cellWidth = mapWidth / cellsWidth;
    float cellHeight = mapHeight / cellsHeight;
    
    for(int i = 0 ; i < cellsHeight ; ++i)
    {
        for(int j = 0 ; j < cellsWidth ; ++j)
        {
            Vector3 cellPosition = cellCenterToPosition(i, j, cellWidth, cellHeight);
            float cost = 0;
            
            if( (i+j) % 2 == 0 ) cost = cellWidth * 100;
            
            additionalCost[i][j] = cost;
        }
    }
}

bool nodoEnLista(AStarNode* nodo, std::vector<AStarNode*> lista)
{
    return (lista.end() != std::find(lista.begin(), lista.end(), nodo));
}

void DEF_LIB_EXPORTED calculatePath(AStarNode* originNode, AStarNode* targetNode,
                                    int cellsWidth, int cellsHeight, float mapWidth,
                                    float mapHeight, float** additionalCost,
                                    std::list<Vector3> &path)
{
    unsigned long iter = 0;
    float min = INF_F;                      // Distancia minima es el maximo valor para los float en esta maquina
    AStarNode* current = originNode;
    std::list<Vector3> auxiliarPath;
    
    std::vector<AStarNode*> nodosCerrados;
    
    std::vector<AStarNode*> nodosAbiertos;
    nodosAbiertos.push_back(current);
    
    std::make_heap (nodosAbiertos.begin(), nodosAbiertos.end());
    
    while(!nodosAbiertos.empty())
    {
        // Itera mientras que el nodo actual sea distinto del nodo final
        while(current != NULL && current != targetNode && !nodosAbiertos.empty())
        {
            current = nodosAbiertos.front(); // Se obtiene el ultimo nodo adyacente agregado
            std::pop_heap(nodosAbiertos.begin(), nodosAbiertos.end());
            nodosAbiertos.pop_back(); // Se elimina dicho nodo de la lista de abiertos
            
            nodosCerrados.push_back(current); // Se añade el nodo actual a la lista de cerrados
            for (auto nodo : current->adjacents) // Se añade a la lista de abiertos todos los adyacentes
            {
                if(!nodoEnLista(nodo, nodosCerrados))
                {
                    float distTarget = _sdistance(nodo->position, targetNode->position);
                    float distParent = _sdistance(nodo->position, current->position);
                    if(!nodoEnLista(nodo, nodosAbiertos))
                    {
                        nodo->G = distParent + current->G;
                        nodo->F = distTarget + nodo->G;
                        
                        nodosAbiertos.push_back(nodo);
                        nodo->parent = current;
                    } else
                    {
                        if(distParent + current->G < nodo->G)
                        {
                            nodo->G = distParent + current->G;
                            nodo->parent = current;
                        }
                        /*
                         *float dist = _sdistance(nodo->position, targetNode->position);
                         *nodo->F = dist;
                         */
                    }
                }
            }
            
            struct CompararAStarNode
            {
                int operator() (const AStarNode* a, const AStarNode* b) const
                { return a->F > b->F; }
            } f;
            
            std::sort_heap (nodosAbiertos.begin(), nodosAbiertos.end(), f);
            
            ++iter;
        }
        
        if(current == targetNode)
        {
            float mejorSolucion = 0;
            while(current != originNode && current != NULL)
            {
                if(current->parent != NULL)
                    mejorSolucion += _sdistance(current->position, current->parent->position);
                auxiliarPath.push_front(current->position);
                current = current->parent;
            }
            if(mejorSolucion < min)
            {
                min = mejorSolucion;
                path = auxiliarPath;
            }
        }
    }
}
