---
title: Reflection environment
---

Setting up a valid reflection environment is essential for all types of physically based materials. The environment ensures that the specular reflections on the materials correctly reflect the surroundings.

[TODO_IMAGE]()

A **Skybox** is one such example of a reflection environment. When one is present all materials will reflect the image displayed by the skybox. This is generally fine for open outdoor areas, but when the camera is indoors you don't want the indoor surfaces to reflect the sky. This is where @b3d::ReflectionProbe component comes into play.

# Reflection probes
While the skybox is used to provide outdoor reflections, reflection probes are used to create reflection cubemaps for indoor environments. Reflection probes have an origin and a radius of influence. Reflection probes also use HDR cubemaps, but instead of using external textures those cubemaps are generated in-engine, at the position of the reflection probe. They are represented using the **ReflectionProbe** component.

~~~~~~~~~~~~~{.cpp}
HSceneObject reflectionProbeSceneObject = SceneObject::Create("Refl. probe");
HReflectionProbe reflectionProbe = reflectionProbeSceneObject->AddComponent<ReflectionProbe>();
~~~~~~~~~~~~~

You must provide the extents of the geometry covered by the reflection probe. These extents serve both to determine a range of influence, and to approximate the surrounding geometry. For example if you are placing a reflection probe that covers a room, you should strive to match the reflection probe extents with the room walls. In practice you'll want to tweak it to what looks best.

You can assign extents in two ways, depending on reflection probe type:
 - @b3d::ReflectionProbeType::Box - Reflection probe is represented by a box and extents are set by calling @b3d::ReflectionProbe::SetExtents.
 - @b3d::ReflectionProbeType::Sphere - Reflection probe is represented by a sphere an extents are set by calling @b3d::ReflectionProbe::SetRadius.

You can change the type of the reflection probe (and therefore extents) by calling @b3d::ReflectionProbe::SetType.

~~~~~~~~~~~~~{.cpp}
HReflectionProbe reflectionProbe = ...;

reflectionProbe->SetType(ReflectionProbeType::Box);
reflectionProbe->SetExtents(Vector3(2.0f, 2.0f, 2.0f));
~~~~~~~~~~~~~

You can also query the current probe type and extents using the corresponding getter methods.

~~~~~~~~~~~~~{.cpp}
HReflectionProbe reflectionProbe = ...;

// Get the current probe type
ReflectionProbeType probeType = reflectionProbe->GetType();

// Get extents based on the probe type
if (probeType == ReflectionProbeType::Box)
{
    Vector3 extents = reflectionProbe->GetExtents();
    B3D_LOG(Info, LogRenderer, "Box probe extents: {0}", extents);
}
else if (probeType == ReflectionProbeType::Sphere)
{
    float radius = reflectionProbe->GetRadius();
    B3D_LOG(Info, LogRenderer, "Sphere probe radius: {0}", radius);
}
~~~~~~~~~~~~~

## Generating reflection probes
Reflection probe cubemap will be generated automatically when the reflection probe is first added to the scene, and whenever it is moved. You can also force the cubemap to regenerate by calling @b3d::ReflectionProbe::Capture(). This is required when surrounding geometry changes and you wish to update the probe cubemap.

~~~~~~~~~~~~~{.cpp}
HReflectionProbe reflectionProbe = ...;

// Capture the scene at the current probe location
reflectionProbe->Capture();
~~~~~~~~~~~~~

## Using external textures
In case you want to use an external HDR texture, similar to a skybox, you can call @b3d::ReflectionProbe::SetCustomTexture. The system will no longer use the automatically generated cubemap and use the provided one instead. If you wish to switch back to the automatic generator, call the method with a null value.

~~~~~~~~~~~~~{.cpp}
HReflectionProbe reflectionProbe = ...;

// Set a custom cubemap texture
HTexture customCubemap = GetImporter().Import<Texture>("MyCustomCubemap.hdr", textureImportOptions);
reflectionProbe->SetCustomTexture(customCubemap);
~~~~~~~~~~~~~

To switch back to automatic generation, pass a null texture:

~~~~~~~~~~~~~{.cpp}
HReflectionProbe reflectionProbe = ...;

// Re-enable automatic cubemap generation
reflectionProbe->SetCustomTexture(HTexture());
~~~~~~~~~~~~~

You can query the currently assigned custom texture using @b3d::ReflectionProbe::GetCustomTexture.

~~~~~~~~~~~~~{.cpp}
HReflectionProbe reflectionProbe = ...;

// Get the current custom texture
HTexture customTexture = reflectionProbe->GetCustomTexture();

if (customTexture)
{
    B3D_LOG(Info, LogRenderer, "Using custom reflection texture");
}
else
{
    B3D_LOG(Info, LogRenderer, "Using automatic reflection capture");
}
~~~~~~~~~~~~~

## Reflection probe interpolation
When multiple reflection probes overlap the system will blend between the reflection probes based on the distance from the origin and the probe extents. If system can't blend with other reflection probes it will instead blend with the sky. This means in most cases you want to ensure that reflection probes overlap, in order to provide clean transitions. When the camera is outside the influence of any reflection probes the sky reflections will be used instead.

# ECS fragments

The **ReflectionProbe** component stores its data internally as ECS fragments, enabling the renderer to batch-process all reflection probes in the scene efficiently.

## Data fragment

The primary fragment is @b3d::ecs::ReflectionProbe, which stores the probe's visual properties:
 - **Type** - Probe type (box or sphere)
 - **Radius** - Radius for sphere reflection probes
 - **Extents** - Extents for box reflection probes
 - **TransitionDistance** - Extra distance used for fading out box probes at the edges
 - **Bounds** - World-space bounding sphere of the probe's area of influence
 - **FilteredTexture** - Pre-filtered cubemap texture generated from a custom texture or scene capture

When you call setter methods like @b3d::ReflectionProbe::SetType or @b3d::ReflectionProbe::SetExtents, the component modifies this fragment and marks the entity as dirty for synchronization with the render thread.

## ID fragment

Each reflection probe also has an @b3d::ecs::ReflectionProbeId fragment that stores a persistent renderer ID used by the @b3d::RendererObjectStorage system for mapping to the packed render-thread representation.

## Dirty tags

The renderer uses two ECS tag types to track which reflection probes need synchronization:
 - `ecs::ReflectionProbeDirty` - Added when any probe property changes. Triggers a full sync.
 - `ecs::ReflectionProbeTransformDirty` - Added when only the transform changes.

## Using raw ECS fragments

You can bypass the **ReflectionProbe** component and create `ecs::ReflectionProbe` fragments directly for maximum performance. You must manage the renderer ID, world transform, and dirty tags manually.

~~~~~~~~~~~~~{.cpp}
const SPtr<SceneInstance>& scene = SceneManager::Instance().GetMainScene();
ecs::Registry& registry = scene->GetECSRegistry();
const SPtr<RendererScene>& rendererScene = scene->GetRendererScene();

// Create an entity with the reflection probe data fragment
ecs::Entity entity = registry.CreateEntity();
ecs::ReflectionProbe& fragment = registry.AddComponent<ecs::ReflectionProbe>(entity);
fragment.Type = ReflectionProbeType::Box;
fragment.Extents = Vector3(2.0f, 2.0f, 2.0f);

// Add a world transform — the renderer reads this to position the probe
registry.AddComponent<ecs::WorldTransform>(entity, ecs::WorldTransform(myTransform));

// Allocate a persistent renderer ID
rendererScene->AllocateReflectionProbeId(registry, entity);

// Mark dirty for initial sync
registry.AddTag<ecs::ReflectionProbeDirty>(entity);
~~~~~~~~~~~~~

When modifying the fragment, add the appropriate dirty tag:

~~~~~~~~~~~~~{.cpp}
ecs::ReflectionProbe& fragment = registry.GetComponents<ecs::ReflectionProbe>(entity);
fragment.Extents = Vector3(4.0f, 4.0f, 4.0f);
registry.AddTag<ecs::ReflectionProbeDirty>(entity);

// For transform-only changes
registry.GetComponents<ecs::WorldTransform>(entity) = ecs::WorldTransform(newTransform);
registry.AddTag<ecs::ReflectionProbeTransformDirty>(entity);
~~~~~~~~~~~~~

When destroying the entity, deallocate the renderer ID first:

~~~~~~~~~~~~~{.cpp}
rendererScene->DeallocateReflectionProbeId(registry, entity);
registry.RemoveComponents<ecs::ReflectionProbeDirty>(entity);
registry.RemoveComponents<ecs::ReflectionProbeTransformDirty>(entity);
registry.RemoveComponents<ecs::ReflectionProbe>(entity);
registry.EraseEntity(entity);
~~~~~~~~~~~~~
