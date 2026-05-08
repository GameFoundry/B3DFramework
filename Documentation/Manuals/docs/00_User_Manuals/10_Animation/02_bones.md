---
title: Bones
---
The @b3d::Bone component allows you to expose a bone in a skeleton used for skeletal animation, for external use. A **SceneObject** with such a component attached will mimic the transformations the bone undergoes during animation. This allows you to:
 - Attach other scene objects as children of the bone, ensuring you can animate other objects depending on the skeletal animation (e.g. a character holding a sword would have the sword attached to the hand bone, assuming the sword is modeled as a separate object)
 - Manipulate the bone's transform manually, ensuring you can control the animation manually (e.g. for purposes of inverse kinematics)

A bone component must always be added on a scene object that is a child of a scene object containing the **Animation** component.

~~~~~~~~~~~~~{.cpp}
HSceneObject animatedRenderableSceneObject = SceneObject::Create("Animated 3D object");
// Set up renderable...
HAnimation animation = animatedRenderableSceneObject->AddComponent<Animation>();

HSceneObject boneSceneObject = SceneObject::Create("Bone");
boneSceneObject->SetParent(animatedRenderableSceneObject);

HBone bone = boneSceneObject->AddComponent<Bone>();
~~~~~~~~~~~~~

After the component has been added to the scene you must specify the name of the bone it will mimic. You can find information about all bones in a specific **Mesh** by calling @b3d::Mesh::GetSkeleton(). This will return a @b3d::Skeleton object which allows you to enumerate all bones.

~~~~~~~~~~~~~{.cpp}
HMesh mesh = ...;

// Assuming our skeleton has a bone named "Hand"
bone->SetBoneName("Hand");

String boneName = bone->GetBoneName();
B3D_LOG(Info, LogGeneric, "Bone name: {0}", boneName);

// Or query the skeleton information and search for the bone you need
// TShared<Skeleton> skeleton = mesh->GetSkeleton();
// ...
~~~~~~~~~~~~~

After it is set-up, you can now attach scene objects as children of the bone, and ensure they animate with animation playback.

~~~~~~~~~~~~~{.cpp}
// Set up a sword to animate with the hand bone
HSceneObject swordSceneObject = SceneObject::Create("Sword");
// Set up renderable...

swordSceneObject->SetParent(boneSceneObject);
~~~~~~~~~~~~~

Or you can manually manipulate the transform of the bone.
~~~~~~~~~~~~~{.cpp}
// Rotate the bone manually
boneSceneObject->SetRotation(Quaternion(Degree(0), Degree(90), Degree(0)));
~~~~~~~~~~~~~
