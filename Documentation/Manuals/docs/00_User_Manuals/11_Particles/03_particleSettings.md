---
title: Other particle system settings
---

We already mentioned @bs::ParticleSystemSettings when we have shown how to apply a **Material** to the particle system. In this manual we will go over the rest of the options provided by that object and show how they affect the particle system. 

To recap, **ParticleSystemSettings** object can be assigned to a particle system by calling @bs::CParticleSystem::setSettings.

~~~~~~~~~~~~~{.cpp}
HParticleSystem particleSystem = ...;

// Set the material to be used by the particle system
ParticleSystemSettings psSettings;
psSettings.material = particleMaterial;

particleSystem->setSettings(psSettings);
~~~~~~~~~~~~~

# Duration and looping

Duration of the particle system controls how are time-based curves in various distributions evaluated. This includes all the distributions in **ParticleEmitter** but also other distributions that are evaluated based on system lifetime. When the particle system is first spawned its internal timer starts ticking and current time is incremented. This time is divided by the duration and then passed to relevant curves in [0, 1] range. Use @bs::ParticleSystemSettings::duration to set the duration in seconds.

Once the time passes the system duration the time will loop back to the start. If you prefer to instead clamp the time when the duration is reached, disable @bs::ParticleSystemSettings::isLooping.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.duration = 5.0f; // 5 seconds
psSettings.isLooping = true;
~~~~~~~~~~~~~

# Simulation space

Simulation space controls in which space are particles spawned. It is set through @bs::ParticleSystemSettings::simulationSpace and can have two values:
 - @bs::ParticleSimulationSpace::Local - Particles are spawned in space local to the particle system. If you move the particle system all the particles will move with it. As an example imagine electrical sparks on a moving vehicle.
 - @bs::ParticleSimulationSpace::World - Particles are spawned in world space. If you move the particle system any existing particles will keep their original location while any newly spawned particles will spawn at the new location. As an example imagine a flamethrower as the player moves or rotates.
 
~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.simulationSpace = ParticleSimulationSpace::World;
~~~~~~~~~~~~~

# Orientation

This set of options allow you to modify how are particle billboards oriented. It is controlled through the @bs::ParticleSystemSettings::orientation field and can take on the following values:
 - @bs::ParticleOrientation::ViewPlane - Particles will be oriented towards the camera plane, so that their X & Y axes are parallel to the camera plane. All particles have the exact same orientation.
 - @bs::ParticleOrientation::ViewPosition - Achieves a similar effect as **ParticleOrientation::ViewPlane**, but orients towards the camera's position. Particles will have slightly different orientation depending on their position, but will still face the camera.
 - @bs::ParticleOrientation::Plane - Particles will always face a certain plane. The plane can be specified by setting @bs::ParticleSystemSettings::orientationPlane. This is particularily useful for 2D games.
 
You can also forbid particles to rotate around the Y axis by setting @bs::ParticleSystemSettings::orientationLockY. This can be useful for particles that must remain "upright", yet still face the camera. As an example imagine grass billboards.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.orientation = ParticleOrientation::ViewPosition;
psSettings.orientationLockY = true;
~~~~~~~~~~~~~

# Render mode

This options allows to to choose should particles render as 2D billboards or as 3D meshes. It can be set through @bs::ParticleSystemSettings::renderMode and has two properties:
 - @bs::ParticleRenderMode::Billboard - Render as 2D billboard.
 - @bs::ParticleRenderMode::Mesh - Render as a 3D mesh.
 
When rendering as a 3D mesh you will need to specify the mesh to use in the @bs::ParticleSystemSettings::mesh field. You might also want to use a shader that supports lighting, such as **BuiltinShader::ParticleLitOpaque**.

~~~~~~~~~~~~~{.cpp}
// Set up rendering of 3D particles

// Grab a mesh to use
HMesh mesh = gBuiltinResources().getMesh(BuiltinMesh::Sphere);

// Set up a material that supports lighting
HShader particleShader = gBuiltinResources().getBuiltinShader(BuiltinShader::ParticlesLitOpaque);
HMaterial particleMaterial = Material::create(particleShader);

particleMaterial->setTexture("gAlbedoTex", gBuiltinResources().getTexture(BuiltinTexture::White));

// Enable 3D particles
ParticleSystemSettings psSettings;
psSettings.material = particleMaterial;
psSettings.renderMode = ParticleRenderMode::Mesh;
psSettings.mesh = mesh;
~~~~~~~~~~~~~

![3D particles](3dparticles.gif)  

# Sorting

Certain particle shaders might require particles to be sorted in a particular order to work properly. You can control particle sorting through the @bs::ParticleSystemSettings::sortMode field. It has the following options:
 - @bs::ParticleSortMode::None - Don't sort the particles.
 - @bs::ParticleSortMode::Distance - Sort the particles back to front, based on distance from the camera.
 - @bs::ParticleSortMode::OldToYoung - Sort the particles so youngest are rendered first.
 - @bs::ParticleSortMode::YoungToOld - Sort the particles so oldest are rendered first.

Note that sorting is a potentially expensive process and should be disabled unless necessary.
 
~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.sortMode = ParticleSortMode::Distance;
~~~~~~~~~~~~~
 
# Bounds

Systems like visibility culling require the world bounds of the particle system to be calculated every frame. Bounds can be calculated automatically or bounds can be provided manually. Use @bs::ParticleSystemSettings::useAutomaticBounds to toggle automatic bound calculation.

When disabled you can provide custom bounds by setting @bs::ParticleSystemSettings::customBounds. User must ensure these bounds completely cover the potential particle system volume, otherwise is might get culled even when particles are still in view.

Using custom bounds saves the potentially expensive per-frame recalculation of bounds, but can decrease culling efficiency since the bounds might need to be larger than the actual effect at a particular frame.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.useAutomaticBounds = false;
psSettings.customBounds = AABox::UNIT_BOX;
~~~~~~~~~~~~~

# Maximum particle count

@bs::ParticleSystemSettings::maxParticles can be used to limit the maximum number of particles. Once this number of reached no new particles will be spawned, until some old particles die and make room.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.maxParticles = 4000;
~~~~~~~~~~~~~

# Seeding

Emitters and most evolvers use random number generation when spawning/manipulating particles. Seed for those generators is generated automatically when the particle system is first spawned. This behaviour can be disabled by toggling @bs::ParticleSystemSettings::useAutomaticSeed. When automatic seed generation is disabled make sure to provide the custom seed in @bs::ParticleSystemSettings::manualSeed.

Using a custom seed ensures you can always achieve the same exact particle effect, while using an automatic seed will make your particle effect slightly different every time it is played.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.useAutomaticSeed = false;
psSettings.manualSeed = 123456;
~~~~~~~~~~~~~