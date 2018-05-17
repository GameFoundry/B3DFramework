Reflection environment						{#reflectionProbes}
===============
[TOC]

Setting up a valid reflection environment is essential for all types of physically based materials. The environment ensures that the specular reflections on the materials correctly reflect the surroundings. 

@ref TODO_IMAGE

A **Skybox** is one such example of a reflection environment. When one is present all materials will reflect the image displayed by the skybox. This is generally fine for open outdoor areas, but when the camera is indoors you don't want the indoor surfaces to reflect the sky. This is where @ref bs::CReflectionProbe "ReflectionProbe" component comes into play.

# Reflection probes {#reflectionProbes_a}
While the skybox is used to provide outdoor reflections, reflection probes are used to create reflection cubemaps for indoor environments. Reflection probes have an origin and a radius of influence. Reflection probes also use HDR cubemaps, but instead of using external textures those cubemaps are generated in-engine, at the position of the reflection probe. They are represented using the **ReflectionProbe** component.

~~~~~~~~~~~~~{.cpp}
HSceneObject reflProbeSO = SceneObject::create("Refl. probe");
HReflectionProbe reflProbe = reflProbeSO->addComponent<CReflectionProbe>();
~~~~~~~~~~~~~

You must provide the extents of the geometry covered by the reflection probe. These extents serve both to determine a range of influence, and to approximate the surrounding geometry. For example if you are placing a reflection probe that covers a room, you should strive to match the reflection probe extents with the room walls. In practice you'll want to tweak it to what looks best.

You can assign extents in two ways, depending on reflection probe type:
 - @ref bs::ReflectionProbeType::Box "ReflectionProbeType::Box" - Reflection probe is represented by a box and extents are set by calling @ref bs::CReflectionProbe::setExtents "CReflectionProbe::setExtents()".
 - @ref bs::ReflectionProbeType::Sphere "ReflectionProbeType::Sphere" - Reflection probe is represented by a sphere an extents are set by calling @ref bs::CReflectionProbe::setRadius "CReflectionProbe::setRadius()".
 
You can change the type of the reflection probe (and therefore extents) by calling @ref bs::CReflectionProbe::setType "CReflectionProbe::setType()".

~~~~~~~~~~~~~{.cpp}
reflProbe->setType(ReflectionProbeType::Box);
reflProbe->setExtents(Vector3(2.0f, 2.0f, 2.0f));
~~~~~~~~~~~~~

## Generating reflection probes {#reflectionProbes_a_a}
Reflection probe cubemap will be generated automatically when the reflection probe is first added to the scene, and whenever it is moved. You can also force the cubemap to regenerate by calling @ref bs::CReflectionProbe::capture() "CReflectionProbe::capture()". This is required when surrounding geometry changes and you wish to update the probe cubemap.

~~~~~~~~~~~~~{.cpp}
reflProbe->capture();
~~~~~~~~~~~~~

## Using external textures {#reflectionProbes_a_b}
In case you want to use an external HDR texture, similar to a skybox, you can call @ref bs::CReflectionProbe::setCustomTexture "CReflectionProbe::setCustomTexture()". The system will no longer use the automatically generated cubemap and use the provided one instead. If you wish to switch back to the automatic generator, call the method with a null value.

~~~~~~~~~~~~~{.cpp}
HTexture myCubemap = ...;

reflProbe->setCustomTexture(myCubemap);
~~~~~~~~~~~~~

## Reflection probe interpolation {#reflectionProbes_a_c}
When multiple reflection probes overlap the system will blend between the reflection probes based on the distance from the origin and the probe extents. If system can't blend with other reflection probes it will instead blend with the sky. This means in most cases you want to ensure that reflection probes overlap, in order to provide clean transitions. When the camera is outside the influence of any reflection probes the sky reflections will be used instead.