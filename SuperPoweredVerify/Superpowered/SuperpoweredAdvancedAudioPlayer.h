#ifndef Header_SuperpoweredAdvancedAudioPlayer
#define Header_SuperpoweredAdvancedAudioPlayer

struct SuperpoweredAdvancedAudioPlayerInternals;
struct SuperpoweredAdvancedAudioPlayerBase;

typedef struct stemsCompressor {
    bool enabled;
    float inputGainDb;
    float outputGainDb;
    float dryWetPercent;
    float ratio;
    float attackSec;
    float releaseSec;
    float thresholdDb;
    float hpCutoffHz;
} stemsCompressor;

typedef struct stemsLimiter {
    bool enabled;
    float releaseSec;
    float thresholdDb;
    float ceilingDb;
} stemsLimiter;

typedef struct stemsInfo {
    char *names[4];
    char *colors[4];
    stemsCompressor compressor;
    stemsLimiter limiter;
} stemsInfo;

typedef enum SuperpoweredAdvancedAudioPlayerSyncMode {
    SuperpoweredAdvancedAudioPlayerSyncMode_None,
    SuperpoweredAdvancedAudioPlayerSyncMode_Tempo,
    SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat
} SuperpoweredAdvancedAudioPlayerSyncMode;

typedef enum SuperpoweredAdvancedAudioPlayerJogMode {
    SuperpoweredAdvancedAudioPlayerJogMode_Scratch,
    SuperpoweredAdvancedAudioPlayerJogMode_PitchBend,
    SuperpoweredAdvancedAudioPlayerJogMode_Parameter
} SuperpoweredAdvancedAudioPlayerJogMode;

typedef enum SuperpoweredAdvancedAudioPlayerEvent {
    SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess,
    SuperpoweredAdvancedAudioPlayerEvent_LoadError,
    SuperpoweredAdvancedAudioPlayerEvent_HLSNetworkError,
    SuperpoweredAdvancedAudioPlayerEvent_ProgressiveDownloadError,
    SuperpoweredAdvancedAudioPlayerEvent_EOF,
    SuperpoweredAdvancedAudioPlayerEvent_JogParameter,
    SuperpoweredAdvancedAudioPlayerEvent_DurationChanged,
    SuperpoweredAdvancedAudioPlayerEvent_LoopEnd,
} SuperpoweredAdvancedAudioPlayerEvent;

typedef struct hlsStreamAlternative {
    char *uri, *name, *language, *groupid;
    int bps;
    bool isDefault, isMp4a;
} hlsStreamAlternative;

#define HLS_DOWNLOAD_EVERYTHING 86401
#define HLS_DOWNLOAD_REMAINING 86400

/**
 @brief Events happen asynchronously, implement this callback to get notified.
 
 LoadSuccess, LoadError, HLSNetworkError and ProgressiveDownloadError are called from an internal thread of this object.
 
 EOF (end of file) and ScratchControl are called from the (probably real-time) audio processing thread, you shouldn't do any expensive there.
 
 clientData Some custom pointer you set when you created the SuperpoweredAdvancedAudioPlayer instance.
 event What happened (load success, load error, end of file, jog parameter).
 value A pointer to a stemsInfo structure or NULL for LoadSuccess (you take ownership over the strings). (const char *) for LoadError, pointing to the error message. (double *) for JogParameter in the range of 0.0 to 1.0. (bool *) for EOF, set it to true to pause playback. (bool *) for LoopEnd, set it to false to exit the loop. Don't call this instance's methods from an EOF event callback!
 */
typedef void (* SuperpoweredAdvancedAudioPlayerCallback) (void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value);

/**
 The maximum playback or scratch speed.
 */
#define SUPERPOWEREDADVANCEDAUDIOPLAYER_MAXSPEED 20.0f


/**
 @brief High performance advanced audio player with:
 
 - time-stretching and pitch shifting,
 
 - beat and tempo sync,
 
 - scratching,
 
 - tempo bend,
 
 - looping,
 
 - slip mode,
 
 - fast seeking (cached points),
 
 - momentum and jog wheel handling,
 
 - 0 latency, real-time operation,
 
 - low memory usage,
 
 - thread safety (all methods are thread-safe),
 
 - direct iPod music library access.
 
 Can not be used for offline processing. Supported file types:
 - Stereo or mono pcm WAV and AIFF (16-bit int, 24-bit int, 32-bit int or 32-bit IEEE float).
 - MP3 (all kind).
 - AAC-LC in M4A container (iTunes).
 - AAC-LC in ADTS container (.aac).
 - Apple Lossless (on iOS only).
 - Http Live Streaming (HLS): vod/live/event streams, AAC-LC/MP3 in audio files or MPEG-TS files. Support for byte ranges and AES-128 encryption.

 positionMs The current position. Always accurate, no matter of time-stretching and other transformations. Read only.
 positionPercent The current position as a percentage (0.0f to 1.0f). Read only.
 positionSeconds The current position as seconds elapsed. Read only.
 displayPositionMs Same as positionMs, but positionMs is not updated until seeking to a different position is finished. This is updated immediately after setPosition() or seek() is called.  Read only.
 durationMs The duration of the current track in milliseconds. Equals to UINT_MAX for live streams. Read only.
 durationSeconds The duration of the current track in seconds. Equals to UINT_MAX for live streams. Read only.
 waitingForBuffering Indicates if the player waits for audio data to be bufferred. Read only.
 playing Indicates if the player is playing or paused. Read only.
 tempo The current tempo. Read only.
 masterTempo Time-stretching is enabled or not. Read only.
 pitchShift Note offset from -12 to 12. 0 means no pitch shift. Read only.
 pitchShiftCents Pitch shift cents, from -1200 (one octave down) to 1200 (one octave up). 0 means no pitch shift. Read only.
 bpm Must be correct for syncing. There is no auto-bpm detection inside. Read only.
 currentBpm The actual bpm of the track (as bpm changes with the current tempo). Read only.
 slip If enabled, scratching or reverse will maintain the playback position as if you had never entered those modes. Read only.
 scratching The player is in scratching mode or not. Read only.
 reverse Indicates if the playback goes backwards. Read only.
 looping Indicates if looping is enabled. Read only.
 firstBeatMs Tells where the first beat (the beatgrid) begins. Must be correct for syncing. Read only.
 msElapsedSinceLastBeat How many milliseconds elapsed since the last beat. Read only.
 phase Reserved for future use.
 quantum Reserved for future use.
 bendMsOffset Reserved for future use.
 beatIndex Which beat has just happened (1 [1.0f-1.999f], 2 [2.0f-2.999f], 3 [3.0f-3.99f], 4 [4.0f-4.99f]). A value of 0 means "don't know". Read only.
 bufferStartPercent What is buffered from the original source, start point. Will always be 0 for non-network sources (files). Read only.
 bufferEndPercent What is buffered from the original source, end point. Will always be 1.0f for non-network sources (files). Read only.
 currentBps The current download speed.
 syncMode The current sync mode (off, tempo, or tempo and beat).
 fixDoubleOrHalfBPM If tempo is >1.4f or <0.6f, it will treat the bpm as half or double. Good for certain genres. False by default.
 waitForNextBeatWithBeatSync Wait for the next beat if beat-syncing is enabled. False by default.
 dynamicHLSAlternativeSwitching Dynamicly changing the current HLS alternative to match the available network bandwidth. Default is true.
 reverseToForwardAtLoopStart If looping and playback direction is reverse, reaching the beginning of the loop will change direction to forward. False by default.
 downloadSecondsAhead The HLS content download strategy: how many seconds ahead of the playback position to download. Default is HLS_DOWNLOAD_REMAINING, meaning it will download everything after the playback position, until the end. HLS_DOWNLOAD_EVERYTHING downloads before the playback position too.
 maxDownloadAttempts If HLS download fails, how many times to try until sleep. Default: 100. After sleep, NetworkError is called continously.
 minTimeStretchingTempo Will not time-stretch, just resample below this tempo. Default: 0.501f (recommended value for low CPU on older mobile devices, such as the first iPad). Set this before an open() call. 
 maxTimeStretchingTempo Will not time-stretch, just resample above this tempo. Default: 2.0f (recommended value for low CPU on older mobile devices, such as the first iPad).
 handleStems Output 4 distinct stereo pairs for Native Instruments STEMS format. Default: false (output stem 0 for STEMS).
 defaultQuantum Reserved for future use.
 fullyDownloadedFilePath The file system path of the fully downloaded audio file for progressive downloads. Progressive downloads are automatically removed if no SuperpoweredAdvancedAudioPlayer instance is active for the same url. This parameter provides an alternative to save the file.
 tempFolderPath The path for temporary files.
*/
class SuperpoweredAdvancedAudioPlayer {
public:
// READ ONLY parameters, don't set them directly, use the methods below.
    double positionMs;
    float positionPercent;
    int positionSeconds;
    double displayPositionMs;
    unsigned int durationMs;
    unsigned int durationSeconds;
    bool waitingForBuffering;
    bool playing;

    double tempo;
    bool masterTempo;
    int pitchShift;
    int pitchShiftCents;
    double bpm;
    double currentBpm;
    
    bool slip;
    bool scratching;
    bool reverse;
    bool looping;
    
    double firstBeatMs;
    double msElapsedSinceLastBeat;
    double phase;
    double quantum;
    double bendMsOffset;
    float beatIndex;

    float bufferStartPercent;
    float bufferEndPercent;
    int currentBps;

    char *fullyDownloadedFilePath;
    static char *tempFolderPath;

// READ-WRITE parameters
    SuperpoweredAdvancedAudioPlayerSyncMode syncMode;
    bool fixDoubleOrHalfBPM;
    bool waitForNextBeatWithBeatSync;
    bool dynamicHLSAlternativeSwitching;
    bool reverseToForwardAtLoopStart;
    int downloadSecondsAhead;
    int maxDownloadAttempts;
    float minTimeStretchingTempo;
    float maxTimeStretchingTempo;
    bool handleStems;
    double defaultQuantum;

/**
 @brief Set the folder path for temporary files. Used for HLS and progressive download only.
 
 Call this first before any player instance is created. It will create a subfolder with the name "SuperpoweredAAP" in this folder.
 
 path File system path of the folder.
 */
    static void setTempFolder(const char *path);

/**
 @brief Remove the temp folder. Use this when your program ends.
 */
    static void clearTempFolder();
    
/**
 @brief Create a player instance with the current sample rate value.
 
 Example: SuperpoweredAdvancedAudioPlayer player = new SuperpoweredAdvancedAudioPlayer(this, playerCallback, 44100, 4);
 
 clientData A custom pointer your callback receives.
 callback Your callback to receive player events.
 sampleRate The current sample rate.
 cachedPointCount Sets how many positions can be cached in the memory. Jumping to a cached point happens with 0 latency. Loops are automatically cached.
 internalBufferSizeSeconds The number of seconds to buffer internally for playback and cached points. Minimum 2, maximum 60. Default: 2.
 negativeSeconds The number of seconds of silence in the negative direction, before the beginning of the track.
*/
    SuperpoweredAdvancedAudioPlayer(void *clientData, SuperpoweredAdvancedAudioPlayerCallback callback, unsigned int sampleRate, unsigned char cachedPointCount, unsigned int internalBufferSizeSeconds = 2, unsigned int negativeSeconds = 0);
    ~SuperpoweredAdvancedAudioPlayer();
/**
 @brief Opens an audio file with playback paused.
 
 Tempo, pitchShift, masterTempo and syncMode are NOT changed if you open a new one.
 
 path Full file system path or progressive download path (http or https).
 customHTTPHeaders NULL terminated list of custom headers for http communication.
*/
    void open(const char *path, char **customHTTPHeaders = 0);
    
/**
 @brief Opens an audio file with playback paused.
 
 Tempo, pitchShift, masterTempo and syncMode are NOT changed if you open a new one.
 
 path Full file system path or progressive download path (http or https).
 offset The byte offset inside the file.
 length The byte length from the offset.
 customHTTPHeaders NULL terminated list of custom headers for http communication.
*/
    void open(const char *path, int offset, int length, char **customHTTPHeaders = 0);
/**
 @brief Opens a HTTP Live Streaming stream with playback paused.
 
 Tempo, pitchShift, masterTempo and syncMode are NOT changed if you open a new one.
 
 url URL of the stream.
 customHTTPHeaders NULL terminated list of custom headers for http communication.
 */
    void openHLS(const char *url, char **customHTTPHeaders = 0);

/**
 @brief Starts playback.
 
 synchronised Set it to true for a beat-synced or tempo-synced start.
 */
    void play(bool synchronised);
    
/**
 @brief Pause playback. 
 
 There is no need for a "stop" method, this player is very efficient with the battery and has no significant "stand-by" processing.
 
 decelerateSeconds Optional momentum. 0 means pause immediately.
 slipMs Enable slip mode for a specific amount of time, or 0 to not slip.
 */
    void pause(float decelerateSeconds = 0, unsigned int slipMs = 0);
    
/**
 @brief Toggle play/pause.
 */
    void togglePlayback();
/**
 @brief Simple seeking to a percentage.
 */
    void seek(double percent);
/**
 @brief Precise seeking.
 
 ms Position in milliseconds.
 andStop If true, stops playback.
 synchronisedStart If andStop is false, a beat-synced start is possible.
 */
    void setPosition(double ms, bool andStop, bool synchronisedStart);
/**
 @brief Cache a position for zero latency seeking. It will cache around +/- 1 second around this point.
 
 ms Position in milliseconds.
 pointID Use this to provide a custom identifier, so you can overwrite the same point later. Use 255 for a point with no identifier.
*/
    void cachePosition(double ms, unsigned char pointID);
/**
 @brief Loop from a start point with some length.
 
 startMs Loop from this millisecond.
 lengthMs Length in millisecond.
 jumpToStartMs If the playhead is within the loop, jump to startMs or not.
 pointID Looping caches startMs, so you can specify a pointID too (or set to 255 if you don't care).
 synchronisedStart Beat-synced start.
 forceDefaultQuantum Reserved for future use.
 */
    bool loop(double startMs, double lengthMs, bool jumpToStartMs, unsigned char pointID, bool synchronisedStart, bool forceDefaultQuantum = false);
/**
 @brief Loop from a start to an end point.
     
 startMs Loop from this millisecond.
 endMs Loop to this millisecond.
 jumpToStartMs If the playhead is within the loop, jump to startMs or not.
 pointID Looping caches startMs, so you can specify a pointID too (or set to 255 if you don't care).
 synchronisedStart Beat-synced start.
 forceDefaultQuantum Reserved for future use.
*/
    bool loopBetween(double startMs, double endMs, bool jumpToStartMs, unsigned char pointID, bool synchronisedStart, bool forceDefaultQuantum = false);
/**
 @brief Exits from the current loop.
 
 synchronisedStart Synchronized start after the loop exit.
 */
    void exitLoop(bool synchronisedStart = false);
/**
 @brief Checks if ms fall into the current loop.
 
 ms The position to check in milliseconds.
 */
    bool msInLoop(double ms);
/**
 @brief There is no auto-bpm detection inside, you must set the original bpm of the track with this for syncing.
 
 Should be called after a successful open().
 
 newBpm The bpm value. A number below 10.0f means "bpm unknown", and sync will not work.
*/
    void setBpm(double newBpm);
/**
  @brief Beat-sync works only if the first beat's position is known. Set it here.
 
  Should be called after a successful open().
 */
    void setFirstBeatMs(double ms);
/**
 @brief Shows you where the closest beat is to a specific position.
 
 ms The position in milliseconds.
 beatIndex Pointer to a beat index value. Set to NULL if beat index is not important. Set to 0 if you want to retrieve the beat index of the position. Set to 1-4 if beat index is important.
*/
    double closestBeatMs(double ms, unsigned char *beatIndex);
    
/**
 @brief "Virtual jog wheel" or "virtual turntable" handling. 
 
 ticksPerTurn Sets the sensitivity of the virtual wheel. Use around 2300 for pixel-perfect touchscreen waveform control.
 mode Jog wheel mode (scratching, pitch bend, or parameter set in the range 0.0 to 1.0).
 scratchSlipMs Enable slip mode for a specific amount of time for scratching, or 0 to not slip.
*/
    void jogTouchBegin(int ticksPerTurn, SuperpoweredAdvancedAudioPlayerJogMode mode, unsigned int scratchSlipMs);
/**
 @brief A jog wheel should send some "ticks" according to the movement. A waveform's movement in pixels for example.
 
 value The cumulated ticks value.
 bendStretch Use time-stretching for bending or not (false makes it "audible").
 bendMaxPercent The maximum tempo change for pitch bend, should be between 0.01f and 0.3f (1% and 30%).
 bendHoldMs How long to maintain the bended state. A value >= 1000 will hold until endContinuousPitchBend is called.
 parameterMode True: if there was no jogTouchBegin, SuperpoweredAdvancedAudioPlayerJogMode_Parameter applies. False: if there was no jogTouchBegin, SuperpoweredAdvancedAudioPlayerJogMode_PitchBend applies.
*/
    void jogTick(int value, bool bendStretch, float bendMaxPercent, unsigned int bendHoldMs, bool parameterMode);
/**
 @brief Call this when the jog touch ends.
 
 decelerate The decelerating rate for momentum. Set to 0.0f for automatic.
 synchronisedStart Beat-synced start after decelerating.
 */
    void jogTouchEnd(float decelerate, bool synchronisedStart);
/**
 @brief Sets the relative tempo of the playback.
 
 tempo 1.0f is "original speed".
 masterTempo Enable or disable time-stretching.
 */
    void setTempo(double tempo, bool masterTempo);
/**
 @brief Sets the pitch shift value. Needs masterTempo enabled.
 
 pitchShift Note offset from -12 to 12. 0 means no pitch shift.
 */
    void setPitchShift(int pitchShift);
    
/**
 @brief Sets the pitch shift value with greater precision. Calling this method requires magnitudes more CPU than setPitchShift.
 
 pitchShiftCents Limited to >= -1200 and <= 1200. 0 means no pitch shift.
 */
    void setPitchShiftCents(int pitchShiftCents);

/**
 @brief Sets playback direction.
 
 reverse Playback direction.
 slipMs Enable slip mode for a specific amount of time, or 0 to not slip.
 */
    void setReverse(bool reverse, unsigned int slipMs);
/**
 @brief Pitch bend (temporary tempo change).
 
 maxPercent The maximum tempo change for pitch bend, should be between 0.01f and 0.3f (1% and 30%).
 bendStretch Use time-stretching for bending or not (false makes it "audible").
 faster Playback speed change direction.
 holdMs How long to maintain the bended state. A value >= 1000 will hold until endContinuousPitchBend is called.
*/
    void pitchBend(float maxPercent, bool bendStretch, bool faster, unsigned int holdMs);
/**
 @brief Ends pitch bend.
 */
    void endContinuousPitchBend();
/**
 @brief Reserved for future use.
 */
    void resetBendMsOffset();
/**
 @brief Call when scratching starts.
 
 @warning This is an advanced method, use it only if you don't want the jogT... methods.
 
 slipMs Enable slip mode for a specific amount of time for scratching, or 0 to not slip.
 stopImmediately Stop now or not.
 */
    void startScratch(unsigned int slipMs, bool stopImmediately);
/**
 @brief Scratch movement.
 
 @warning This is an advanced method, use it only if you don't want the jogT... methods.
 
 pitch The current speed.
 smoothing Should be between 0.05f (max. smoothing) and 1.0f (no smoothing).
 */
    void scratch(double pitch, float smoothing);
/**
 @brief Ends scratching.
 
 @warning This is an advanced method, use it only if you don't want the jogT... methods.
 
 returnToStateBeforeScratch Return to the previous playback state (direction, speed) or not.
 */
    void endScratch(bool returnToStateBeforeScratch);
/**
 @brief Returns the last process() numberOfSamples converted to milliseconds.
 */
    double lastProcessMs();
/**
 @brief Sets the sample rate.
     
 sampleRate 44100, 48000, etc.
*/
    void setSamplerate(unsigned int sampleRate);
/**
 @brief Call this on a phone call or other interruption.
 
 Apple's built-in codec may be used in some cases, for example ALAC files. 
 Call this after a media server reset or audio session interrupt to resume playback.
*/
    void onMediaserverInterrupt();
/**
 @brief Reserved for future use.

 phase Reserved for future use.
 quantum Reserved for future use.
 */
    double getMsDifference(double phase, double quantum);

/**
 @brief Processes the audio, stereo version.
 
 @return Put something into output or not.
 
 buffer 32-bit interleaved stereo input/output buffer. Should be numberOfSamples * 8 + 64 bytes big.
 bufferAdd If true, the contents of buffer will be preserved and audio will be added to them. If false, buffer is completely overwritten.
 numberOfSamples The number of samples to provide.
 volume 0.0f is silence, 1.0f is "original volume". Changes are automatically smoothed between consecutive processes.
 masterBpm A bpm value to sync with. Use 0.0f for no syncing.
 masterMsElapsedSinceLastBeat How many milliseconds elapsed since the last beat on the other stuff we are syncing to. Use -1.0 to ignore.
 phase Reserved for future use.
 quantum Reserved for future use.
*/
    bool process(float *buffer, bool bufferAdd, unsigned int numberOfSamples, float volume = 1.0f, double masterBpm = 0.0f, double masterMsElapsedSinceLastBeat = -1.0, double phase = -1.0, double quantum = -1.0);

/**
 @brief Processes the audio, multi-channel version.

 @return Put something into output or not.

 buffers 32-bit interleaved stereo input/output buffer pairs. Each pair should be numberOfSamples * 8 + 64 bytes big.
 bufferAdds If true, the contents of buffer will be preserved and audio will be added to them. If false, buffer is completely overwritten.
 numberOfSamples The number of samples to provide.
 volumes 0.0f is silence, 1.0f is "original volume". Changes are automatically smoothed between consecutive processes.
 masterBpm A bpm value to sync with. Use 0.0f for no syncing.
 masterMsElapsedSinceLastBeat How many milliseconds elapsed since the last beat on the other stuff we are syncing to. Use -1.0 to ignore.
 phase Reserved for future use.
 quantum Reserved for future use.
 */
    bool processMulti(float **buffers, bool *bufferAdds, unsigned int numberOfSamples, float *volumes, double masterBpm = 0.0f, double masterMsElapsedSinceLastBeat = -1.0, double phase = -1.0, double quantum = -1.0);
    
private:
    SuperpoweredAdvancedAudioPlayerInternals *internals;
    SuperpoweredAdvancedAudioPlayerBase *base;
    SuperpoweredAdvancedAudioPlayer(const SuperpoweredAdvancedAudioPlayer&);
    SuperpoweredAdvancedAudioPlayer& operator=(const SuperpoweredAdvancedAudioPlayer&);
};

#endif
