//
//  AudioDecoder.cpp
//  pitch
//
//  Created by Park, Soo-Chul on 24/08/2017.
//  Copyright © 2017 Park, Soo-Chul. All rights reserved.
//

#include "AudioDecoder.hpp"
#include <stdlib.h>
#include <iostream>

using namespace std;

AudioDecoder::AudioDecoder() {
    this->decoder = make_shared<SuperpoweredDecoder>();
}

AudioDecoder::~AudioDecoder() {

}

int AudioDecoder::open(const char* filename) {
    const char *ret = this->decoder->open(filename, false, 0, 0);
    if(ret != NULL)
        return 0;

    unsigned int neededByteNum = (unsigned int) (sizeof(short) * this->decoder->durationSamples * 2 + 32768);
    
    return neededByteNum;
}

int AudioDecoder::open(const char* filename, int offset, int length) {
    const char *ret = this->decoder->open(filename, false, offset, length);
    if(ret != NULL)
        return 0;

    unsigned int neededByteNum = (unsigned int ) (sizeof(short) * this->decoder->durationSamples + (decoder->samplesPerFrame * 4 + 16384));

    return neededByteNum;
}

int AudioDecoder::decode(unique_ptr<short[]>& decodedSamples) {
    int offset = 0;
    short int samples[decoder->samplesPerFrame * 2 + 32768];
    while(true)
    {
        unsigned int decodedSamplesNum = decoder->samplesPerFrame;
        if(decoder->decode(samples, &decodedSamplesNum) == SUPERPOWEREDDECODER_ERROR)
            break;

        if(decodedSamplesNum < 1)
            break;

        memcpy(&decodedSamples[offset * 2], samples, sizeof(short int) * decodedSamplesNum * 2);
        offset += decodedSamplesNum;
    }
    
    return offset;
}

int AudioDecoder::getSampleRate() {
    return decoder->samplerate;
}
