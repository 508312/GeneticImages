#ifndef HABITAT_H
#define HABITAT_H

#include "utils.h"
#include "vector"

#define SETTINGS_DEFAULT Settings{16, 30, 0.85, 65, 0.01, 1}

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

struct distToImg {
    int imgId;
    int distance;
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
        void calculateClosest();
        const PopulationGroup& getBestGroup();
    protected:

    private:
        std::vector<PopulationGroup> mPopulation;
        const std::vector<SrcImage>* mRefImages;
        const SrcImage* mReconstructionImage;
        std::vector<std::vector<distToImg>> mClosestImages;
        uint8_t* mRecSobel;
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
