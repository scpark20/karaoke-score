//
//  ACFPitchDetector.hpp
//  pitch
//
//  Created by Park, Soo-Chul on 21/08/2017.
//  Copyright © 2017 Park, Soo-Chul. All rights reserved.
//

#ifndef ACFPitchDetector_hpp
#define ACFPitchDetector_hpp

#include "PitchDetector.hpp"

class ACFPitchDetector: public PitchDetector {
public:
    ACFPitchDetector(unique_ptr<short[]>& samples, const int sample_length, const int sample_rate, const int frame_size, const int hopSizeInMS);
    ~ACFPitchDetector();
    
    virtual int getPitchArray(unique_ptr<float[]>& pitch_array, unique_ptr<float[]>& rms_array, const int pitchArrayLength, PitchType pitchType);
};

#endif /* ACFPitchDetector_hpp */
