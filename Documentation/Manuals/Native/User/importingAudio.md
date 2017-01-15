Importing audio 						{#importingAudio}
===============

Audio in Banshee is represented in the form of an @ref bs::AudioClip "AudioClip" object. An audio clip is a resource, meaning it can be imported, saved and loaded as we described in the Resource manuals.

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
HAudioClip clip = gImporter().import<AudioClip>("myAudioClip.ogg");
~~~~~~~~~~~~~

# Customizing import
Import can be customized by providing a @ref bs::AudioClipImportOptions "AudioClipImportOptions" object to the importer.

~~~~~~~~~~~~~{.cpp}
auto importOptions = AudioClipImportOptions::create();
// Set required options here (as described below)

HAudioClip clip = gImporter().import<AudioClip>("myAudioClip.ogg", importOptions);
~~~~~~~~~~~~~

A variety of properties can be customized on import, the most important of which being audio format, compression/streaming mode, bit depth and 2D/3D toggle.

## Audio format
Audio format determines in what format will audio data be stored in, once imported. It is controlled by @ref bs::AudioClipImportOptions::setFormat "AudioClipImportOptions::setFormat()" which accepts @ref bs::AudioFormat "AudioFormat" enumeration as a parameter.

There are only two formats supported:
 - PCM - This is raw uncompressed audio data stored in pulse-code modulation format
 - Vorbis - This is compressed audio data stored in the Vorbis format
 
In most cases you want to use the Vorbis compressed format, in order to save on memory use. Using uncompressed audio can be useful when:
 - No quality loss can be accepted
 - CPU overhead of decompression is too high (see streaming/decompression section below)
 
~~~~~~~~~~~~~{.cpp}
importOptions->setFormat(AudioFormat::Vorbis);
~~~~~~~~~~~~~

## Streaming/decompression
When an audio clip is loaded for playback, you can choose how is its data accessed, in order to tweak required memory and/or CPU usage by setting the read mode though @ref bs::AudioClipImportOptions::setReadMode "AudioClipImportOptions::setReadMode()".

Available read modes are part of the @ref bs::AudioReadMode "AudioReadMode" enumeration. They are:
 - **LoadDecompressed** - When an audio clip is first loaded it will immediately be decompressed in entirety, and playback will then proceed by using the uncompressed version. This uses more memory as the uncompressed clip is present in its entirety in memory, but has very small CPU overhead during playback due to it not requiring decompression. This option is not relevant for clips in PCM format, as they are already decompressed.
 - **LoadCompressed** - Audio clip will be loaded compressed into memory, and then decompressed as the playback requires. This requires only the decompressed audio data to be in memory, having much smaller memory footprint than the method above. However it has a higher CPU overhead as decompression is done on-the-fly. This option is not relevant for clips in PCM format, as they are already decompressed.
 - **Streaming** - Audio clip data will not be loaded into memory until playback starts. At that point only the required portion of the clip will be loaded and older portions unloaded, as playback proceeds. If the source data is compressed, this means the decompression is also performed on-the-fly, same as with **LoadCompressed**. This uses the least amount of memory, since it doesn't require the audio clip to be loaded fully in memory, but has a decompression overhead if source is compressed(same as **LoadCompressed**), as well as a IO overhead as it needs to read data from the storage device during playback.
 
In most cases you will want to use **LoadCompressed** for smaller audio clips (like audio cues), and **Streaming** for longer clips (like music).

~~~~~~~~~~~~~{.cpp}
importOptions->setReadMode(AudioReadMode::LoadCompressed);
~~~~~~~~~~~~~

## Bit depth
@ref bs::AudioClipImportOptions::setBitDepth "AudioClipImportOptions::setBitDepth()" controls how many bits will a single audio sample be stored in. Smaller values yield smaller memory footprint, but lesser quality, and the other way around.

In general value of 16 is enough (the default). Other accepted values are 8, 24 and 32.

~~~~~~~~~~~~~{.cpp}
importOptions->setBitDepth(24);
~~~~~~~~~~~~~

## 2D/3D
Audio files can be played back in 2D or 3D mode. 2D audio is the familiar audio playback, like music. 3D audio will control playback volume, pitch and other properties based on the position and velocity of the listener compared to the audio source. This can be controlled by setting the @ref bs::AudioClipImportOptions::setIs3D "AudioClipImportOptions::setIs3D()". Note this is on by default.

In general you want to use 2D audio for music or narration, and 3D audio for audio cues, in-game character voices and similar.

~~~~~~~~~~~~~{.cpp}
importOptions->setIs3D(false);
~~~~~~~~~~~~~