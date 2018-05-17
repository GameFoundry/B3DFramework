Loading animation clips						{#animationClip}
===============
[TOC]

When it comes to animating 3D objects (meshes), bs::f supports two types of animation:
 - Skeletal - Each vertex on the mesh is assigned to a bone using an index and a weight - this is called a skin. Each bone is part of a hierarchy which is called a skeleton. The bones are then animated using animation clips, and as the bones move, so do the vertices (skin) attached to them. 
 - Morph - Many different versions of the same mesh exist, representing different shapes of the mesh. By interpolating between the shapes animation is produced. This interpolation is also controlled by animation clips.
 
In general it is preferred to use skeletal animation wherever possible, as it has lower memory requirements, allows interesting ways to combine animations and allows other objects to be attached to bones (as we'll show later). Morph animation is preferable for complex animations where bones just don't cut it (e.g. facial animation can sometimes benefit, as well as objects that have very soft structure without a clear way to set up the skeleton).

@ref TODO_IMAGE_SKELETAL_ANIM

@ref TODO_IMAGE_MORPH_ANIM

Both animation types are controlled via @ref bs::AnimationClip "AnimationClip"%s. 

Animation clip consists of a set of animation curves, each animating either a bone or a set of morph shapes. Each animation curve has a set of keyframes (time and value pairs). As animation is playing, time moves forward and the system interpolates between those keyframes and applies the latest available value, which produces the animation. Knowledge of this internal structure is not necessary to play animation, but it will be useful to know when we explain some more advanced features below.

# Import {#animationClip_a}
Animation clips are imported from the same source file that contains the mesh object. The import process is similar to how we imported physics meshes, using **Importer::importAll()** to retrieve the clips.

To enable animation clip import you must set @ref bs::MeshImportOptions::setImportAnimation "MeshImportOptions::setImportAnimation()" to true, import the mesh and retrieve the clip from the resulting resource array. Note that a source file could contain multiple animation clips (depending on how it was created).

~~~~~~~~~~~~~{.cpp}
auto importOptions = MeshImportOptions::create();
importOptions->setImportAnimation(true);

auto resources = gImporter().importAll("humanAnimated.fbx", importOptions);
HAnimationClip animationClip = static_resource_cast<AnimationClip>(resources[1].value);
~~~~~~~~~~~~~

We'll show how to use animation clips in the following chapter, while in this one we'll just focus on importing them.

Aside from the animation clip you will also need to import additional data for the mesh itself. Depending on whether you're using skeletal or morph animation (or both), you need to enable @ref bs::MeshImportOptions::setImportSkin "MeshImportOptions::setImportSkin()" and/or @ref bs::MeshImportOptions::setImportBlendShapes "MeshImportOptions::setImportBlendShapes()". This will import required additional per-vertex information, skeleton & bone information, as well as morph shapes.

~~~~~~~~~~~~~{.cpp}
importOptions->setImportSkin(true); // For skeletal animation
importOptions->setImportBlendShapes(true); // For morph shapes
~~~~~~~~~~~~~

> These options add additional information to **Mesh**, accessible through @ref bs::Mesh::getSkeleton "Mesh::getSkeleton()" and @ref bs::Mesh::getMorphShapes "Mesh::getMorphShapes()". This information will automatically be used by the animation system (you don't need to do anything but enable these flags on import), but can be accessed by the user if required for some special purpose.

# Animation properties {#animationClip_b}
Once imported the animation clip provides a few properties you can query. Use @ref bs::AnimationClip::getLength "AnimationClip::getLength()" to get the duration of the animation in seconds. Use @ref bs::AnimationClip::getCurves "AnimationClip::getCurves()" to retrieve all animation curves that are a part of the animation.

These aren't properties you need to access manually for normal animation playback, but it can be useful to know they're there. You can also manually assign custom animation curves by calling @ref bs::AnimationClip::setCurves "AnimationClip::setCurves()". This allows you to create fully customized animations within the engine. Creation of animation curves is out of the scope of this manual but you can look up documentation for @ref bs::AnimationCurves "AnimationCurves" and related classes for more information.

# Advanced {#animationClip_c}
## Splitting {#animationClip_c_a}
Often the creator of the animation will place several animations into the same set of animation curves, one playing after another. When imported in bs::f this will result in a single continous animation clip. This is not useful and in such cases you can break up the animation into multiple clips by populating a set of @ref bs::AnimationSplitInfo "AnimationSplitInfo" structures, and providing them to @ref bs::MeshImportOptions::setAnimationClipSplits "MeshImportOptions::setAnimationClipSplits()".

Each of **AnimationSplitInfo** entries requires the starting and ending frame of the animation, as well as a name to make it easier to identify. Starting/ending frames are something you must receive from the animation creator, or guess from animation playback.

~~~~~~~~~~~~~{.cpp}
Vector<AnimationSplitInfo> splitInfos(3);
splitInfos[0].name = "Walk";
splitInfos[0].startFrame = 0;
splitInfos[0].endFrame = 120;

splitInfos[1].name = "Run";
splitInfos[1].startFrame = 120;
splitInfos[1].endFrame = 240;

splitInfos[2].name = "Wave";
splitInfos[2].startFrame = 240;
splitInfos[2].endFrame = 300;

importOptions->setAnimationClipSplits(splitInfos);
auto resources = gImporter().importAll("humanAnimated.fbx", importOptions);

HAnimationClip walkClip, runClip, waveClip;
for(auto& entry : resource)
{
	if(entry.name == "Walk")
		walkClip = entry.value;
	else if(entry.name == "Run")
		runClip = entry.value;
	else if(entry.name == "Wave")
		waveClip = entry.value;
}
~~~~~~~~~~~~~

## Keyframe reduction {#animationClip_c_b}
Tools that create animation will often output a large set of animation keyframes, usually at a fixed rate (e.g. 60 per second). In most cases this amount of keyframes is not necessary as many of them are static and change very slowly. Additionally bs::f uses keyframe tangents to better approximate the animation curve, ensuring less keyframes need to be used. 

By enabling @ref bs::MeshImportOptions::setKeyFrameReduction "MeshImportOptions::setKeyFrameReduction()" you can ensure that bs::f eliminates any keyframes it deems unnecessary. This can greately reduce the memory usage of animation clips, but might yield animation clips that don't look exactly as imagined by the creator. In most cases you should enable this unless you notice problems.

~~~~~~~~~~~~~{.cpp}
importOptions->setKeyFrameReduction(true);
~~~~~~~~~~~~~

## Animation events {#animationClip_c_c}
Often while animation is playing you might want to be notified when a certain point in an animation is reached. For example in a walk animation we might want a notification when the character's foot touches the ground, so we can play the footstep sound. These kind of notifications can be accomplished by using animation events.

They are extremely simple, represented by @ref bs::AnimationEvent "AnimationEvent" structure, which consists of a name and a time. You can register animation events by calling @ref bs::AnimationClip::setEvents "AnimationClip::setEvents()".

~~~~~~~~~~~~~{.cpp}
Vector<AnimationEvent> events(2);
events[0].name = "FootstepLeft";
events[0].time = 0.5f;
events[1].name = "FootstepRight";
events[1].time = 1.5f;

walkClip->setEvents(events);
~~~~~~~~~~~~~

We'll show how to receive event notifications in the following chapter.

## Root motion {#animationClip_c_d}
Root motion represents an animation curve used for movement of the root bone in skeletal animation. This animation curve is useful for characters and objects that are animated and also move through the world in a dynamic way (like a user controller character or an AI controlled entity).

By enabling root motion you ensure that the animation clip's root motion curve is removed, and you instead get access to the relevant curve directly. You can then use this curve to move the entity through the world. This results in motion that is more realistic and more in line with the animation, than if you just moved the entity in some linear fashion (e.g. a walking character would appear to be gliding, rather than actually walking, since humans and animals don't move at a constant pace).

Enable root motion import by calling @ref bs::MeshImportOptions::setImportRootMotion "MeshImportOptions::setImportRootMotion()". Then retrieve the root motion curve through @ref bs::AnimationClip::getRootMotion "AnimationClip::getRootMotion()". This will return a @ref bs::RootMotion "RootMotion" object, which contains two animation curves, one for rotation, other for translation.

Animation curves are represented through the @ref bs::TAnimationCurve<T> "TAnimationCurve<T>" object. You can evaluate the curve by calling @ref bs::TAnimationCurve<T>::evaluate "TAnimationCurve<T>::evaluate()" with a specific time value.

~~~~~~~~~~~~~{.cpp}
// Import animation with root motion
importOptions->setImportRootMotion(true);

auto resources = gImporter().importAll("humanAnimated.fbx", importOptions);
HAnimationClip animationClip = static_resource_cast<AnimationClip>(resources[1].value);

SPtr<RootMotion> rootMotion = animationClip->getRootMotion();

// Move a character using root motion
Vector3 characterPosition(BsZero);

// Get initial root translation
Vector3 lastRootMotion = rootMotion->translation->evaluate(0.0f);
float time = 0.0f;

// While W key is pressed move character forward
if(gInput().isButtonHeld(BC_W))
{
	// Find out how much the animation changed between this and previous frame
	Vector3 curRootMotion = rootMotion->translation->evaluate(time);
	Vector3 movementDiff = curRootMotion - lastRootMotion;
	
	// Apply that change to actual movement
	characterPosition += movementDiff;
	
	time += gTime().frameDelta();
	lastRootMotion = curRootMotion;
}
~~~~~~~~~~~~~

## Additive animation {#animationClip_c_e}
Additive animation is a special type of skeletal animation that is not meant to be played on its own. Instead it is animation that is meant to be played on top of another animation clip. For example if you had a walk/run/crawl animation for your character, and then wanted to add a wave animation to the game. Instead of creating separate versions of walk/run/crawl animations, you can instead create an additive wave animation. Such additive animation can then play on top of any other compatible animations, without having to modify any existing animations.

Note that artist creating such animation needs to pay attention to two things:
 - Bones not part of the additive animation should not have any animation curves, or if they do they should all have the same value.
 - All animation curves animating the bone should have the same or similar 0th keyframe. This keyframe is used as a baseline and if they're too different added animations can make final animation look weird.
 
Additive animations can only be enabled through **AnimationSplitInfo** info, by setting **AnimationSplitInfo::isAdditive** to true.

~~~~~~~~~~~~~{.cpp}
Vector<AnimationSplitInfo> splitInfo(1);
splitInfo[0].name = "Walk";
splitInfo[0].startFrame = 0;
splitInfo[0].endFrame = 120;
splitInfo[0].isAdditive = true;

importOptions->setAnimationClipSplits(splitInfo);
~~~~~~~~~~~~~