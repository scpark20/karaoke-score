//
//  PrecisionRecallScore.hpp
//  pitch
//
//  Created by Park, Soo-Chul on 23/08/2017.
//  Copyright © 2017 Park, Soo-Chul. All rights reserved.
//

#ifndef PrecisionRecallScore_hpp
#define PrecisionRecallScore_hpp

#include "KaraokeScore.hpp"

class PrecisionRecallScore : public KaraokeScore {
public:
    PrecisionRecallScore(const int sampleRate, const int frameSize, const int hopSizeInMS);

    virtual ~PrecisionRecallScore();

private:
    virtual float getScoreInternal(int partIndex, int pitchTune, int tolerancePitch, int toleranceTimeInMS);

};

#endif /* PrecisionRecallScore_hpp */
