Playing animation				{#animation}
===============

Animation playback is handled through the @ref bs::CAnimation "Animation" component. This component must be attached to the same **SceneObject** as the **Renderable** of the object we wish to animate.

~~~~~~~~~~~~~{.cpp}
HSceneObject animRenderableSO = SceneObject::create("Animated 3D object");
animRenderableSO->addComponent<CRenderable>();
// ...set up renderable mesh and material...

animation = animRenderableSO->addComponent<CAnimation>();
~~~~~~~~~~~~~

# Playback
To start animation playback simply call @ref bs::CAnimation::play "CAnimation::play()", with the **AnimationClip** you wish to play as the parameter.

~~~~~~~~~~~~~{.cpp}
animation->play(animationClip);
~~~~~~~~~~~~~

To stop playback call @ref bs::CAnimation::stopAll "CAnimation::stopAll()".

~~~~~~~~~~~~~{.cpp}
animation->stopAll();
~~~~~~~~~~~~~

To check if animation is currently playing call @ref bs::CAnimation::isPlaying "CAnimation::isPlaying()".

~~~~~~~~~~~~~{.cpp}
if(animation->isPlaying())
	gDebug().logDebug("Animation is playing!");
~~~~~~~~~~~~~

You can control the speed of the playback through @ref bs::CAnimation::setSpeed "CAnimation::setSpeed()". The default value is 1, which represents normal speed, while lower values slow down playback accordingly (e.g. 0.5 is half speed) and higher values speed it up (2 is double speed). You can change animation speed during or before you start playing a clip.

~~~~~~~~~~~~~{.cpp}
animation->setSpeed(2.0f);
~~~~~~~~~~~~~

Finally, you can decide what happens when the animation clip reaches the end. By calling @ref bs::CAnimation::setWrapMode "CAnimation::setWrapMode()" with the parameter of type @ref bs::AnimWrapMode "AnimWrapMode" you can choose:
 - @ref bs::AnimWrapMode::Clamp "AnimWrapMode::Clamp" - Once the end is reached the animation will stop.
 - @ref bs::AnimWrapMode::Loop "AnimWrapMode::Loop" - Once the end is reached the animation will loop back to the beginning, and continue to do so indefinitely, until manually stopped.
 
~~~~~~~~~~~~~{.cpp}
animation->setWrapMode(AnimWrapMode::Loop);
~~~~~~~~~~~~~

## Play on wake
Aside from starting playback manually, you can also tell the animation component to start playing animation as soon as its component is instantiated. This can be useful for animations permanently part of the game level, so they can start playback as soon as the level is loaded.

To do this call @ref bs::CAnimation::setDefaultClip "CAnimation::setDefaultClip()". The system will then keep a permanent reference to the animation clip, and automatically play it whenever the component is instantiated.

~~~~~~~~~~~~~{.cpp}
animation->setDefaultClip(animationClip);
~~~~~~~~~~~~~

# Morph shapes
Both skeletal and morph shape animation playback is handled as we described above. However with morph shapes there is an additional option to consider, concering morph channels. Morph shape animation consists out of one or multiple *morph channels*, each channel can have one or multiple *morph shapes*. A single shape is simply a mesh in a certain pose (i.e. shape).

For example, a morph animation of a raindrop splashing on the ground would have a single channel, and multiple shapes, each shape representing one shape of the raindrop as it splashes. For such animations no additional tweaking is necessary, as the interpolation between the different shapes is done by the animation clip.

But when a morph animation has multiple channels, you must manually control which channels are visible by calling @ref bs::CAnimation::setMorphChannelWeight "CAnimation::setMorphChannelWeight()". The weight should be in [0, 1] range, where channels with 0 weight with not be visible at all, and ones with weight 1 will be fully visible. Multiple channels with non-zero weights will be added together to form the final animation.

~~~~~~~~~~~~~{.cpp}
// Disable channel 0 and 2, fully enable channel 1
animation->setMorphChannelWeight(0, 0.0f);
animation->setMorphChannelWeight(1, 1.0f);
animation->setMorphChannelWeight(2, 0.0f);
~~~~~~~~~~~~~

Multiple channels are particularily useful for things like facial animation. In such cases you would have one channel for each facial expression, and then modify their weights depending on what is the character feeling or saying. Such channels would also most likely only have a single shape per channel (but don't have to).

You can find out how many channels a morph shape animation has by retrieving a @ref bs::MorphShapes "MorphShapes" object from **Mesh** by calling @ref bs::Mesh::getMorphShapes "Mesh::getMorphShapes()". Use @ref bs::MorphShapes::getNumChannels "MorphShapes::getNumChannels()" to get the channel count, and @ref bs::MorphShapes::getChannel "MorphShapes::getChannel()" to retrieve information about a channel in the form of @ref bs::MorphChannel "MorphChannel".

~~~~~~~~~~~~~{.cpp}
// Assuming we have a mesh imported with morph shapes
SPtr<MorphShapes> morphShapes = mesh->getMorphShapes();

UINT32 numChannels = morphShapes->getNumChannels();
for(UINT32 i = 0; i < numChannels; i++)
{
	SPtr<MorphChannel> channel = morphShapes->getChannel(i);
	gDebug().logDebug("Found morph channel: " + channel->getName());
}
~~~~~~~~~~~~~

# Mesh culling
Normally, when objects are about to be rendered, the system tries to detect if an object is in view of the camera or not. This ensures that the GPU doesn't waste time on objects that are certain not to be visible. For the purposes of culling the system uses an approximation of the mesh to be rendered, in the form of its bounding box. This is efficient because that bounding box can be pre-calculated, stored and then easily checked for visibility.

But with animation the object is constantly transforming, and it is not efficient to re-calculate the bounding box with each new frame of the animation. The system then instead uses the same bounding box it would have used if the object is static. This means that if the animated object's mesh ever leaves the bounds of that box, the system could decide to cull the object, even though some part of it is still visible. This can result in noticeable graphical artifacts, but Banshee provides a way to fix the issue by providing your own bounds.

By providing your own bounding box, you can ensure the box is large enough to cover the entire range of motion of the animation, so that mesh at no point can leave it. This ensures no incorrect culling will happen. To do this you must first enable custom bounds by calling @ref bs::CAnimation::setUseBounds "CAnimation::setUseBounds()", followed by setting the actual bounds though @ref bs::CAnimation::setBounds "CAnimation::setBounds()".

~~~~~~~~~~~~~{.cpp}
animation->setUseBounds(true);

AABox customBounds(Vector3(-1, -1, -1), Vector3(1, 1, 1));
animation->setBounds(customBounds);
~~~~~~~~~~~~~