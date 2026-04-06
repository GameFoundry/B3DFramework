---
title: Advanced animation
---

So far we have shown how you can use the **Animation** component to play a single animation clip at once. When it comes to skeletal animation the system is capable of a few more advanced features that allow you to play and blend multiple animation clips at once, as well as other advanced functionality.

# Cross fade
Often your animation system might need to transition from playing one animation clip to another (e.g. moving from a walk to a run animation). In that case you can use @b3d::Animation::CrossFade instead of **Animation::Play()**. Cross fade will slowly fade in the new animation clip while fading out the previously playing one, over the specified time period.

~~~~~~~~~~~~~{.cpp}
HAnimation animation = ...; // Set up animation component

// Play some clip normally
animation->Play(animationClip);

// Then after some time you decide to switch to a new clip, and cross fade it over 1 second
animation->CrossFade(animationClip2, 1.0f);
~~~~~~~~~~~~~

# Sampling
Sometimes it is useful to sample just a single frame from an animation clip (e.g. "freezing" an enemy in place). For such purpose you can use @b3d::Animation::Sample. Just provide the animation clip and the time to sample at.

~~~~~~~~~~~~~{.cpp}
// Sample animation clip at specified point
animation->Sample(animationClip, 2.0f);
~~~~~~~~~~~~~

# Blending
Blending comes in multiple various forms, and all of them allow you to somehow play multiple animation clips at once.

## 1D blending
In this form of blending multiple animation clips are placed linearly one after another. The user can then use a weight value in order to interpolate between the animations. The two animations closest to the weight value are choosen and blended together.

For example if you had different versions of walk/run animations that alternate depending on how fast the character is moving, you could set them up as a 1D blend, and then use the characters speed as the weight, ensuring the system plays the right animation depending on the speed.

To start a 1D blend operation call @b3d::Animation::Blend1D. This method expects a @b3d::Blend1DInfo structure, containing a list of animations to blend between, and their positions. It also expects the weight value that determines which animations are blended together.

~~~~~~~~~~~~~{.cpp}
// Set up a 1D blend that blends between three animations on a [0, 1] range
Blend1DInfo blendInfo;
blendInfo.Clips.resize(3);
blendInfo.Clips[0].Clip = animationClip0;
blendInfo.Clips[0].Position = 0.0f;

blendInfo.Clips[1].Clip = animationClip1;
blendInfo.Clips[1].Position = 0.5f;

blendInfo.Clips[2].Clip = animationClip2;
blendInfo.Clips[2].Position = 1.0f;

// Blend with weight of 0.75f. This will play both animationClip1 and animationClip2, each contributing 50% of the animation
animation->Blend1D(blendInfo, 0.75f);
~~~~~~~~~~~~~

## 2D blending
2D blending works similarly as 1D blending, except it uses a two-dimensional value for the weight. It also interpolates between four animation clips at once instead of two. It is limited to four clips, and a [0, 1] range. Therefore the clips can be imagined as being on corners of a square.

For example you can use this blend type to interpolate between character aiming animations in a third person game. You would adjust the 2D weight based on the vertical and horizontal directions the character is aiming and the system would interpolate between look up/down/left/right animations to animate the character in the direction the user is aiming.

To start a 2D blend operation call @b3d::Animation::Blend2D. This method expects a @b3d::Blend2DInfo structure, containing four animation clips to blend between, as well as a 2D weight determining how the animations are blended together.

~~~~~~~~~~~~~{.cpp}
// In this case you can imagine the blend square as a diamond instead (rotated 45 degrees), so its corners map with the animation directions, for a more intuitive understanding
Blend2DInfo blendInfo;
blendInfo.TopLeftClip = lookLeftClip;
blendInfo.TopRightClip = lookUpClip;
blendInfo.BottomLeftClip = lookDownClip;
blendInfo.BottomRightClip = lookRightClip;

// Horizontal look direction is fully towards the left (x = 0.0f), while the vertical direction is in the middle (y = 0.5f), neither up nor down.
animation->Blend2D(blendInfo, Vector2(0.0f, 0.5f));
~~~~~~~~~~~~~

## Additive blending
Additive blending serves a different purpose than from what we have seen so far. It is used to play a special type of animations called additive animations. Such animation clips need to be imported with the special additive property, as discussed earlier.

Such animation clips can then be played on top of another primary animation. For example imagine that your character has three primary animations: standing, walking and running. Now imagine you wanted to add a weapon reload animation in the game. Instead of creating three new separate animations, you can instead create an additive animation, which can then be played on top of any of the existing three primary animations.

To play an additive animation call @b3d::Animation::BlendAdditive. This method expects the following parameters:
 - A clip to play
 - Optional weight in [0, 1] range of how much should the animation affect the result
 - Optional fade length which serves a similar purpose as the **Animation::CrossFade()** method, allowing you to slowly increase the effect of the additive animation over time
 - Layer to play the animation on. Each additive animation can be played on its own layer - playing another additive animation on the same layer replaces the previous one.

~~~~~~~~~~~~~{.cpp}
// Play some clip normally
animation->Play(walkClip);

// Play a reload animation additively, slowly blending it over one second, on 0th layer
animation->BlendAdditive(reloadClip, 1.0f, 1.0f, 0);
~~~~~~~~~~~~~

To stop an additive animation you can call @b3d::Animation::Stop(). It accepts a layer index as a parameter and determines which clip to stop. Calling **Animation::StopAll()** will stop all clips, additive or otherwise.

# Animation states
**Animation** component allows you to retrieve and set animation states. Animation states tell you the current state of an animation clip queued for playing in some way, including whether it is currently playing, current play time, wrap mode and more.

Call @b3d::Animation::GetState with an animation clip as the parameter to retrieve information about that clip. The method outputs an @b3d::AnimationClipState that can be used to query various information about the clip.

~~~~~~~~~~~~~{.cpp}
AnimationClipState state;

// Returns true if the clip was queued for playback (in any way) on the component
if(animation->GetState(animationClip, state))
{
	B3D_LOG(Info, LogGeneric, "Animation is currently at time: {0}", state.Time);
	B3D_LOG(Info, LogGeneric, "Animation speed: {0}", state.Speed);
	B3D_LOG(Info, LogGeneric, "Animation weight: {0}", state.Weight);
}
~~~~~~~~~~~~~

More important that querying information, states can be used for starting playback, as well as having full control over blending and similar operations. Essentially all animation playback methods discussed so far can be implemented just by using states.

To set a clip state call @b3d::Animation::SetState. If a particular clip is already playing its state will be updated, and if the clip isn't currently playing, playback will start according to the assigned state.

The state has the following relevant properties:
 - @b3d::AnimationClipState::Time - Allows you to seek animation playback to a specific point.
 - @b3d::AnimationClipState::Speed - Scale to speed up or slow down animation clip playback.
 - @b3d::AnimationClipState::Weight - Influence of the animation clip on the final pose. Useful for additive animations primarily.
 - @b3d::AnimationClipState::WrapMode - Wrap mode same as **Animation::SetWrapMode()** except it can be set for each clip individually.
 - @b3d::AnimationClipState::Layer - Layer used for additive animations.
 - @b3d::AnimationClipState::Stopped - When true the system will not advance the animation past the specified time.

~~~~~~~~~~~~~{.cpp}
AnimationClipState state;
state.Time = 2.0f;

// Start animation playback from 2 second mark
animation->SetState(animationClip, state);
~~~~~~~~~~~~~
