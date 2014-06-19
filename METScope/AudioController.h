//
//  AudioController.h
//  DigitalSoundFX
//
//  Created by Jeff Gregorio on 5/11/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <pthread.h>

#define kAudioSampleRate        44100.0
#define kAudioBytesPerPacket    4
#define kAudioFramesPerPacket   1
#define kAudioChannelsPerFrame  2
#define kMaxPlotBufferTime      3.0

/* Struct of class data passed to the callback method(s) */
typedef struct AudioPlayer {
    AUGraph graph;                      // AUGraph
    AudioUnit remoteIOUnit;             // Input/Output audio unit
    Float32 gain;                       // Input gain control
    UInt32 bufferSizeFrames;            // Current buffer length
    Float32 *currentBuffer;             //
    UInt32 longScaleBufferLength;
    Float32 *longScaleBuffer;
    pthread_mutex_t currentBufferMutex;   // Buffer mutexes
    pthread_mutex_t longScaleBufferMutex;
} AudioPlayer;

#pragma mark -
#pragma mark AudioController
@interface AudioController : NSObject {
    
    AudioPlayer player;
    AudioStreamBasicDescription IOStreamFormat;
}

@property (readonly) bool inputEnabled;
@property (readonly) bool outputEnabled;
@property (readonly) bool isRunning;
@property (readonly) bool isInitialized;

/* Update the input gain */
- (void)setGain:(Float32)gain;

/* Get the current buffer length */
- (UInt32)getBufferLength;
- (UInt32)getLongScaleBufferLength;

/* Start/stop audio */
- (void)startAUGraph;
- (void)stopAUGraph;

/* Enable/disable audio input/output */
- (void)setInputEnabled: (bool)enabled;
- (void)setOutputEnabled:(bool)enabled;

/* Internal pre/post processing buffer getters */
- (void)getCurrentBuffer:(Float32 *)outBuffer;
- (void)getLongScaleBuffer:(Float32 *)outBuffer;

@end
