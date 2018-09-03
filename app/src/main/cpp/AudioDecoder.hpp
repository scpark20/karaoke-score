//
//  AudioDecoder.hpp
//  pitch
//
//  Created by Park, Soo-Chul on 24/08/2017.
//  Copyright © 2017 Park, Soo-Chul. All rights reserved.
//

#ifndef AudioDecoder_hpp
#define AudioDecoder_hpp

#include <SuperpoweredDecoder.h>
#include <memory>

using namespace std;

#define DECODE_SUCCESS 0
#define DECODE_FAIL 1

class AudioDecoder {
public:
    AudioDecoder();
    ~AudioDecoder();
    
    int open(const char* filename);
    int open(const char* filename, int offset, int length);
    int decode(unique_ptr<short[]>& decodedSamples);
    int getSampleRate();
    
private:
    shared_ptr<SuperpoweredDecoder> decoder;
};
#endif /* AudioDecoder_hpp */
