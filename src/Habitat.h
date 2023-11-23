#ifndef HABITAT_H
#define HABITAT_H

#include "utils.h"
#include "vector"

#define SETTINGS_DEFAULT Settings{200, 20, 0.75, 90, 0.2, 2}

struct Individual {
    const SrcImage* img;

    int imgID;
    int x;
    int y;
    float angle;
    float scale;
};

struct PopulationGroup {
    std::vector<Individual> individuals;
    uint8_t* pastedData;
    uint32_t fitness;
};

struct Settings {
    int imgCount;
    int popSize;
    float reroll;
    int crossoverChance;
    float minScale;
    float maxScale;
};

class Habitat
{
    public:
        Habitat(const SrcImage* reconstructionImage, const std::vector<SrcImage>* refImages);
        Habitat(const SrcImage* reconstructionImage, const std::vector<SrcImage>* refImages, Settings settings);
        virtual ~Habitat();

        void step();
        const PopulationGroup& getBestGroup();
    protected:

    private:
        std::vector<PopulationGroup> mPopulation;
        const std::vector<SrcImage>* mRefImages;
        const SrcImage* mReconstructionImage;
        Settings mSettings;

        void init_pop();
        Individual random_individual();
        void crossover(const PopulationGroup& grpA, const PopulationGroup& grpB, PopulationGroup& grpC);
        void drawComputeFit(PopulationGroup& grp);
        void mutate(PopulationGroup& grp);
};

#endif // HABITAT_H
