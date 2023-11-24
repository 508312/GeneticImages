#include "Habitat.h"
#include "rotate.h"
#include <string.h>
#include <algorithm>
#include <random>
#include <math.h>
#include <execution>
#include <iostream>

bool cmp(const PopulationGroup& a, const PopulationGroup& b) {
	return a.fitness < b.fitness;
}

Habitat::Habitat(const SrcImage* reconstructionImage, const std::vector<SrcImage>* refImages)
        : Habitat(reconstructionImage, refImages, SETTINGS_DEFAULT) {
}

Habitat::Habitat(const SrcImage* reconstructionImage, const std::vector<SrcImage>* refImages, Settings settings) {
    mRefImages = refImages;
    mReconstructionImage = reconstructionImage;
    mSettings = settings;
    init_pop();

    /*
    std::vector<int> indexes;
    for(int i=0; i<mSettings.popSize; i++) {
        indexes.push_back(i);
    }

    std::for_each(std::execution::par_unseq, indexes.begin(), indexes.end(), [&](int i) {
        drawComputeFit(mPopulation[i]);
    }); */
}

void Habitat::step() {
    std::sort(mPopulation.begin(), mPopulation.end(), cmp);

    std::vector<int> indexes;
    for(int i=0; i<mSettings.popSize; i++) {
		if(i>(mSettings.popSize - ceil(mSettings.popSize * mSettings.reroll) - 1)) {
            indexes.push_back(i);
        }
    }

    std::for_each(std::execution::par_unseq, indexes.begin(), indexes.end(), [&](int i) {
        if(rand()%100 < mSettings.crossoverChance) {
            int ind1 = rand() % (int)(mSettings.popSize - ceil(mSettings.popSize * mSettings.reroll) - 1);
            int ind2 = rand() % (int)(mSettings.popSize - ceil(mSettings.popSize * mSettings.reroll) - 1);
            crossover(mPopulation[ind1], mPopulation[ind2], mPopulation[i]);
        } else {
            mutate(mPopulation[i]);
        }

        drawComputeFit(mPopulation[i]);
    });
}

const PopulationGroup& Habitat::getBestGroup() {
    return mPopulation[0];
}

void Habitat::init_pop() {
    mPopulation.clear();
    for (int i = 0; i < mSettings.popSize; i++) {
        mPopulation.push_back(PopulationGroup{});
        mPopulation[i].pastedData = new uint8_t[mReconstructionImage->width*mReconstructionImage->height*4];
        mPopulation[i].fitness = UINT32_MAX;
        for (int j = 0; j < mSettings.imgCount; j++) {
            mPopulation[i].individuals.push_back(random_individual());
        }
    }
}

void Habitat::drawComputeFit(PopulationGroup& grp) {
    memset(grp.pastedData, 0xFF, mReconstructionImage->width*mReconstructionImage->height*4);

    RotatePixel_t *pDstBase = static_cast<RotatePixel_t*>((void*)grp.pastedData);

    for (int i = 0; i < grp.individuals.size(); i++) {
        const SrcImage* pImg = grp.individuals[i].img;
        RotatePixel_t *pSrcBase = static_cast<RotatePixel_t*>((void*)pImg->data);
        RotateDrawClip(pDstBase, mReconstructionImage->width, mReconstructionImage->height, mReconstructionImage->pitch,
                       pSrcBase, pImg->width, pImg->height, pImg->pitch,
                       grp.individuals[i].x, grp.individuals[i].y,
                       0, 0,
                       grp.individuals[i].angle, grp.individuals[i].scale);
    }

    grp.fitness = compute_sad(grp.pastedData, mReconstructionImage->data,
                               mReconstructionImage->width*mReconstructionImage->height*4);
}

void Habitat::crossover(const PopulationGroup& grpA, const PopulationGroup& grpB, PopulationGroup& grpC) {
    grpC.individuals.clear();
    for (int i = 0; i < std::min(grpA.individuals.size(),
                                grpB.individuals.size()); i++) {
        if (rand()%2) {
            grpC.individuals.push_back(grpA.individuals[i]);
        } else {
            grpC.individuals.push_back(grpB.individuals[i]);
        }
    }
}

void Habitat::mutate(PopulationGroup& grp) {
    int roll = rand()%100;
    if (roll < 60) {
        mutateAdjust(grp);
    } else if (roll < 85) {
        mutateAdd(grp);
    } else {
        mutateRemove(grp);
    }
}

void Habitat::mutateAdd(PopulationGroup& grp) {
    grp.individuals.push_back(random_individual());
}

void Habitat::mutateRemove(PopulationGroup& grp) {
    if (grp.individuals.size() == 0)
        return;

    int ind = rand()%grp.individuals.size();
    grp.individuals.erase(grp.individuals.begin() + ind);
}

void Habitat::mutateAdjust(PopulationGroup& grp) {
    //for (int i = 0; i < grp.individuals.size(); i++) {
    {
        int i = rand()%grp.individuals.size();
        // ADD CLOSEST IMAGES

        grp.individuals[i].angle += rand()%50 / 100.0 * std::pow(-1, rand()%2);
        grp.individuals[i].x += rand()%(mReconstructionImage->width) * 0.1 * std::pow(-1, rand()%2);
        grp.individuals[i].y += rand()%(mReconstructionImage->height) * 0.1 * std::pow(-1, rand()%2);
        float randScale = (rand()%((int)(1000*mSettings.maxScale - 1000*mSettings.minScale))
                                                + (1000*mSettings.minScale)) / 1000.0;
        grp.individuals[i].scale += randScale * 0.1 * std::pow(-1, rand()%2);
        if (grp.individuals[i].scale < mSettings.minScale) {
            grp.individuals[i].scale = mSettings.minScale;
        } else if (grp.individuals[i].scale > mSettings.maxScale) {
            grp.individuals[i].scale = mSettings.maxScale;
        }
    }
}

Individual Habitat::random_individual() {
    Individual newIndiv;

    SrcImage* img;
    int imgID;
    int x;
    int y;
    float angle;
    float scale;

    newIndiv.imgID = rand()%(mRefImages->size());
    newIndiv.img = &(*mRefImages)[newIndiv.imgID];

    newIndiv.x = rand()%(mReconstructionImage->width);
    newIndiv.y = rand()%(mReconstructionImage->height);
    newIndiv.angle = rand()%(314) / 100.0;
    newIndiv.scale = (rand()%((int)(1000*mSettings.maxScale - 1000*mSettings.minScale))
                                                + (1000*mSettings.minScale)) / 1000.0;
    //newIndiv.scale = mSettings.minScale;

    return newIndiv;

}

Habitat::~Habitat() {

}
