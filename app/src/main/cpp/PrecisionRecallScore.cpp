//
//  PrecisionRecallScore.cpp
//  pitch
//
//  Created by Park, Soo-Chul on 23/08/2017.
//  Copyright © 2017 Park, Soo-Chul. All rights reserved.
//

#include "PrecisionRecallScore.hpp"
#include "PitchUtil.hpp"
#include <iostream>
#include <stdlib.h>
#include <cmath>

using namespace std;

/* android NDK libraries */
#include <android/log.h>
#define LOG_TAG "karaokescore"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


PrecisionRecallScore::PrecisionRecallScore(const int sampleRate, const int frameSize, const int hopSizeInMS):
KaraokeScore(sampleRate, frameSize, hopSizeInMS) {

}

PrecisionRecallScore::~PrecisionRecallScore() {
    
}

float PrecisionRecallScore::getScoreInternal(int partIndex, int pitchTune, int tolerancePitch, int toleranceTimeInMS) {

    /* calculate true/false, pos/neg values */
    int true_positive = 0;
    int true_negative = 0;
    int false_positive = 0;
    int false_negative = 0;

    int timeToleranceInHop = (int) ceil(toleranceTimeInMS / this->hopSizeInMS);

    if(partIndex < 0 || partIndex >= ssa->parts.size())
        return 0;

    vector<shared_ptr<Section>> sections = ssa->parts[partIndex]->sections;

    for(int i=0;i<this->pitchArrayLength;i++) {

        bool interested = false;
        for(vector<shared_ptr<Section>>::iterator iter = sections.begin();
                iter != sections.end();
                iter++) {
            shared_ptr<Section> section = *iter;
            int sectionStartInHop = (int) ceil(section->startInCS * 10 / this->hopSizeInMS);
            int sectionEndInHop = (int) ceil(section->endInCS * 10 / this->hopSizeInMS);

            if(i <= sectionEndInHop && i > sectionStartInHop)
                interested = true;
        }

        if(!interested)
            continue;

        int start_index = MAX(i - timeToleranceInHop, 0);
        int end_index = MIN(i + timeToleranceInHop, this->pitchArrayLength);
        
        int localLength = end_index - start_index;
              
        if(localLength <= 0)
            continue;

        if(this->scoreMode == Pitch) {
            float localDiffArray[localLength];

            for(int j=0;j<localLength;j++)
                localDiffArray[j] = abs(voxPitchArray[start_index + j] - refPitchArray[i]);

            if (abs(refPitchArray[i]) < 1.0f) {
                if (PitchUtil::min(localDiffArray, localLength) < tolerancePitch)
                    true_negative += 1;
                else
                    false_positive += 1;
            } else {
                if (PitchUtil::min(localDiffArray, localLength) < tolerancePitch) {

                    true_positive += 1;
                } else {
                    false_negative += 1;
                }
            }

        } else {
            float localRMSArray[localLength];

            for(int j=0;j<localLength;j++)
                localRMSArray[j] = abs(voxRMSArray[start_index + j]);

            if(abs(refRMSArray[i]) < 1.0f) {
                if(PitchUtil::max(localRMSArray, localLength) < 1.0f)
                    true_negative += 1;
                else
                    false_positive += 1;
            } else {
                if(PitchUtil::max(localRMSArray, localLength) < 0.2f)
                    false_negative += 1;
                else
                    true_positive += 1;
            }
        }
    }

    /* calculate precision and recall */
    float precision;
    if(true_positive + false_positive <= 0)
        precision = 0;
    else
        precision = (float)true_positive / ((float)true_positive + (float)false_positive);

    float recall;
    if(true_positive + false_negative <= 0)
        recall = 0;
    else
        recall = (float)true_positive / ((float)true_positive + (float)false_negative);
    
    LOGD("result %d, %d, %d, %d, (%f, %f)", true_positive, true_negative, false_positive, false_negative, precision, recall);
    return recall;
}
