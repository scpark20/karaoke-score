//
//  PitchUtil.hpp
//  pitch
//
//  Created by Park, Soo-Chul on 23/08/2017.
//  Copyright © 2017 Park, Soo-Chul. All rights reserved.
//

#ifndef PitchUtil_hpp
#define PitchUtil_hpp

#if defined(HAVE_NEON)
    #include <arm_neon.h>
#endif

#include <math.h>
#include <iostream>
#include <omp.h>
#include <memory>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

using namespace std;

class PitchUtil {
public:
    static void frequency2note(unique_ptr<float[]>& frequencies,unique_ptr<float[]>& notes, int array_length, bool round);
    
    static inline float frequency2note(float frequency);
    
    static float max(float *values, int valuesLength);
    
    static float min(float *values, int valuesLength);

    /* dest[j] = src[j] * kernel[j] for all j in (0, length) */
    static void multiplyAndSave(float *dest, float *src, float *kernel, float length);

    /* real[j] = log(real[j] * real[j] + imag[j] * imag[j]) */
    static void calcLogPower(float *real, float *imag, int frame_size);

    /* real[j] = pow(real[j] * real[j] + imag[j] * imag[j], 1.0f/3.0f) */
    static void calcPower(float *real, float *imag, int frame_size);
};

#endif /* PitchUtil_hpp */
