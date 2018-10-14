#import "Superpowered-SwiftSample-Bridging-Header.h"
#import "SuperpoweredAdvancedAudioPlayer.h"
#import "SuperpoweredSimple.h"
#import "SuperpoweredIOSAudioIO.h"
#import <mach/mach_time.h>

/*
 This is a .mm file, meaning it's Objective-C++.
 You can perfectly mix it with Objective-C or Swift, until you keep the member variables and C++ related includes here.
 Yes, the header file (.h) isn't the only place for member variables.
 */
@implementation Superpowered {
    SuperpoweredAdvancedAudioPlayer *player;
    SuperpoweredIOSAudioIO *output;
    float *stereoBuffer;
    bool started;
    bool isLoop;
    uint64_t timeUnitsProcessed, maxTime;
    unsigned int lastPositionSeconds, lastSamplerate, samplesProcessed;
}

- (void)dealloc {
    delete player;
    free(stereoBuffer);
#if !__has_feature(objc_arc)
    [output release];
    [super dealloc];
#endif
}

- (void)togglePlayback { // Play/pause.
    player->togglePlayback();
}

- (void)play {
    player->play(false);
}

- (void)pause {
    player->pause();
}

- (void)seekTo:(float)percent {
    player->seek(percent);
}

- (void)setTempo:(float)tempo {
    player->setTempo(tempo, 1.0);
}

- (void)setPitch:(int)pitch {
    player->setPitchShift(pitch);
}

- (void)toggle {
    if (started) [output stop]; else [output start];
    started = !started;
}

- (void)setSong:(NSURL*)url {
    started = false;
    NSString *filePathStr = url.absoluteString;
    char *filePathChar = (char *)[filePathStr UTF8String];
    player->open(filePathChar);
    player->setBpm(125.0f);
}

- (void)setLoop:(bool)loop {
    self->isLoop = loop;
}


- (void)interruptionStarted {}
- (void)recordPermissionRefused {}
- (void)mapChannels:(multiOutputChannelMap *)outputMap inputMap:(multiInputChannelMap *)inputMap externalAudioDeviceName:(NSString *)externalAudioDeviceName outputsAndInputs:(NSString *)outputsAndInputs {}

- (void)interruptionEnded {
    player->onMediaserverInterrupt(); // If the player plays Apple Lossless audio files, then we need this. Otherwise unnecessary.
}

// This is where the Superpowered magic happens.
static bool audioProcessing(void *clientdata, float **buffers, unsigned int inputChannels, unsigned int outputChannels, unsigned int numberOfSamples, unsigned int samplerate, uint64_t hostTime) {
    __unsafe_unretained Superpowered *self = (__bridge Superpowered *)clientdata;
    uint64_t startTime = mach_absolute_time();
    
    if (samplerate != self->lastSamplerate) { // Has samplerate changed?
        self->lastSamplerate = samplerate;
        self->player->setSamplerate(samplerate);
    };
    
    /*
     Let's process some audio.
     */
    bool silence = !self->player->process(self->stereoBuffer, false, numberOfSamples, 1.0f, 0.0f, -1.0);
    
    // CPU measurement code to show some nice numbers for the business guys.
    uint64_t elapsedUnits = mach_absolute_time() - startTime;
    if (elapsedUnits > self->maxTime) self->maxTime = elapsedUnits;
    self->timeUnitsProcessed += elapsedUnits;
    self->samplesProcessed += numberOfSamples;
    if (self->samplesProcessed >= samplerate) {
        self->avgUnitsPerSecond = self->timeUnitsProcessed;
        self->maxUnitsPerSecond = (double(samplerate) / double(numberOfSamples)) * self->maxTime;
        self->samplesProcessed = self->timeUnitsProcessed = self->maxTime = 0;
    };
    
    self->playing = self->player->playing;
    if (!silence) SuperpoweredDeInterleave(self->stereoBuffer, buffers[0], buffers[1], numberOfSamples); // The stereoBuffer is ready now, let's put the finished audio into the requested buffers.
    return !silence;
}

void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    SuperpoweredAdvancedAudioPlayer *cdPlayer = *((SuperpoweredAdvancedAudioPlayer **)clientData);
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        // Handle player prepared
    } else if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadError) {
        // Handle error with player
    } else if (event == SuperpoweredAdvancedAudioPlayerEvent_EOF && !cdPlayer->looping) {
        // Notify at the end of music
        [[NSNotificationCenter defaultCenter] postNotificationName:@"spPlayerEnded" object:nil];
        cdPlayer->pause();
    }
}

- (id)init {
    self = [super init];
    if (!self) return nil;
    started = false;
    lastPositionSeconds = lastSamplerate = samplesProcessed = timeUnitsProcessed = maxTime = avgUnitsPerSecond = maxUnitsPerSecond = 0;
    if (posix_memalign((void **)&stereoBuffer, 16, 4096 + 128) != 0) abort(); // Allocating memory, aligned to 16.
    // Create the Superpowered units we'll use.
    player = new SuperpoweredAdvancedAudioPlayer(&player, playerEventCallback, 44100, 0);
    output = [[SuperpoweredIOSAudioIO alloc] initWithDelegate:(id<SuperpoweredIOSAudioIODelegate>)self preferredBufferSize:12 preferredMinimumSamplerate:44100 audioSessionCategory:AVAudioSessionCategoryPlayback channels:2 audioProcessingCallback:audioProcessing clientdata:(__bridge void *)self];
    return self;
}

@end
