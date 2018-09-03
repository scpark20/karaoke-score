//
//  PitchUtil.cpp
//  pitch
//
//  Created by Park, Soo-Chul on 23/08/2017.
//  Copyright © 2017 Park, Soo-Chul. All rights reserved.
//

#include "PitchUtil.hpp"
#include <memory>

/* android NDK libraries */
#include <android/log.h>
#define LOG_TAG "karaokescore"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace std;

void PitchUtil::frequency2note(unique_ptr<float[]>& frequencies,unique_ptr<float[]>& notes, int array_length, bool round) {
    if (round) {
        #pragma omp parallel for
        for (int i = 0; i < array_length; i++)
            notes[i] = roundf(frequency2note(frequencies[i]));
    } else {
        #pragma omp parallel for
        for (int i = 0; i < array_length; i++)
            notes[i] = frequency2note(frequencies[i]);
    }
    return;
}

float PitchUtil::frequency2note(float frequency) {
    if (frequency <= 0.0)
        return 0;

    float a = log2f(440.0f / 32.0f) - log2f(frequency);
    return -12.0f * a + 9.0f;
}

float PitchUtil::max(float *values, int valuesLength) {
    if (valuesLength == 0)
        return 0;

    float maxValue = values[0];
    for (int i = 1; i < valuesLength; i++)
        if (values[i] > maxValue)
            maxValue = values[i];

    return maxValue;
}

float PitchUtil::min(float *values, int valuesLength) {
    if (valuesLength == 0)
        return 0;

    float minValue = values[0];
    for (int i = 1; i < valuesLength; i++)
        if (values[i] < minValue)
            minValue = values[i];

    return minValue;
}

void PitchUtil::multiplyAndSave(float *dest, float *src, float *kernel, float length) {
#if defined(HAVE_NEON)
    for (int j = 0; j < length; j += 4) {
        float32x4_t src_v = vld1q_f32(&src[j]);
        float32x4_t kernel_v = vld1q_f32(&kernel[j]);
        src_v = vmulq_f32(src_v, kernel_v);

        vst1q_f32(&dest[j], src_v);
    }
#else
    for(int j=0;j<length;j++)
        dest[j] = src[j] * kernel[j];
#endif
}

void PitchUtil::calcLogPower(float *real, float *imag, int frame_size) {

#if defined(HAVE_NEON)
    const static float bias[] = {1.0f, 1.0f, 1.0f, 1.0f};
    const static float32x4_t bias_v = vld1q_f32(bias);

    for (int j = 0; j < frame_size; j += 4) {
        float32x4_t real_v = vld1q_f32(&real[j]);
        float32x4_t imag_v = vld1q_f32(&imag[j]);


        real_v = vmulq_f32(real_v, real_v);
        real_v = vmlaq_f32(real_v, imag_v, imag_v);
        real_v = vaddq_f32(real_v, bias_v);

        vst1q_f32(&real[j], real_v);
    }
    for(int j=0;j<frame_size;j++) {
            real[j] = logf(real[j]);
        }
#else
    for(int j=0;j<frame_size;j++) {
            real[j] = real[j] * real[j] + imag[j] * imag[j] + 1;
            real[j] = logf(real[j]);
        }
#endif
}

void PitchUtil::calcPower(float *real, float *imag, int frame_size) {

#if defined(HAVE_NEON)
    for (int j = 0; j < frame_size; j += 4) {
        float32x4_t real_v = vld1q_f32(&real[j]);
        float32x4_t imag_v = vld1q_f32(&imag[j]);


        real_v = vmulq_f32(real_v, real_v);
        real_v = vmlaq_f32(real_v, imag_v, imag_v);
        real_v = vrsqrteq_f32(real_v);
        real_v = vrecpeq_f32(real_v);

        vst1q_f32(&real[j], real_v);
    }
#else
    for(int j=0;j<frame_size;j++) {
            real[j] = real[j] * real[j] + imag[j] * imag[j];
            real[j] = sqrt(real[j]);
        }
#endif
}