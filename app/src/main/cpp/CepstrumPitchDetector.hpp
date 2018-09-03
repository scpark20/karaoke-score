//
// Created by Park, Soo-Chul on 30/08/2017.
//

#ifndef KARAOKESCORE_CEPSTRUMPITCHDETECTOR_H
#define KARAOKESCORE_CEPSTRUMPITCHDETECTOR_H

#include "PitchDetector.hpp"

class CepstrumPitchDetector: public PitchDetector {
public:
    CepstrumPitchDetector(short* samples, const int sample_length, const int sample_rate, const int frame_size, const int overlap_num);
    ~CepstrumPitchDetector();

    virtual int getPitchArray(float* pitch_array, const int pitchArrayLength, PitchType pitchType);

private:
};


#endif //KARAOKESCORE_CEPSTRUMPITCHDETECTOR_H
