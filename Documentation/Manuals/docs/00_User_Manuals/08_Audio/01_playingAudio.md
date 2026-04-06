---
title: Playing audio
---

To play an audio clip create an @b3d::AudioSource component.

~~~~~~~~~~~~~{.cpp}
HSceneObject audioSourceSceneObject = SceneObject::Create("Audio source");
HAudioSource audioSource = audioSourceSceneObject->AddComponent<AudioSource>();
~~~~~~~~~~~~~

Each audio source can have a single **AudioClip** associated with it. Attach the clip to the source by calling @b3d::AudioSource::SetClip.

~~~~~~~~~~~~~{.cpp}
HAudioClip audioClip = GetImporter().Import<AudioClip>("myAudioClip.ogg");

audioSource->SetClip(audioClip);
~~~~~~~~~~~~~

Once the clip has been assigned, you can control playback through these methods:
 - @b3d::AudioSource::Play - Starts playing the assigned audio clip. If playback is currently stopped, it starts playback from the clip beginning. If paused, playback is resumed from the point it was paused.
 - @b3d::AudioSource::Stop - Stops playing the assigned audio clip.
 - @b3d::AudioSource::Pause - Pauses playing the assigned audio clip, allowing you to later resume it with a call to **AudioSource::Play()**.

~~~~~~~~~~~~~{.cpp}
audioSource->Play();
~~~~~~~~~~~~~

You can also control playback by enabling or disabling the scene object the component is attached to. Disabling the scene object will stop any playback.

~~~~~~~~~~~~~{.cpp}
// Stops playback on any audio source components attached to the scene object
audioSourceSceneObject->SetActive(false);
~~~~~~~~~~~~~

When enabling a scene object, you can tell the audio source component to automatically start playback of its assigned clip by enabling the option through @b3d::AudioSource::SetPlayOnStart.

~~~~~~~~~~~~~{.cpp}
// When the scene object this component is attached to is enabled, playback will start automatically
// Otherwise you must call Play() manually
audioSource->SetPlayOnStart(true);

// Starts playback
audioSourceSceneObject->SetActive(true);
~~~~~~~~~~~~~

Query current playback state with @b3d::AudioSource::GetState:

~~~~~~~~~~~~~{.cpp}
AudioSourceState state = audioSource->GetState();

if (state == AudioSourceState::Playing)
	B3D_LOG(Info, LogAudio, "Audio is playing");
else if (state == AudioSourceState::Paused)
	B3D_LOG(Info, LogAudio, "Audio is paused");
else if (state == AudioSourceState::Stopped)
	B3D_LOG(Info, LogAudio, "Audio is stopped");
~~~~~~~~~~~~~

This is all you need to play basic audio clips. But let's investigate a few more options that let you control the audio in more detail.

# Volume
Control the volume of the audio source through @b3d::AudioSource::SetVolume. The provided value ranges from 0 to 1.

~~~~~~~~~~~~~{.cpp}
// Set at half volume
audioSource->SetVolume(0.5f);
~~~~~~~~~~~~~

Query current volume:

~~~~~~~~~~~~~{.cpp}
float currentVolume = audioSource->GetVolume();
~~~~~~~~~~~~~

# Pitch
Control pitch through @b3d::AudioSource::SetPitch by providing a pitch multiplier. Values larger than 1 yield a higher pitch, while values smaller than 1 yield a lower pitch.

~~~~~~~~~~~~~{.cpp}
// Increase pitch 100%
audioSource->SetPitch(2.0f);

// Decrease pitch by 50% (from default)
audioSource->SetPitch(0.5f);
~~~~~~~~~~~~~

Query current pitch:

~~~~~~~~~~~~~{.cpp}
float currentPitch = audioSource->GetPitch();
~~~~~~~~~~~~~

# Seeking
Seek to a specific position within the currently assigned audio clip by calling @b3d::AudioSource::SetTime. It accepts a time in seconds. If the clip is currently playing, playback will skip to the provided time. If the clip is currently paused, it will resume from the provided time the next time **AudioSource::Play()** is called.

~~~~~~~~~~~~~{.cpp}
// Seek to 30 seconds
audioSource->SetTime(30.0f);
~~~~~~~~~~~~~

Query current playback time:

~~~~~~~~~~~~~{.cpp}
float currentTime = audioSource->GetTime();
~~~~~~~~~~~~~

# Loop
By default, when playback reaches the end of the current audio clip, playback will end. You can ensure playback loops instead by calling @b3d::AudioSource::SetIsLooping.

~~~~~~~~~~~~~{.cpp}
// When playback reaches the end, loop back to start
audioSource->SetIsLooping(true);
~~~~~~~~~~~~~

Check if looping is enabled:

~~~~~~~~~~~~~{.cpp}
bool isLooping = audioSource->GetIsLooping();
~~~~~~~~~~~~~

# Priority
When more audio sources are playing than supported by the hardware, some might be disabled. By setting a higher priority, the audio source is guaranteed to be disabled after sources with lower priority.

~~~~~~~~~~~~~{.cpp}
// Set high priority
audioSource->SetPriority(100);

// Query current priority
u32 priority = audioSource->GetPriority();
~~~~~~~~~~~~~

# 3D sounds
If an **AudioClip** has been marked as 3D sound (as described in the previous chapter), sound playback will be influenced by the position and velocity of the scene object the **AudioSource** component is attached to. Such sounds will sound differently depending on their distance from the listener (among other properties). This ensures sounds feel realistic as the player walks around the level (sounding quieter when far away, or using surround to project the sound behind the player).

3D sounds only work if there is a listener defined in the scene.

## Listener
Listener provides a reference point used for 3D sound effects. It is represented with an @b3d::AudioListener component. It requires no additional properties aside from being present in the scene.

~~~~~~~~~~~~~{.cpp}
HSceneObject audioListenerSceneObject = SceneObject::Create("Audio listener");
audioListenerSceneObject->AddComponent<AudioListener>();
~~~~~~~~~~~~~

Normally you want to attach this component to a scene object representing your player or the player's camera. The listener's position and orientation are automatically updated based on its scene object's transform.

## Attenuation
Attenuation determines how quickly audio volume drops off as the listener moves further from the audio source. Change it through @b3d::AudioSource::SetAttenuation. Higher values mean the sound attenuates more quickly with distance.

You can further control attenuation by setting @b3d::AudioSource::SetMinDistance. If the listener is closer to the source than the minimum distance, the audio will play at full volume. This ensures the effect of attenuation can be avoided when the listener is close to the source.

~~~~~~~~~~~~~{.cpp}
// Normal attenuation
audioSource->SetAttenuation(1.0f);

// Attenuation starts at distance of 2 units
audioSource->SetMinDistance(2.0f);
~~~~~~~~~~~~~

Query attenuation settings:

~~~~~~~~~~~~~{.cpp}
float attenuation = audioSource->GetAttenuation();
float minDistance = audioSource->GetMinDistance();
~~~~~~~~~~~~~

# Global controls
Audio options can also be controlled globally through the @b3d::Audio system, accessible through @b3d::GetAudio.

Change audio volume globally by calling @b3d::Audio::SetVolume:

~~~~~~~~~~~~~{.cpp}
// Mute all sounds
GetAudio().SetVolume(0.0f);

// Restore volume
GetAudio().SetVolume(1.0f);

// Query current global volume
float globalVolume = GetAudio().GetVolume();
~~~~~~~~~~~~~

Pause or unpause all sounds globally by calling @b3d::Audio::SetPaused:

~~~~~~~~~~~~~{.cpp}
// Pause all sounds
GetAudio().SetPaused(true);

// Resume all sounds
GetAudio().SetPaused(false);

// Check if globally paused
bool isPaused = GetAudio().IsPaused();
~~~~~~~~~~~~~

# Direct clip playback
Sometimes you do not need all the features offered by the **AudioSource** component, and just want to play a sound with no extra options. In that case, you can call @b3d::Audio::Play and provide it with an **AudioClip** to play. The clip will play at the provided volume and position. It will stop once it ends (no looping), and you won't have any control over its playback once it starts.

~~~~~~~~~~~~~{.cpp}
// Play at default position and volume
GetAudio().Play(audioClip);

// Play at specific position with custom volume
GetAudio().Play(audioClip, Vector3(10.0f, 0.0f, 5.0f), 0.8f);
~~~~~~~~~~~~~

# Device enumeration and switching
If the user has multiple audio devices, you can use the **Audio** system to enumerate through them and switch which device you wish to output to.

To retrieve a list of all devices, call @b3d::Audio::GetAllDevices. This will return a set of @b3d::AudioDevice objects, which contain unique names for all the output devices.

To switch the active device, call @b3d::Audio::SetActiveDevice.

To retrieve the default audio device (the one the user selected in the operating system), call @b3d::Audio::GetDefaultDevice.

~~~~~~~~~~~~~{.cpp}
// Enumerate devices and choose the second one available if present
const Vector<AudioDevice>& devices = GetAudio().GetAllDevices();
if (devices.size() > 1)
	GetAudio().SetActiveDevice(devices[1]);

// Get current active device
AudioDevice activeDevice = GetAudio().GetActiveDevice();
B3D_LOG(Info, LogAudio, "Active device: {0}", activeDevice.Name);

// Get default device
AudioDevice defaultDevice = GetAudio().GetDefaultDevice();
B3D_LOG(Info, LogAudio, "Default device: {0}", defaultDevice.Name);
~~~~~~~~~~~~~
