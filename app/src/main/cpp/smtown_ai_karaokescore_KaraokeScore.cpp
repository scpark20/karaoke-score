//
// Created by Park, Soo-Chul on 30/08/2017.
//

#include "smtown_ai_karaokescore_KaraokeScore.h"
#include <stdlib.h>
#include "KaraokeScore.hpp"
#include "PrecisionRecallScore.hpp"

#include <pthread.h>

static const int instances_num = 100;
KaraokeScore **karaokeScores = nullptr;

/*
 * Class:     smtown_ai_karaokescore_KaraokeScore
 * Method:    getScoreInstanceID
 * Signature: (III)I
 */
JNIEXPORT jint JNICALL Java_smtown_ai_karaokescore_KaraokeScore_getScoreInstanceID
        (JNIEnv *env, jobject obj, jint sampleRate, jint frameSize, jint hopSizeInMS) {
    if (karaokeScores == nullptr) {
        karaokeScores = (KaraokeScore **) malloc(sizeof(KaraokeScore*) * instances_num);
        for(int i=0;i<instances_num;i++)
            karaokeScores[i] = nullptr;
    }

    for (int i = 0; i < instances_num; i++) {
        if (karaokeScores[i] == nullptr) {
            karaokeScores[i] = new PrecisionRecallScore(sampleRate, frameSize, hopSizeInMS);

            //allocation success
            return i;
        }
    }

    //allocation failed
    return -1;
}

/*
 * Class:     smtown_ai_karaokescore_KaraokeScore
 * Method:    readFilesNative
 * Signature: (ILjava/lang/String;[Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_smtown_ai_karaokescore_KaraokeScore_readFilesNative
        (JNIEnv *env, jobject obj, jint scoreInstanceID, jstring refSSAFilePath, jobjectArray refMidiFilePaths, jstring voxFilePath) {

    if (scoreInstanceID < 0 || scoreInstanceID >= instances_num)
        return -1;

    KaraokeScore *score = karaokeScores[scoreInstanceID];
    if (score != nullptr) {
        const char *refSSAFilePathChars = nullptr;
        const char *voxFilePathChars = nullptr;

        jsize refMidiFileCount = env->GetArrayLength(refMidiFilePaths);
        jstring refFilePathsStrings[refMidiFileCount];
        const char *refFilePathsChars[refMidiFileCount];

        for(int i=0;i<refMidiFileCount;i++) {
            refFilePathsStrings[i] = (jstring) env->GetObjectArrayElement(refMidiFilePaths, i);
            if(refFilePathsStrings[i] != nullptr)
                refFilePathsChars[i] = env->GetStringUTFChars(refFilePathsStrings[i], 0);
            else
                refFilePathsChars[i] = nullptr;
        }

        if(refSSAFilePath != nullptr)
            refSSAFilePathChars = env->GetStringUTFChars(refSSAFilePath, 0);

        if(voxFilePath != nullptr)
            voxFilePathChars = env->GetStringUTFChars(voxFilePath, 0);

        int result = score->readFiles(refSSAFilePathChars, refFilePathsChars, refMidiFileCount, voxFilePathChars);

        if(refSSAFilePath != nullptr)
            env->ReleaseStringUTFChars(refSSAFilePath, refSSAFilePathChars);

        if(voxFilePath != nullptr)
            env->ReleaseStringUTFChars(voxFilePath, voxFilePathChars);

        for(int i=0;i<refMidiFileCount;i++) {
            if(refFilePathsStrings[i]!= nullptr)
                env->ReleaseStringUTFChars(refFilePathsStrings[i], refFilePathsChars[i]);
        }

        return result;
    } else {
        return -1;
    }
}

/*
 * Class:     smtown_ai_karaokescore_KaraokeScore
 * Method:    getPitchArrayLengthNative
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_smtown_ai_karaokescore_KaraokeScore_getPitchArrayLengthNative
        (JNIEnv *env, jobject obj, jint scoreInstanceID) {

    if (scoreInstanceID < 0 || scoreInstanceID >= instances_num)
        return -1;

    KaraokeScore *score = karaokeScores[scoreInstanceID];
    if (score != nullptr) {
        return score->getPitchArrayLength();
    } else {
        return -1;
    }
}

/*
 * Class:     smtown_ai_karaokescore_KaraokeScore
 * Method:    getPitchArraiesNative
 * Signature: (I[F[F)V
 */
JNIEXPORT void JNICALL Java_smtown_ai_karaokescore_KaraokeScore_getPitchArraiesNative
        (JNIEnv *env, jobject obj, jint scoreInstanceID, jfloatArray refPitchArray, jfloatArray voxPitchArray) {
    if (scoreInstanceID < 0 || scoreInstanceID >= instances_num)
        return;

    KaraokeScore *score = karaokeScores[scoreInstanceID];
    if (score != nullptr) {
        jfloat *refFloats = env->GetFloatArrayElements(refPitchArray, NULL);
        int refLength = env->GetArrayLength(refPitchArray);

        jfloat *voxFloats = env->GetFloatArrayElements(voxPitchArray, NULL);
        int voxLength = env->GetArrayLength(voxPitchArray);

        score->getPitchArraies(refFloats, refLength, voxFloats, voxLength);

        env->ReleaseFloatArrayElements(refPitchArray, refFloats, 0);
        env->ReleaseFloatArrayElements(voxPitchArray, voxFloats, 0);

        return;
    } else {
        return;
    }
}

/*
 * Class:     smtown_ai_karaokescore_KaraokeScore
 * Method:    getPartCountNative
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_smtown_ai_karaokescore_KaraokeScore_getPartCountNative
        (JNIEnv *env, jobject obj, jint scoreInstanceID) {
    if (scoreInstanceID < 0 || scoreInstanceID >= instances_num)
        return 0;

    KaraokeScore *score = karaokeScores[scoreInstanceID];
    if (score != nullptr) {
        return score->getPartCount();
    } else {
        return 0;
    }
}

/*
 * Class:     smtown_ai_karaokescore_KaraokeScore
 * Method:    getScoreNative
 * Signature: (IIFFFIII)F
 */
JNIEXPORT jfloat JNICALL Java_smtown_ai_karaokescore_KaraokeScore_getScoreNative
        (JNIEnv *env, jobject obj, jint scoreInstanceID, jint partIndex, jfloat min, jfloat max, jfloat random, jint pitchTune,
            jint tolerancePitch, jint toleranceTimeInMS) {
    if (scoreInstanceID < 0 || scoreInstanceID >= instances_num)
        return min + (max - min) / 2;

    KaraokeScore *score = karaokeScores[scoreInstanceID];
    if (score != nullptr) {
        return score->getScore(partIndex, min, max, random, pitchTune, tolerancePitch, toleranceTimeInMS);
    } else {
        return min + (max - min) / 2;
    }
}

/*
 * Class:     smtown_ai_karaokescore_KaraokeScore
 * Method:    releaseNative
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_smtown_ai_karaokescore_KaraokeScore_releaseNative
        (JNIEnv *env, jobject obj, jint scoreInstanceID) {
    if (scoreInstanceID < 0 || scoreInstanceID >= instances_num)
        return;

    KaraokeScore *score = karaokeScores[scoreInstanceID];
    if(score != nullptr) {
        delete score;
        karaokeScores[scoreInstanceID] = nullptr;
        return;
    } else {
        return;
    }
}

/*
 * Class:     smtown_ai_karaokescore_KaraokeScore
 * Method:    getAudioBufferLength
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_smtown_ai_karaokescore_KaraokeScore_getAudioBufferLength
        (JNIEnv *env, jobject obj, jint scoreInstanceID) {
    if (scoreInstanceID < 0 || scoreInstanceID >= instances_num)
        return -1;

    KaraokeScore *score = karaokeScores[scoreInstanceID];
    if(score != nullptr) {
        return score->getAudioBufferLength();
    } else {
        return -1;
    }
}

/*
 * Class:     smtown_ai_karaokescore_KaraokeScore
 * Method:    getAudioBuffer
 * Signature: (I[S)V
 */
JNIEXPORT void JNICALL Java_smtown_ai_karaokescore_KaraokeScore_getAudioBuffer
        (JNIEnv *env, jobject obj, jint scoreInstanceID, jshortArray audioBuffer) {
    if (scoreInstanceID < 0 || scoreInstanceID >= instances_num)
        return;

    KaraokeScore *score = karaokeScores[scoreInstanceID];
    if(score != nullptr) {
        jshort *jshorts = env->GetShortArrayElements(audioBuffer, NULL);

        score->getAudioBuffer(jshorts);

        env->ReleaseShortArrayElements(audioBuffer, jshorts, 0);
        return;
    } else {
        return;
    }
}