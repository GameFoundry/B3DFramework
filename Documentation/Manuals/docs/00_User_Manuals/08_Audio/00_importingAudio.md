---
title: Importing audio
---

Audio in the framework is represented in the form of an @b3d::AudioClip object. An audio clip is a resource, meaning it can be imported, saved and loaded as we described in the resource manuals.

Different audio file formats are supported depending on which audio backend is used:
 - OpenAudio (default)
   - .FLAC 
   - .OGG
   - .WAV
 - FMOD
   - .AIFF
   - .ASF
   - .ASX
   - .DLS
   - .FLAC
   - .MIDI
   - .MP3
   - .OGG
   - .WAV
   - .WMA
  
~~~~~~~~~~~~~{.cpp}
// Import an audio clip from disk
HAudioClip audioClip = GetImporter().Import<AudioClip>("myAudioClip.ogg");
~~~~~~~~~~~~~

# Customizing import
Import can be customized by providing a @b3d::AudioClipImportOptions object to the importer.

~~~~~~~~~~~~~{.cpp}
TShared<AudioClipImportOptions> importOptions = AudioClipImportOptions::Create();
// Set required options here (as described below)

HAudioClip audioClip = GetImporter().Import<AudioClip>("myAudioClip.ogg", importOptions);
~~~~~~~~~~~~~

A variety of properties can be customized on import, the most important of which being audio format, compression/streaming mode, bit depth and 2D/3D toggle.

## Audio format
Audio format determines in what format will audio data be stored in, once imported. It is controlled by @b3d::AudioClipImportOptions::Format which accepts @b3d::AudioFormat enumeration.

There are only two formats supported:
 - **PCM** - Raw uncompressed audio data stored in pulse-code modulation format
 - **VORBIS** - Compressed audio data stored in the Vorbis format

In most cases you want to use the Vorbis compressed format to save on memory use. Using uncompressed audio can be useful when:
 - No quality loss can be accepted
 - CPU overhead of decompression is too high (see streaming/decompression section below)

~~~~~~~~~~~~~{.cpp}
importOptions->Format = AudioFormat::VORBIS;
~~~~~~~~~~~~~

## Streaming/decompression
When an audio clip is loaded for playback, you can choose how its data is accessed to tweak required memory and CPU usage by setting the read mode through @b3d::AudioClipImportOptions::ReadMode.

Available read modes are part of the @b3d::AudioReadMode enumeration:
 - **LoadDecompressed** - When an audio clip is first loaded, it will immediately be decompressed in entirety, and playback will then proceed using the uncompressed version. This uses more memory as the uncompressed clip is present in its entirety in memory, but has very small CPU overhead during playback due to not requiring decompression. This option is not relevant for clips in PCM format, as they are already decompressed.
 - **LoadCompressed** - Audio clip will be loaded compressed into memory, and then decompressed as playback requires. This requires only the compressed audio data to be in memory, having a much smaller memory footprint than the method above. However, it has higher CPU overhead as decompression is done on-the-fly. This option is not relevant for clips in PCM format, as they are already decompressed.
 - **Stream** - Audio clip data will not be loaded into memory until playback starts. At that point, only the required portion of the clip will be loaded and older portions unloaded as playback proceeds. If the source data is compressed, decompression is also performed on-the-fly, same as with **LoadCompressed**. This uses the least amount of memory since it doesn't require the audio clip to be loaded fully in memory, but has a decompression overhead if source is compressed (same as **LoadCompressed**) as well as an IO overhead as it needs to read data from the storage device during playback.

In most cases you will want to use **LoadCompressed** for smaller audio clips (like audio cues), and **Stream** for longer clips (like music).

~~~~~~~~~~~~~{.cpp}
importOptions->ReadMode = AudioReadMode::LoadCompressed;
~~~~~~~~~~~~~

## Bit depth
@b3d::AudioClipImportOptions::BitDepth controls how many bits a single audio sample will be stored in. Smaller values yield smaller memory footprint but lower quality, and vice versa.

In general, a value of 16 is enough (the default). Other accepted values are 8, 24, and 32.

~~~~~~~~~~~~~{.cpp}
importOptions->BitDepth = 24;
~~~~~~~~~~~~~

## 2D/3D
Audio files can be played back in 2D or 3D mode. 2D audio is familiar audio playback, like music. 3D audio will control playback volume, pitch, and other properties based on the position and velocity of the listener compared to the audio source. This is controlled by setting @b3d::AudioClipImportOptions::Is3D. Note this is enabled by default.

In general, you want to use 2D audio for music or narration, and 3D audio for audio cues, in-game character voices, and similar. 3D clips will be converted to mono on import.

~~~~~~~~~~~~~{.cpp}
importOptions->Is3D = false;
~~~~~~~~~~~~~

# Querying audio clip properties

After importing an audio clip, you can query various properties:

~~~~~~~~~~~~~{.cpp}
// Get audio format
AudioFormat format = audioClip->GetFormat();

// Get read mode
AudioReadMode readMode = audioClip->GetReadMode();

// Get bit depth
u32 bitDepth = audioClip->GetBitDepth();

// Get sample rate (frequency)
u32 sampleRate = audioClip->GetFrequency();

// Get number of channels
u32 channelCount = audioClip->GetChannelCount();

// Get total duration in seconds
float duration = audioClip->GetLength();

// Get total sample count (includes all channels)
u32 sampleCount = audioClip->GetSampleCount();

// Check if 3D audio
bool is3D = audioClip->Is3D();
~~~~~~~~~~~~~
