---
title: Indirect lighting
---

When you set up a **Light** component, surfaces lit by that light will only be lit if the surface is directly in the light path. But in real world the light bounces off surfaces, providing lighting to surfaces that are not in a direct path to the light. Indirect lighting provides an additional way to add realism to your scene by accounting for that non-direct lighting.

Indirect lighting needs to be enabled through **RenderSettings::EnableIndirectLighting** for the relevant **Camera**, as it is disabled by default.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
SPtr<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable indirect lighting
renderSettings->EnableIndirectLighting = true;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

# Sky lighting
If you have set up a **Skybox** and enabled indirect lighting your scene will immediately receive indirect lighting from the skybox, no additional settings are required.

Note that you might want to tweak the HDR texture used by the skybox so it doesn't overpower normal lighting. This can happen because some cameras might record HDR values in non-physical units in which case sky might appear too bright or too dark, in which case analytical lights might appear over- or underpowered.

# Light probes
When it comes to indoors you must follow a similar approach as with reflection probes. Again you don't want indoors to receive indirect lighting from the sky, and must therefore set up an additional component. This is done through @b3d::LightProbeVolume.

~~~~~~~~~~~~~{.cpp}
// Set up a light probe volume
HSceneObject lightProbeVolumeSceneObject = SceneObject::Create("LightProbeVolume");
HLightProbeVolume lightProbeVolume = lightProbeVolumeSceneObject->AddComponent<LightProbeVolume>();
~~~~~~~~~~~~~

**LightProbeVolume** allows you to set up light probes over the scene. The light probes will record lighting information at their position, and nearby surfaces will then use that information for indirect lighting. If a camera is outside of a light probe volume it will fall back to sky lighting.

## Placing probes
You generally want to place light probes wherever there is a major change in lighting. The probes are fairly cheap performance-wise, and dozens can be used per a single room.

To add a probe to the volume call @b3d::LightProbeVolume::AddProbe. The method only requires you to provide a position at which to place the probe at. The position is relative to the **SceneObject** the volume is attached to.

~~~~~~~~~~~~~{.cpp}
HLightProbeVolume lightProbeVolume = ...;

// Register a couple of probes
u32 probe1Handle = lightProbeVolume->AddProbe(Vector3(0.0f, 1.0f, 0.0f));
u32 probe2Handle = lightProbeVolume->AddProbe(Vector3(5.0f, 1.0f, 0.0f));
~~~~~~~~~~~~~

The method returns a handle that can be used to identify and manipulate the probe later.

~~~~~~~~~~~~~{.cpp}
HLightProbeVolume lightProbeVolume = ...;
u32 probeHandle = lightProbeVolume->AddProbe(Vector3(0.0f, 1.0f, 0.0f));

// Move the probe to a new position
lightProbeVolume->SetProbePosition(probeHandle, Vector3(0.0f, 2.0f, 0.0f));

// Get the current probe position
Vector3 probePosition = lightProbeVolume->GetProbePosition(probeHandle);
B3D_LOG(Info, LogRenderer, "Probe position: {0}", probePosition);

// Remove the probe
lightProbeVolume->RemoveProbe(probeHandle);
~~~~~~~~~~~~~

You can also retrieve information about all probes in the volume using @b3d::LightProbeVolume::GetProbes.

~~~~~~~~~~~~~{.cpp}
HLightProbeVolume lightProbeVolume = ...;

// Get information about all probes
Vector<LightProbeInfo> probes = lightProbeVolume->GetProbes();

for (const LightProbeInfo& probeInfo : probes)
{
    B3D_LOG(Info, LogRenderer, "Probe handle: {0}, Position: {1}",
        probeInfo.Handle, probeInfo.Position);
}
~~~~~~~~~~~~~

## Generic probe positioning
Note that when first created the volume will contain eight probes placed on the corners of a unit box, at the volume's location. You can call @b3d::LightProbeVolume::Resize() to change the size of the box for the eight probes. You can also increase the density in which case the probes will also be placed in-between the box corners, as a uniform grid. This is particularily useful if you do not feel like placing probes manually.

~~~~~~~~~~~~~{.cpp}
HLightProbeVolume lightProbeVolume = ...;

// Set up a probe volume using a uniform grid distribution of probes with a total of 50 probes
AABox area(Vector3(-5, -5, -5), Vector3(5, 5, 5));
Vector3I probeCount(5, 2, 5);

lightProbeVolume->Resize(area, probeCount);
~~~~~~~~~~~~~

You can query the current grid volume and cell count using the getter methods.

~~~~~~~~~~~~~{.cpp}
HLightProbeVolume lightProbeVolume = ...;

// Get the current grid volume
AABox gridVolume = lightProbeVolume->GetGridVolume();
B3D_LOG(Info, LogRenderer, "Grid volume: {0}", gridVolume);

// Get the current cell count
Vector3I cellCount = lightProbeVolume->GetCellCount();
B3D_LOG(Info, LogRenderer, "Cell count: {0}", cellCount);
~~~~~~~~~~~~~

If you want to remove any probes outside of the current grid volume, call @b3d::LightProbeVolume::Clip().

~~~~~~~~~~~~~{.cpp}
HLightProbeVolume lightProbeVolume = ...;

// Remove probes outside the grid volume
lightProbeVolume->Clip();
~~~~~~~~~~~~~

To reset all probes to match the original grid pattern, call @b3d::LightProbeVolume::Reset(). This will reset probe positions, as well as add/remove probes as necessary, essentially losing any custom changes to the probes.

~~~~~~~~~~~~~{.cpp}
HLightProbeVolume lightProbeVolume = ...;

// Reset all probes to the grid pattern
lightProbeVolume->Reset();
~~~~~~~~~~~~~

## Rendering probes
Once you have positioned the probes, you need to render them by calling @b3d::LightProbeVolume::RenderProbes. This will update the lighting information for all probes in a volume. You will want to do this any time you add/remove or move probes, or when the lighting environment changes.

~~~~~~~~~~~~~{.cpp}
HLightProbeVolume lightProbeVolume = ...;

// Update probes based on current scene
lightProbeVolume->RenderProbes();
~~~~~~~~~~~~~

The rendered probes will be saved with the component so you do not need to render them after scene load.

If you want to update just a single probe, you can call @b3d::LightProbeVolume::RenderProbe with the probe handle.

~~~~~~~~~~~~~{.cpp}
HLightProbeVolume lightProbeVolume = ...;
u32 probeHandle = ...;

// Update a specific probe
lightProbeVolume->RenderProbe(probeHandle);
~~~~~~~~~~~~~
