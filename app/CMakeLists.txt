# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

if (${ANDROID_ABI} STREQUAL "armeabi-v7a" OR ${ANDROID_ABI} STREQUAL "arm64-v8a")
  # make a list of neon files and add neon compiling flags to them
  add_definitions("-DHAVE_NEON=1")
endif ()

set(
	PATH_TO_SUPERPOWERED
	${CMAKE_CURRENT_SOURCE_DIR}/libs/Superpowered
)



######################
## MidiFile library ##
######################

add_library(midifile-lib
            SHARED
            src/main/cpp/midifile/Binasc.cpp
            src/main/cpp/midifile/MidiEvent.cpp
            src/main/cpp/midifile/MidiEventList.cpp
            src/main/cpp/midifile/MidiFile.cpp
            src/main/cpp/midifile/MidiMessage.cpp
            src/main/cpp/midifile/Options.cpp)

target_include_directories(midifile-lib PUBLIC
                            src/main/cpp/midifile)

#########
## SSA ##
#########

add_library(ssa-lib
            SHARED
            src/main/cpp/ssa/Dialogue.cpp
            src/main/cpp/ssa/Part.cpp
            src/main/cpp/ssa/Property.cpp
            src/main/cpp/ssa/Section.cpp
            src/main/cpp/ssa/SSA.cpp
            src/main/cpp/ssa/Word.cpp)

###########################
## karaoke score library ##
###########################

add_library(karaokescore-lib
            SHARED
            src/main/cpp/smtown_ai_karaokescore_KaraokeScore.cpp
            src/main/cpp/PitchUtil.cpp
            #src/main/cpp/CepstrumPitchDetector.cpp
            src/main/cpp/ACFPitchDetector.cpp
            src/main/cpp/KaraokeScore.cpp
            src/main/cpp/PrecisionRecallScore.cpp
            src/main/cpp/AudioDecoder.cpp)

target_include_directories(karaokescore-lib PUBLIC
                            src/main/cpp/Superpowered
                            src/main/cpp/ssa)

find_library(log-lib log)

target_link_libraries( karaokescore-lib
                       midifile-lib
                       ssa-lib
                       ${PATH_TO_SUPERPOWERED}/libSuperpoweredAndroid${ANDROID_ABI}.a
                       ${log-lib})

