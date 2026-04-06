---
title: Other particle system settings
---

We already mentioned @b3d::ParticleSystemSettings when we have shown how to apply a **Material** to the particle system. In this manual we will go over the rest of the options provided by that object and show how they affect the particle system.

To recap, **ParticleSystemSettings** object can be assigned to a particle system by calling @b3d::ParticleSystem::SetSettings.

~~~~~~~~~~~~~{.cpp}
HParticleSystem particleSystem = ...;

// Set the material to be used by the particle system
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.Material = particleMaterial;

particleSystem->SetSettings(particleSystemSettings);
~~~~~~~~~~~~~

Query the current settings:

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings particleSystemSettings = particleSystem->GetSettings();
~~~~~~~~~~~~~

# Duration and looping

Duration of the particle system controls how are time-based curves in various distributions evaluated. This includes all the distributions in **ParticleEmitter** but also other distributions that are evaluated based on system lifetime. When the particle system is first spawned its internal timer starts ticking and current time is incremented. This time is divided by the duration and then passed to relevant curves in [0, 1] range. Use @b3d::ParticleSystemSettings::Duration to set the duration in seconds.

Once the time passes the system duration the time will loop back to the start. If you prefer to instead clamp the time when the duration is reached, disable @b3d::ParticleSystemSettings::IsLooping.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.Duration = 5.0f; // 5 seconds
particleSystemSettings.IsLooping = true;
~~~~~~~~~~~~~

# Simulation space

Simulation space controls in which space are particles spawned. It is set through @b3d::ParticleSystemSettings::SimulationSpace and can have two values:
 - @b3d::ParticleSimulationSpace::Local - Particles are spawned in space local to the particle system. If you move the particle system all the particles will move with it. As an example imagine electrical sparks on a moving vehicle.
 - @b3d::ParticleSimulationSpace::World - Particles are spawned in world space. If you move the particle system any existing particles will keep their original location while any newly spawned particles will spawn at the new location. As an example imagine a flamethrower as the player moves or rotates.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.SimulationSpace = ParticleSimulationSpace::World;
~~~~~~~~~~~~~

# Orientation

This set of options allow you to modify how are particle billboards oriented. It is controlled through the @b3d::ParticleSystemSettings::Orientation field and can take on the following values:
 - @b3d::ParticleOrientation::ViewPlane - Particles will be oriented towards the camera plane, so that their X & Y axes are parallel to the camera plane. All particles have the exact same orientation.
 - @b3d::ParticleOrientation::ViewPosition - Achieves a similar effect as **ParticleOrientation::ViewPlane**, but orients towards the camera's position. Particles will have slightly different orientation depending on their position, but will still face the camera.
 - @b3d::ParticleOrientation::Plane - Particles will always face a certain plane. The plane can be specified by setting @b3d::ParticleSystemSettings::OrientationPlaneNormal. This is particularly useful for 2D games.

You can also forbid particles to rotate around the Y axis by setting @b3d::ParticleSystemSettings::OrientationLockY. This can be useful for particles that must remain "upright", yet still face the camera. As an example imagine grass billboards.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.Orientation = ParticleOrientation::ViewPosition;
particleSystemSettings.OrientationLockY = true;
~~~~~~~~~~~~~

# Render mode

This options allows you to choose should particles render as 2D billboards or as 3D meshes. It can be set through @b3d::ParticleSystemSettings::RenderMode and has two properties:
 - @b3d::ParticleRenderMode::Billboard - Render as 2D billboard.
 - @b3d::ParticleRenderMode::Mesh - Render as a 3D mesh.

When rendering as a 3D mesh you will need to specify the mesh to use in the @b3d::ParticleSystemSettings::Mesh field. You might also want to use a shader that supports lighting, such as **BuiltinShader::ParticlesLitOpaque**.

~~~~~~~~~~~~~{.cpp}
// Set up rendering of 3D particles

// Grab a mesh to use
HMesh mesh = GetBuiltinResources().GetMesh(BuiltinMesh::Sphere);

// Set up a material that supports lighting
HShader particleShader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::ParticlesLitOpaque);
HMaterial particleMaterial = Material::Create(particleShader);

particleMaterial->SetTexture("gAlbedoTex", GetBuiltinResources().GetTexture(BuiltinTexture::White));

// Enable 3D particles
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.Material = particleMaterial;
particleSystemSettings.RenderMode = ParticleRenderMode::Mesh;
particleSystemSettings.Mesh = mesh;
~~~~~~~~~~~~~

![3D particles](../../Images/3dparticles.gif)  

# Sorting

Certain particle shaders might require particles to be sorted in a particular order to work properly. You can control particle sorting through the @b3d::ParticleSystemSettings::SortMode field. It has the following options:
 - @b3d::ParticleSortMode::None - Don't sort the particles.
 - @b3d::ParticleSortMode::Distance - Sort the particles back to front, based on distance from the camera.
 - @b3d::ParticleSortMode::OldToYoung - Sort the particles so youngest are rendered first.
 - @b3d::ParticleSortMode::YoungToOld - Sort the particles so oldest are rendered first.

Note that sorting is a potentially expensive process and should be disabled unless necessary.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.SortMode = ParticleSortMode::Distance;
~~~~~~~~~~~~~
 
# Bounds

Systems like visibility culling require the world bounds of the particle system to be calculated every frame. Bounds can be calculated automatically or bounds can be provided manually. Use @b3d::ParticleSystemSettings::UseAutomaticBounds to toggle automatic bound calculation.

When disabled you can provide custom bounds by setting @b3d::ParticleSystemSettings::CustomBounds. User must ensure these bounds completely cover the potential particle system volume, otherwise it might get culled even when particles are still in view.

Using custom bounds saves the potentially expensive per-frame recalculation of bounds, but can decrease culling efficiency since the bounds might need to be larger than the actual effect at a particular frame.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.UseAutomaticBounds = false;
particleSystemSettings.CustomBounds = AABox::kUnit;
~~~~~~~~~~~~~

# Maximum particle count

@b3d::ParticleSystemSettings::MaxParticles can be used to limit the maximum number of particles. Once this number is reached no new particles will be spawned, until some old particles die and make room.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.MaxParticles = 4000;
~~~~~~~~~~~~~

# Seeding

Emitters and most evolvers use random number generation when spawning/manipulating particles. Seed for those generators is generated automatically when the particle system is first spawned. This behaviour can be disabled by toggling @b3d::ParticleSystemSettings::UseAutomaticSeed. When automatic seed generation is disabled make sure to provide the custom seed in @b3d::ParticleSystemSettings::ManualSeed.

Using a custom seed ensures you can always achieve the same exact particle effect, while using an automatic seed will make your particle effect slightly different every time it is played.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.UseAutomaticSeed = false;
particleSystemSettings.ManualSeed = 123456;
~~~~~~~~~~~~~
