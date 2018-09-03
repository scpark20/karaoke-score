//
//  ACFPitchDetector.cpp
//  pitch
//
//  Created by Park, Soo-Chul on 21/08/2017.
//  Copyright © 2017 Park, Soo-Chul. All rights reserved.
//

#include "ACFPitchDetector.hpp"
#include <SuperpoweredFFT.h>
#include "PitchUtil.hpp"

#include <android/log.h>
#define LOG_TAG "karaokescore"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


using namespace std;

ACFPitchDetector::ACFPitchDetector(unique_ptr<short[]>& samples, const int sample_length, const int sample_rate, const int frame_size, const int hopSizeInMS)
:PitchDetector(samples, sample_length, sample_rate, frame_size, hopSizeInMS){
    
}

ACFPitchDetector::~ACFPitchDetector() {
    
}

#define THREAD_NUM 8
int ACFPitchDetector::getPitchArray(unique_ptr<float[]>& pitch_array, unique_ptr<float[]>& rms_array, const int pitchArrayLength, PitchType pitchType) {

    float **floats1Array = (float**) malloc(sizeof(float*) * THREAD_NUM);
    float **floats2Array = (float**) malloc(sizeof(float*) * THREAD_NUM);

    for(int i=0;i<THREAD_NUM;i++) {
        floats1Array[i] = (float *) malloc(sizeof(float) * this->sample_length);
        floats2Array[i] = (float *) malloc(sizeof(float) * this->sample_length);
    }

    int arrayLength = MIN(pitchArrayLength, this->getFrameNum());

    #pragma omp parallel for schedule(static) num_threads(THREAD_NUM)
    for(int i=0;i<arrayLength;i++) {

        int tid = omp_get_thread_num();

        float *floats1 = floats1Array[tid];
        float *floats2 = floats2Array[tid];

        /** do FFT **/
        int start_index = i * this->hop_size;
        int end_index;
        if(start_index + this->frame_size > this->sample_length)
            end_index = this->sample_length;
        else
            end_index = start_index + this->frame_size;
        
        memcpy(floats1, &this->samples[i * this->hop_size], sizeof(float) * (end_index - start_index));
        memset(floats2, 0x00, sizeof(float) * this->frame_size);
        SuperpoweredFFTComplex(floats1, floats2, this->log_size, true);
        
        /** get power **/
        /** should be optimized by SIMD operations **/

        PitchUtil::calcPower(floats1, floats2, this->frame_size);

        rms_array[i] = 0;
        for(int j=0;j<this->frame_size;j++)
            rms_array[i] += floats1[j];

        rms_array[i] /= this->frame_size;

        /** do IFFT **/
        
        memset(floats2, 0x00, sizeof(float) * this->frame_size);
        SuperpoweredFFTComplex(floats1, floats2, this->log_size, true);

        // cut values below zero
        for(int j=0;j<this->frame_size/2;j++)
            if(floats1[j] < 0.0)
                floats1[j] = 0.0;
        
        memcpy(floats2, floats1, sizeof(float) * this->frame_size/2);
        
        for(int j=2;j<3;j++)
            for(int k=0;k<this->frame_size/2;k++)
                floats1[k] -= floats2[(int)((float)k/(float)j)];

        // find the max value's index
        int max_index = 0;
        for(int j=1;j<this->frame_size/2;j++)
            if(floats1[j] > floats1[max_index])
                max_index = j;
        
        if(max_index==0)
            pitch_array[i] = 0;
        else
            pitch_array[i] = (float)sample_rate / (float)max_index;
    }

    for(int i=0;i<THREAD_NUM;i++) {
        free(floats1Array[i]);
        free(floats2Array[i]);
    }

    free(floats1Array);
    free(floats2Array);
    
    if(pitchType == Note)
        PitchUtil::frequency2note(pitch_array, pitch_array, pitchArrayLength, true);
    else if(pitchType == FloatNote)
        PitchUtil::frequency2note(pitch_array, pitch_array, pitchArrayLength, false);
    
    return PD_SUCCESS;
}
