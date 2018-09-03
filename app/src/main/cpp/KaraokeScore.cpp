
// Created by Park, Soo-Chul on 30/08/2017.
//

/* standard libraries */
#include <iostream>
#include <stdlib.h>
#include <list>

using namespace std;

/* android NDK libraries */
#include <android/log.h>
#include <vector>
#include <memory>
#include <array>

#define LOG_TAG "karaokescore"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

/* Pitch libraries */
#include "PrecisionRecallScore.hpp"
#include "AudioDecoder.hpp"
#include "CepstrumPitchDetector.hpp"
#include "ACFPitchDetector.hpp"
#include "PitchUtil.hpp"

/* MidiFile Library */
#include "MidiFile.h"
#include "NoteItem.h"

/* SSA Library */
#include <SSA.hpp>

/* own header */
#include "KaraokeScore.hpp"

KaraokeScore::KaraokeScore(const int sampleRate, const int frameSize, const int hopSizeInMS) :
        sampleRate(sampleRate), frameSize(frameSize), hopSizeInMS(hopSizeInMS) {


}

KaraokeScore::~KaraokeScore() {

    //delete refPitchArray;
    //delete voxPitchArray;

    this->state = STATE_RELEASED;

}

int KaraokeScore::readFiles(const char* refSSAFilePath, const char* refMidiFilePaths[],
                             int refMidiFilePathsLength, const char* voxFilePath) {

    state = STATE_RELEASED;

    // Possible Case 1 : refSSA - exist, refMidi - null, voxFile - exist : hip-hop rap song
    // Possible Case 2 : refSSA - exist, refMidi - exist, voxFile - exist : normal melody song

    /****************************/
    /* Read Reference SSA File  */
    /****************************/

    ssa = make_unique<SSA>();
    float durationInSeconds;
    if(refSSAFilePath!= nullptr) {
        int ret = ssa->read(refSSAFilePath);
        if(ret != SSA_READ_SUCCESS) {
            return state;
        }

        durationInSeconds = ssa->getDurationInSeconds();
    } else {
        return state;
    }

    //Allocate analysis arraies
    this->pitchArrayLength = (int) ceil(durationInSeconds * 1000 / hopSizeInMS);
    this->refPitchArray = make_unique<float[]>(this->pitchArrayLength);
    this->refRMSArray = make_unique<float[]>(this->pitchArrayLength);
    this->voxPitchArray = make_unique<float[]>(this->pitchArrayLength);
    this->voxRMSArray = make_unique<float[]>(this->pitchArrayLength);

    setRefRMSArray(this->ssa, this->hopSizeInMS, this->pitchArrayLength, this->refRMSArray);

    /****************************/
    /* Read Reference Midi File */
    /****************************/

    this->scoreMode = Pitch;
    if(refMidiFilePaths != nullptr) {
        for(int i=0;i<refMidiFilePathsLength;i++) {
            if(this->readRefMidiFiles(refMidiFilePaths[i], this->pitchArrayLength, this->refPitchArray, this->refRMSArray) == NO_ERROR) {
                ; //pass
            } else {
                this->scoreMode = Power;
                break;
            }
        }
    } else {
        this->scoreMode = Power;
    }

    /************************/
    /* decode voice samples */
    /************************/

    if(voxFilePath!=nullptr) {
        AudioDecoder *audioDecoder = new AudioDecoder();
        int neededByteNum = audioDecoder->open(voxFilePath);

        // Return if there is no data to decode
        if (neededByteNum <= 0) {
            //delete[] refPitchArray;
            delete audioDecoder;
            return state;
        }

        voxSamples = make_unique<short[]>(neededByteNum/2); // new short int[neededByteNum/2];
        voxSampleNum = audioDecoder->decode(voxSamples);
        int voxSampleRate = audioDecoder->getSampleRate();

        delete audioDecoder;

        /************************/
        /* pitch detect : voice */
        /************************/
        //svector<shared_ptr<Section>> sections = this->ssa.partsp[partIndex]->sections;

        PitchDetector *pd = new ACFPitchDetector(voxSamples, voxSampleNum, voxSampleRate, frameSize, hopSizeInMS);
        //voxPitchArray = new float[this->pitchArrayLength];
        int ret = pd->getPitchArray(voxPitchArray, voxRMSArray, this->pitchArrayLength, FloatNote);
        delete pd;
        //delete voxSamples;

        if (ret != PD_SUCCESS) {
            //delete[] refPitchArray;
            //delete[] voxPitchArray;
            return state;
        }
    }

    state = STATE_INITED;
    return state;
}

int KaraokeScore::setRefRMSArray(unique_ptr<SSA>& ssa, int hopSizeInMS, int pitchArrayLength, unique_ptr<float[]>& refRMSArray) {

    vector<shared_ptr<Part>> parts = ssa->parts;
    for(vector<shared_ptr<Part>>::iterator iter = parts.begin();
        iter != parts.end();
        iter++) {
        shared_ptr<Part> part = *iter;
        for(vector<shared_ptr<Section>>::iterator iter2 = part->sections.begin();
            iter2 != part->sections.end();
            iter2++) {
            shared_ptr<Section> section = *iter2;
            int from_index = (int) section->startInCS * 10 / hopSizeInMS;
            int to_index = (int) section->endInCS * 10 / hopSizeInMS;
            from_index = MAX(from_index, 0);
            to_index = MIN(to_index, pitchArrayLength);

            if (to_index >= from_index) {
                for(int index=from_index;index<to_index;index++) {
                    refRMSArray[index] = 1.0f;
                }
            }
        }
    }

    return 0;
}

int KaraokeScore::readRefMidiFiles(const char* refMidiFilePath, int pitchArrayLength,
                                   unique_ptr<float[]>& refPitchArray, unique_ptr<float[]>& refRMSArray) {

    LOGD("readRefMidiFiles %s", refMidiFilePath);
    MidiFile midiFile;
    int ret = midiFile.read(refMidiFilePath);

    if(!ret)
        return MIDIFILE_READ_ERROR;

    //Merge all tracks into one track
    midiFile.joinTracks();

    MidiEvent *event;
    NoteItem *currentNote = nullptr;
    std::list<NoteItem*> noteItemList;

    for (int eventID = 0;eventID < midiFile[0].size(); eventID++) {

        double currentTime = midiFile.getTimeInSeconds(0, eventID);
        event = &midiFile[0][eventID];

        if(event->isNoteOn()) {

            // A note is started while previous note is not ended
            if(currentNote != nullptr) {
                currentNote->end = currentTime;
                noteItemList.push_back(currentNote);
            }

            currentNote = new NoteItem(currentTime, 0, event->getKeyNumber());
        }
        else if(event->isNoteOff()) {

            // There is no started note
            if(currentNote == nullptr) {
                continue;
            }

            // If started note's pitch is different to event note's
            if((int)currentNote->note != event->getKeyNumber()) {
                continue;
            }

            currentNote->end = currentTime;
            noteItemList.push_back(currentNote);
            currentNote = nullptr;
        }
    }

    for(std::list<NoteItem*>::iterator noteIterator = noteItemList.begin();
        noteIterator != noteItemList.end();
        noteIterator++) {
        NoteItem *currentNote = *noteIterator;
        int from_index = (int) floor(currentNote->start * 1000 / hopSizeInMS);
        int to_index = (int) floor(currentNote->end * 1000 / hopSizeInMS);
        from_index = MAX(from_index, 0);
        to_index = MIN(to_index, pitchArrayLength);

        if(to_index >= from_index) {
            for(int index=from_index;index<to_index;index++)
                refPitchArray[index] = (int) currentNote->note;
        }

        delete currentNote;
    }

    return NO_ERROR;
}

int KaraokeScore::getPitchArrayLength() {
    if (state == STATE_RELEASED) {
        return -1;
    } else {
        if (refPitchArray == nullptr || voxPitchArray == nullptr)
            return -1;
        else
            return pitchArrayLength;
    }
}

void KaraokeScore::getPitchArraies(float *refPitchArray, int refLength, float *voxPitchArray, int voxLength) {
    if (state == STATE_RELEASED)
        return;

    if (refLength != this->pitchArrayLength || voxLength != this->pitchArrayLength)
        return;

    if (this->refPitchArray == nullptr || this->voxPitchArray == nullptr)
        return;

    memcpy(refPitchArray, this->refPitchArray.get(), sizeof(float) * pitchArrayLength);
    memcpy(voxPitchArray, this->voxPitchArray.get(), sizeof(float) * pitchArrayLength);

    return;
}

float KaraokeScore::getScore(int partIndex, float min, float max, float random, int pitchTune, int tolerancePitch, int toleranceTimeInMS) {
    float rawScore;
    if (state == STATE_RELEASED)
        rawScore = 0;
    else
        rawScore = this->getScoreInternal(partIndex, pitchTune, tolerancePitch, toleranceTimeInMS);

    float randomScore = ((double) rand() / RAND_MAX - 0.5) * 2.0 * random;

    float score = min + (max - min) * rawScore + randomScore;
    if (score < min)
        score = min;
    else if (score > max)
        score = max;

    return score;
}