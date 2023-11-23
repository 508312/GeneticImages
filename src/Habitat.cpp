#include "Habitat.h"
#include "rotate.h"
#include <string.h>
#include <algorithm>
#include <random>
#include <math.h>

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
}

void Habitat::step() {
    std::sort(mPopulation.begin(), mPopulation.end(), cmp);

	for(int i=0; i<mSettings.popSize; i++) {
		if(i>mSettings.popSize - ceil(mSettings.popSize * mSettings.reroll)) {
			if(rand()%100 < mSettings.crossoverChance) {
				int ind1 = rand() % mSettings.popSize;
				int ind2 = rand() % mSettings.popSize;
				crossover(mPopulation[ind1], mPopulation[ind2], mPopulation[i]);
			}
			else {
                mutate(mPopulation[i]);
			}

            drawComputeFit(mPopulation[i]);
		}
	}
}

const PopulationGroup& Habitat::getBestGroup() {
    return mPopulation[0];
}

void Habitat::init_pop() {
    mPopulation.clear();
    for (int i = 0; i < mSettings.popSize; i++) {
        mPopulation.push_back(PopulationGroup{});
        mPopulation[i].pastedData = new uint8_t[mReconstructionImage->pitch*mReconstructionImage->height];
        mPopulation[i].fitness = UINT32_MAX;
        for (int j = 0; j < mSettings.imgCount; j++) {
            mPopulation[i].individuals.push_back(random_individual());
        }
    }
}

void Habitat::drawComputeFit(PopulationGroup& grp) {
    memset(grp.pastedData, 0xFFFFFF00, mReconstructionImage->width*mReconstructionImage->height);

    RotatePixel_t *pDstBase = static_cast<RotatePixel_t*>((void*)grp.pastedData);

    for (int i = 0; i < mSettings.imgCount; i++) {
        const SrcImage* pImg = grp.individuals[i].img;
        RotatePixel_t *pSrcBase = static_cast<RotatePixel_t*>((void*)pImg->data);
        RotateDrawClip(pDstBase, mReconstructionImage->width, mReconstructionImage->height, mReconstructionImage->pitch,
                       pSrcBase, pImg->width, pImg->height, pImg->pitch,
                       grp.individuals[i].x, grp.individuals[i].y,
                       0, 0,
                       grp.individuals[i].angle, grp.individuals[i].scale);
    }

    grp.fitness = compute_sad(grp.pastedData, mReconstructionImage->data,
                               mReconstructionImage->pitch*mReconstructionImage->height);
}

void Habitat::crossover(const PopulationGroup& grpA, const PopulationGroup& grpB, PopulationGroup& grpC) {
    for (int i = 0; i < mSettings.imgCount; i++) {
        if (rand()%2) {
            grpC.individuals[i] = grpA.individuals[i];
        } else {
            grpC.individuals[i] = grpB.individuals[i];
        }
    }
}

void Habitat::mutate(PopulationGroup& grp) {
    for (int i = 0; i < mSettings.imgCount; i++) {
        grp.individuals[i].angle += rand()%50 / 100.0;
        grp.individuals[i].x += rand()%(mReconstructionImage->width) * 0.1 * std::pow(-1, rand()%2);
        grp.individuals[i].y += rand()%(mReconstructionImage->height) * 0.1 * std::pow(-1, rand()%2);
        float randScale = (rand()%((int)(100*mSettings.maxScale - 100*mSettings.minScale))
                                                + (100*mSettings.minScale)) / 100.0;
        grp.individuals[i].scale += randScale * 0.3 * std::pow(-1, rand()%2);
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

    newIndiv.imgID = rand()%mRefImages->size();
    newIndiv.img = &(*mRefImages)[newIndiv.imgID];

    newIndiv.x = rand()%(mReconstructionImage->width - 10) + 5;
    newIndiv.y = rand()%(mReconstructionImage->height - 10) + 5;
    newIndiv.angle = rand()%(314) / 100.0;
    newIndiv.scale = (rand()%((int)(100*mSettings.maxScale - 100*mSettings.minScale))
                                                + (100*mSettings.minScale)) / 100.0;

    return newIndiv;

}

Habitat::~Habitat() {

}
