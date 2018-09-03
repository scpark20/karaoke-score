//
//  PitchDetector.hpp
//  pitch
//
//  Created by Park, Soo-Chul on 21/08/2017.
//  Copyright © 2017 Park, Soo-Chul. All rights reserved.
//

#ifndef PitchDetector_hpp
#define PitchDetector_hpp

#include <SuperpoweredSimple.h>
#include <stdlib.h>
#include <math.h>
#include <memory>


#define PD_SUCCESS 0
#define PD_ERROR_WRONG_PITCH_ARRAY_NUM 1

using namespace std;

enum PitchType {Frequency, Note, FloatNote, RMS};

class PitchDetector {
public:
    
    PitchDetector(unique_ptr<short[]>& samples, const int sample_length, const int sample_rate, const int frame_size, const int hopSizeInMS)
    :sample_length(sample_length), sample_rate(sample_rate), frame_size(frame_size),
     hop_size((int) (sample_rate / 1000.0f * hopSizeInMS)) {
        switch (frame_size) {
            case 128:
                log_size = 7; break;
            case 256:
                log_size = 8; break;
            case 512:
                log_size = 9; break;
            case 1024:
                log_size = 10; break;
            case 2048:
                log_size = 11; break;
            case 4096:
                log_size = 12; break;
            case 8192:
                log_size = 13; break;
            default:
                log_size = 0; break;
        }

        float *stereoSamples = new float[sample_length * 2];
        this->samples = make_unique<float[]>(sample_length);

        SuperpoweredShortIntToFloat(samples.get(), stereoSamples, sample_length * 2, 1);
        SuperpoweredStereoToMono(stereoSamples, this->samples.get(), 1.0f, 1.0f, 1.0f, 1.0f, sample_length);

        delete[] stereoSamples;
    }
    
    virtual ~PitchDetector() {
        //delete this->samples;
    }
    
    int getFrameNum() {
        return (int)(ceil(sample_length / hop_size));
    }
    
    //virtual int getPitchArray(float* pitch_array, float* rms_array, const int pitchArrayLength, PitchType pitchType) = 0;
    virtual int getPitchArray(unique_ptr<float[]>& pitch_array, unique_ptr<float[]>& rms_array, const int pitchArrayLength, PitchType pitchType) = 0;
    virtual int getSampleLength() {return sample_length;}
    
protected:
    unique_ptr<float[]> samples;
    const int sample_length;
    const int sample_rate;
    const int frame_size;
    const int hop_size;
    int log_size;
};

#endif /* PitchDetector_hpp */
