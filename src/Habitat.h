#ifndef HABITAT_H
#define HABITAT_H

#include "utils.h"
#include "vector"

#define SETTINGS_DEFAULT Settings{12, 30, 0.7, 70, 0.001, 2}

struct Individual {
    const SrcImage* img;

    int imgID;
    float xp; //xpercent
    float yp; //ypercent
    float angle;
    float scale;
};

struct PopulationGroup {
    std::vector<Individual> individuals;
    uint8_t* pastedData;
    uint64_t fitness;
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

        // Todo: are pointers inside invididual worth this function?
        void reload_indiv_pointers();
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
        void mutateAdjust(PopulationGroup& grp);
        void mutateAdd(PopulationGroup& grp);
        void mutateRemove(PopulationGroup& grp);
};

#endif // HABITAT_H
