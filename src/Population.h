#ifndef POPULATION_H
#define POPULATION_H

#include "utils.h"
#include "vector"

struct individual {
    image* img;

    int imgID;
    int x;
    int y;
    float angle;
    float scale;
};

struct settings {

};

class Population
{
    public:
        Population();
        virtual ~Population();

        void mutate();

    protected:

    private:
        std::vector<individual> mPopulation;
};

#endif // POPULATION_H
