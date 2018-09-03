//
//  KaraokeScore.hpp
//  pitch
//
//  Created by Park, Soo-Chul on 23/08/2017.
//  Copyright © 2017 Park, Soo-Chul. All rights reserved.
//

#ifndef KaraokeScore_hpp
#define KaraokeScore_hpp

#define STATE_RELEASED 0
#define STATE_INITED 1

#define MIDIFILE_READ_ERROR -1
#define NO_ERROR 0

#include <string.h>
#include <SSA.hpp>

enum ScoreMode {Pitch, Power};

class KaraokeScore {
public:

    /* Constructor
     * param1: sampleRate - sampling rate to decode/analysis audio file.
     * param2: frameSize - FFT analysis frame size, 1024 or 2048 recommended.
     * param3: hopSizeInMS - FFT analysis hop size in milliseconds, 20 - 80 recommended. */
    KaraokeScore(const int sampleRate, const int frameSize, const int hopSizeInMS);

    virtual ~KaraokeScore();

    /* Read reference midi file and user voice audio file. then analysis pitch data
     * param1 : refSSAFilePath - reference .SSA file's path
     * param2 : refMidiFilePaths - reference midi file's paths
     * param3 : refMidiFilePathsLength - length of reference midi file's paths
     * param4 : voxFilePath - user voice audio file's path */
    virtual int readFiles(const char* refSSAFilePath, const char* refMidiFilePaths[], int refMidiFilePathsLength, const char* voxFilePath);

    /* Return pitch data array's length. It is used to allocate pitch data array.
     * no params */
    virtual int getPitchArrayLength();

    /* Fill the float arraies with analysized pitch data.
     * param1 : refPitchArray - reference pitch array.
     * param2 : refLength - reference pitch array's length. it should be determined by return value of getPitchArrayLength() method.
     * param3 : voxPitchArray - voice pitch array.
     * param4 : voxLength - user voice pitch array's length. it should be determined by return value of getPitchArrayLength() method. */
    virtual void getPitchArraies(float *refPitchArray, int refLength, float *voxPitchArray, int voxLength);

    /* Return the karaoke score.
     * param1 = partIndex - the part index to be caculated.
     * param1 : min - minimum score.
     * param2 : max - maximum score.
     * param3 : random - random score.
     * param4 : pitchTune - reference audio data's pitch difference in MIDI note value.
     * param5 : tolerancePitch
     * param6 : toleranceTimeInMS */
    virtual float getScore(int partIndex, float min, float max, float random, int pitchTune, int tolerancePitch, int toleranceTimeInMS);

    virtual int getAudioBufferLength() { return voxSampleNum; }

    virtual void getAudioBuffer(short* audioBuffer) {
        memcpy(audioBuffer, voxSamples.get(), sizeof(short) * voxSampleNum);
    }

    virtual int getPartCount() {
        return ssa->parts.size();
    }

protected:

    //should be implements in sub class
    virtual float getScoreInternal(int partIndex, int pitchTune, int tolerancePitch, int toleranceTimeInMS) = 0;

    int state;
    ScoreMode scoreMode = Pitch;
    const int frameSize;
    const int hopSizeInMS;

    unique_ptr<SSA> ssa;

    unique_ptr<short[]> voxSamples;
    int voxSampleNum;

    unique_ptr<float[]> refPitchArray;
    unique_ptr<float[]> refRMSArray;
    unique_ptr<float[]> voxPitchArray;
    unique_ptr<float[]> voxRMSArray;

    int pitchArrayLength;

    int sampleRate;

private:
    int setRefRMSArray(unique_ptr<SSA>& ssa, int hopSizeInMS, int pitchArrayLength, unique_ptr<float[]>& refRMSArray);

    int readRefMidiFiles(const char* refMidiFilePath, int pitchArrayLength,
                                       unique_ptr<float[]>& refPitchArray, unique_ptr<float[]>& refRMSArray);
};
#endif /* KaraokeScore_hpp */
