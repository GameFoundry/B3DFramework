Other particle system settings			{#particleSettings}
===============
[TOC]

We already mentioned @ref bs::ParticleSystemSettings "ParticleSystemSettings" when we have shown how to apply a **Material** to the particle system. In this manual we will go over the rest of the options provided by that object and show how they affect the particle system. 

To recap, **ParticleSystemSettings** object can be assigned to a particle system by calling @ref bs::CParticleSystem::setSettings "CParticleSystem::setSettings()".

~~~~~~~~~~~~~{.cpp}
HParticleSystem particleSystem = ...;

// Set the material to be used by the particle system
ParticleSystemSettings psSettings;
psSettings.material = particleMaterial;

particleSystem->setSettings(psSettings);
~~~~~~~~~~~~~

# Duration and looping {#particleSettings_a}

Duration of the particle system controls how are time-based curves in various distributions evaluated. This includes all the distributions in **ParticleEmitter** but also other distributions that are evaluated based on system lifetime. When the particle system is first spawned its internal timer starts ticking and current time is incremented. This time is divided by the duration and then passed to relevant curves in [0, 1] range. Use @ref bs::ParticleSystemSettings::duration "ParticleSystemSettings::duration" to set the duration in seconds.

Once the time passes the system duration the time will loop back to the start. If you prefer to instead clamp the time when the duration is reached, disable @ref bs::ParticleSystemSettings::isLooping "ParticleSystemSettings::isLooping".

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.duration = 5.0f; // 5 seconds
psSettings.isLooping = true;
~~~~~~~~~~~~~

# Simulation space {#particleSettings_b}

Simulation space controls in which space are particles spawned. It is set through @ref bs::ParticleSystemSettings::simulationSpace "ParticleSystemSettings::simulationSpace" and can have two values:
 - @ref bs::ParticleSimulationSpace::Local "ParticleSimulationSpace::Local" - Particles are spawned in space local to the particle system. If you move the particle system all the particles will move with it. As an example imagine electrical sparks on a moving vehicle.
 - @ref bs::ParticleSimulationSpace::World "ParticleSimulationSpace::World" - Particles are spawned in world space. If you move the particle system any existing particles will keep their original location while any newly spawned particles will spawn at the new location. As an example imagine a flamethrower as the player moves or rotates.
 
~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.simulationSpace = ParticleSimulationSpace::World;
~~~~~~~~~~~~~

# Orientation {#particleSettings_c}

This set of options allow you to modify how are particle billboards oriented. It is controlled through the @ref bs::ParticleSystemSettings::orientation "ParticleSystemSettings::orientation" field and can take on the following values:
 - @ref bs::ParticleOrientation::ViewPlane "ParticleOrientation::ViewPlane" - Particles will be oriented towards the camera plane, so that their X & Y axes are parallel to the camera plane. All particles have the exact same orientation.
 - @ref bs::ParticleOrientation::ViewPosition "ParticleOrientation::ViewPosition" - Achieves a similar effect as **ParticleOrientation::ViewPlane**, but orients towards the camera's position. Particles will have slightly different orientation depending on their position, but will still face the camera.
 - @ref bs::ParticleOrientation::Plane "ParticleOrientation::Plane" - Particles will always face a certain plane. The plane can be specified by setting @ref bs::ParticleSystemSettings::orientationPlane "ParticleSystemSettings::orientationPlane". This is particularily useful for 2D games.
 
You can also forbid particles to rotate around the Y axis by setting @ref bs::ParticleSystemSettings::orientationLockY "ParticleSystemSettings::orientationLockY". This can be useful for particles that must remain "upright", yet still face the camera. As an example imagine grass billboards.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.orientation = ParticleOrientation::ViewPosition;
psSettings.orientationLockY = true;
~~~~~~~~~~~~~

# Render mode {#particleSettings_d}

This options allows to to choose should particles render as 2D billboards or as 3D meshes. It can be set through @ref bs::ParticleSystemSettings::renderMode "ParticleSystemSettings::renderMode" and has two properties:
 - @ref bs::ParticleRenderMode::Billboard "ParticleRenderMode::Billboard" - Render as 2D billboard.
 - @ref bs::ParticleRenderMode::Mesh "ParticleRenderMode::Mesh" - Render as a 3D mesh.
 
When rendering as a 3D mesh you will need to specify the mesh to use in the @ref bs::ParticleSystemSettings::mesh "ParticleSystemSettings::mesh" field. You might also want to use a shader that supports lighting, such as **BuiltinShader::ParticleLitOpaque**.

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

# Sorting {#particleSettings_e}

Certain particle shaders might require particles to be sorted in a particular order to work properly. You can control particle sorting through the @ref bs::ParticleSystemSettings::sortMode "ParticleSystemSettings::sortMode" field. It has the following options:
 - @ref bs::ParticleSortMode::None "ParticleSortMode::None" - Don't sort the particles.
 - @ref bs::ParticleSortMode::Distance "ParticleSortMode::Distance" - Sort the particles back to front, based on distance from the camera.
 - @ref bs::ParticleSortMode::OldToYoung "ParticleSortMode::OldToYoung" - Sort the particles so youngest are rendered first.
 - @ref bs::ParticleSortMode::YoungToOld "ParticleSortMode::YoungToOld" - Sort the particles so oldest are rendered first.

Note that sorting is a potentially expensive process and should be disabled unless necessary.
 
~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.sortMode = ParticleSortMode::Distance;
~~~~~~~~~~~~~
 
# Bounds {#particleSettings_f}

Systems like visibility culling require the world bounds of the particle system to be calculated every frame. Bounds can be calculated automatically or bounds can be provided manually. Use @ref bs::ParticleSystemSettings::useAutomaticBounds "ParticleSystemSettings::useAutomaticBounds" to toggle automatic bound calculation.

When disabled you can provide custom bounds by setting @ref bs::ParticleSystemSettings::customBounds "ParticleSystemSettings::customBounds". User must ensure these bounds completely cover the potential particle system volume, otherwise is might get culled even when particles are still in view.

Using custom bounds saves the potentially expensive per-frame recalculation of bounds, but can decrease culling efficiency since the bounds might need to be larger than the actual effect at a particular frame.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.useAutomaticBounds = false;
psSettings.customBounds = AABox::UNIT_BOX;
~~~~~~~~~~~~~

# Maximum particle count {#particleSettings_g}

@ref bs::ParticleSystemSettings::maxParticles "ParticleSystemSettings::maxParticles" can be used to limit the maximum number of particles. Once this number of reached no new particles will be spawned, until some old particles die and make room.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.maxParticles = 4000;
~~~~~~~~~~~~~

# Seeding {#particleSettings_h}

Emitters and most evolvers use random number generation when spawning/manipulating particles. Seed for those generators is generated automatically when the particle system is first spawned. This behaviour can be disabled by toggling @ref bs::ParticleSystemSettings::useAutomaticSeed "ParticleSystemSettings::useAutomaticSeed". When automatic seed generation is disabled make sure to provide the custom seed in @ref bs::ParticleSystemSettings::manualSeed "ParticleSystemSettings::manualSeed".

Using a custom seed ensures you can always achieve the same exact particle effect, while using an automatic seed will make your particle effect slightly different every time it is played.

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings;
psSettings.useAutomaticSeed = false;
psSettings.manualSeed = 123456;
~~~~~~~~~~~~~