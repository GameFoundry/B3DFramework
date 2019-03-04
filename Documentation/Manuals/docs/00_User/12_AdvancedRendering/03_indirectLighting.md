Indirect lighting				{#indirectLighting}
===============
[TOC]

When you set up a **Light** component, surfaces lit by that light will only be lit if the surface is directly in the light path. But in real world the light bounces off surfaces, providing lighting to surfaces that are not in a direct path to the light. Indirect lighting provides an additional way to add realism to your scene by accounting for that non-direct lighting.

Indirect lighting needs to be enabled through **RenderSettings::enableIndirectLighting** for the relevant **Camera**, as it is disabled by default.

# Sky lighting {#indirectLighting_a}
If you have set up a **Skybox** and enabled indirect lighting your scene will immediately receive indirect lighting from the skybox, no additional settings are required.

Note that you might want to tweak the HDR texture used by the skybox so it doesn't overpower normal lighting. This can happen because some cameras might record HDR values in non-physical units in which case sky might appear too bright or too dark, in which case analytical lights might appear over- or underpowered.

# Light probes {#indirectLighting_b}
When it comes to indoors you must follow a similar approach as with reflection probes. Again you don't want indoors to receive indirect lighting from the sky, and must therefore set up an additional component. This is done through @ref bs::CLightProbeVolume "LightProbeVolume". 

~~~~~~~~~~~~~{.cpp}
// Set up a light probe volume
HSceneObject lightProbeVolumeSO = SceneObject::create("LightProbeVolume");
HLightProbeVolume lightProbeVolume = lightProbeVolumeSO->addComponent<CLightProbeVolume>();
~~~~~~~~~~~~~

**LightProbeVolume** allows you to set up light probes over the scene. The light probes will record lighting information at their position, and nearby surfaces will then use that information for indirect lighting. If a camera is outside of a light probe volume it will fall back to sky lighting. 

## Placing probes {#indirectLighting_b_a}
You generally want to place light probes wherever there is a major change in lighting. The probes are fairly cheap performance-wise, and dozens can be used per a single room. 

To add a probe to the volume call @ref bs::CLightProbeVolume::addProbe "CLightProbeVolume::addProbe()". The method only requires you to provide a position at which to place the probe at. The position is relative to the **SceneObject** the volume is attached to. 

~~~~~~~~~~~~~{.cpp}
// Register a couple of probes
lightProbeVolume->addProbe(Vector3(0.0f, 1.0f, 0.0f)));
lightProbeVolume->addProbe(Vector3(5.0f, 1.0f, 0.0f)));
~~~~~~~~~~~~~

## Generic probe positioning {#indirectLighting_b_b}
Note that when first created the volume will contain eight probes placed on the corners of a unit box, at the volume's location. You can call @ref bs::CLightProbeVolume::resize() "CLightProbeVolume::resize()" to change the size of the box for the eight probes. You can also increase the density in which case the probes will also be placed in-between the box corners, as a uniform grid. This is particularily useful if you do not feel like placing probes manually.

~~~~~~~~~~~~~{.cpp}
// Set up a probe volume using a uniform grid distribution of probes with a total of 50 probes
AABox area(Vector3(-5, -5, -5), Vector3(5, 5, 5));
Vector3I probeCount(5, 2, 5);

lightProbeVolume->resize(area, probeCount);
~~~~~~~~~~~~~

## Rendering probes {#indirectLighting_b_c}
Once you have positioned the probes, you need to render them by calling @ref bs::CLightProbeVolume::renderProbes "CLightProbeVolume::renderProbes()". This will update the lighting information for all probes in a volume. You will want to do this any time you add/remove or move probes, or when the lighting environment changes.

~~~~~~~~~~~~~{.cpp}
// Update probes based on current scene
lightProbeVolume->renderProbes();
~~~~~~~~~~~~~

The renderered probes will be saved with the component so you do not need to render them after scene load.