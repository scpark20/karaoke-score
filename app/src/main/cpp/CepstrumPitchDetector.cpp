//
// Created by Park, Soo-Chul on 30/08/2017.
//

#include "CepstrumPitchDetector.hpp"
#include <SuperpoweredFFT.h>
#include <string.h>
#include <math.h>
#include "PitchUtil.hpp"
#include <time.h>

/* android NDK libraries */
#include <android/log.h>
#define LOG_TAG "karaokescore"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

CepstrumPitchDetector::CepstrumPitchDetector(short *samples, const int sample_length, const int sample_rate, const int frame_size,
                                   const int hopSizeInMS)
        : PitchDetector(samples, sample_length, sample_rate, frame_size, hopSizeInMS) {

}

CepstrumPitchDetector::~CepstrumPitchDetector() {

}

int CepstrumPitchDetector::getPitchArray(float* pitch_array, const int pitchArrayLength, PitchType pitchType) {
    float hanning[this->frame_size];
    for (int i=0;i<this->frame_size;i++)
        hanning[i] = 0.5 * (1.0 - cos(2 * M_PI * i / (this->frame_size - 1)));

    int arrayLength = MIN(pitchArrayLength, this->getFrameNum());

    //#pragma omp parallel for schedule(static) num_threads(4)
    for (int i = 0; i < arrayLength; i++) {

        float floats1[this->frame_size];
        float floats2[this->frame_size];

        /** do FFT **/
        int start_index = i * this->hop_size;
        int end_index;
        if (start_index + this->frame_size > this->sample_length)
            end_index = this->sample_length;
        else
            end_index = start_index + this->frame_size;

        PitchUtil::multiplyAndSave(floats1, &this->samples[i * this->hop_size], hanning, this->frame_size);
        memset(floats2, 0x00, sizeof(float) * this->frame_size);
        SuperpoweredFFTComplex(floats1, floats2, this->log_size, true);

        /** get power **/
        PitchUtil::calcLogPower(floats1, floats2, this->frame_size);

        /** do IFFT **/
        memset(floats2, 0x00, sizeof(float) * this->frame_size);
        SuperpoweredFFTComplex(floats1, floats2, this->log_size, false);

//        for (int j=0;j<this->frame_size;j++)
//            floats1[j] /= this->frame_size;

        // cut values below zero
        for (int j = 0; j < this->frame_size / 2; j++)
            if (floats1[j] < 0.0)
                floats1[j] = 0.0;
//            else
//                floats1[j] /= this->frame_size;

        memcpy(floats2, floats1, sizeof(float) * (this->frame_size / 2));

        for (int j = 2; j < 4; j++)
            for (int k = 0; k < this->frame_size / 2; k++)
                floats1[k] -= floats2[(int) ((float) k / (float) j)];

        for (int j = 0; j < 10; j++)
            floats1[j] = 0;

        // find the max value's index
        int max_index = 0;

        for (int j = 0; j < this->frame_size / 2; j++)
            if (floats1[j] >= floats1[max_index])
                max_index = j;

        if (max_index == 0)
            pitch_array = 0;
        else
            pitch_array[i] = (float) sample_rate / (float) max_index;
    }

    if (pitchType == Note)
        PitchUtil::frequency2note(pitch_array, pitch_array, pitchArrayLength, true);
    else if (pitchType == FloatNote)
        PitchUtil::frequency2note(pitch_array, pitch_array, pitchArrayLength, false);

    return PD_SUCCESS;
}

