// ###### Config options ################


// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"
#include <iostream>
#include <vector>

using namespace Asedio;

// estructura que expresa la valoracion de una defensa
struct ValoracionDefensa {
    size_t id_;
    unsigned int coste_;
    double valor_;
    ValoracionDefensa(size_t id = 0, unsigned int coste = 0, double valor = 0.0) :
                      id_(id), coste_(coste), valor_(valor) { }
    bool operator <(const ValoracionDefensa& vd) { return coste_ >= vd.coste_; }
    // La sobrecarga del operador < es contradictoria debido a que se deseaba
    // que las defensas peor valoradas estuvieran al final del vector
    // para que asi pudieramos hacer la tsp de menor a mayor valor
};

double darValorDefensa(Defense* d) // funcion para valorar las defensas
{
    // float range, dispersion, damage, attacksPerSecond, health; unsigned int cost, type;
    double valor = 0.0;
    valor += (d->range * 0.3) + (d->damage * 0.2) + (d->dispersion * 0.1);
    valor += (d->attacksPerSecond * 0.1) + (d->health * 0.1) - (d->cost * 0.2);
    
    return valor;
}

void DEF_LIB_EXPORTED selectDefenses(std::list<Defense*> defenses, unsigned int ases,
                                     std::list<int> &selectedIDs , float mapWidth,
                                     float mapHeight, std::list<Object*> obstacles)
{
    std::vector<ValoracionDefensa> valoraciones; // Crea vector de valoraciones
    std::list<Defense*>::iterator it = defenses.begin(); // iterador para recorrer las defensas
    
    selectedIDs.push_back((*it)->id); // Se compra la primera defensa forzosamente
    ases -= (*it)->cost; // Se disminuye la cantidad de ases disponibles para las defensas normales
    while(++it != defenses.end()) // Bucle elegante para recorrer y valorar las defensas
    {
        valoraciones.push_back(
           ValoracionDefensa(
               (*it)->id, (*it)->cost, darValorDefensa((*it)) // Se crea una Valoracion de la defensa
           )
        );
    }
    std::sort(valoraciones.begin(), valoraciones.end()); // Se ordena de mayor a menor valoracion
                                                         // Mediante la sobrecarga del operador <
    std::vector<ValoracionDefensa> valoraciones2 = valoraciones; // Se copian las valoraciones
    
    std::vector< std::vector<double> > tsp(valoraciones.size(), std::vector<double>(ases));
    // Estructura para la tsp
    
    for (int i = 0; tsp.size() && !valoraciones.empty(); i++) // Bucle externo para acceder a las distintas
                                                              // valoraciones
    {
        ValoracionDefensa& vd = *(valoraciones.rbegin()); // Se obtiene la menor valoracion
        
        for (int j = 0; j < tsp[i].size(); j++)
        {
            if(i <= 0) // Condicion para la primera fila
            {
                if(j < vd.coste_) tsp[i][j] = 0;
                else tsp[i][j] = vd.coste_;
            }
            else // Caso general de la tsp
            {
                if(j < vd.coste_) tsp[i][j] = tsp[i - 1][j];
                else
                {
                    if(j == vd.coste_)
                    {
                        if(tsp[i][j-1] < vd.valor_) tsp[i][j] = vd.valor_;
                    }
                    else
                    {
                        double antiguo_valor = tsp[i - 1][j];
                        double nuevo_valor = tsp[i - 1][j-vd.coste_] + vd.valor_;
                        
                        tsp[i][j] = (antiguo_valor < nuevo_valor) ? nuevo_valor : antiguo_valor;
                    }
                }
            }
        }
        valoraciones.pop_back(); // Se elimina la menor valoracion
    }
    
    // Resolución del problema, dando marcha atras (^-^) a la tsp
    unsigned int row = tsp.size() - 1;
    unsigned int col = ases - 1;
    
    while(row > 0 && col > 0) // Bucle para hacer la marcha atras (^-^)
    {
        if(tsp[row][col] != tsp[row - 1][col])
        {
            size_t id = valoraciones2[row].id_;
            selectedIDs.push_back(id);
            col -= valoraciones2[row].coste_;
        }
        row -= 1;
    }
    
    // Comprobación de la primera fila de la tsp
    if(tsp[row][col] != 0)
    {
        size_t id = valoraciones2[row].id_;
        selectedIDs.push_back(id);
    }
}
