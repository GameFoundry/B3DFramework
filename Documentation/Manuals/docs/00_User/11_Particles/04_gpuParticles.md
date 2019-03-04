GPU particles			{#gpuParticles}
===============
[TOC]

GPU simulated particles move all your particle simulation to the GPU, freeing up your CPU for other things and allowing much higher particle counts. While it's generally not suggested to have more than 5000 particles in a CPU simulated system, a GPU simulated one can handle dozens or even hundreds of thousands of particles.

To enable GPU simulation set the @ref bs::ParticleSystemSettings::gpuSimulation "ParticleSystemSettings::gpuSimulation" field to true.

~~~~~~~~~~~~~{.cpp}
// Enable GPU simulation
ParticleSystemSettings psSettings;
psSettings.gpuSimulation = true;

HParticleSystem particleSystem = ...;
particleSystem->setSettings(psSettings);
~~~~~~~~~~~~~

GPU simulation does not support particle evolvers and instead the simulation is controlled through a fixed set of parameters provided on @ref bs::ParticleGpuSimulationSettings "ParticleGpuSimulationSettings", which can be set on the particle system through @ref bs::ParticleSystem::setGpuSimulationSettings "ParticleSystem::setGpuSimulationSettings()". This makes the GPU simulated system more limited than a CPU simulated one, at the benefit of better performance.

~~~~~~~~~~~~~{.cpp}
ParticleGpuSimulationSettings gpuSimSettings;
// ... set some properties ...

particleSystem->setGpuSimulationSettings(gpuSimSettings);
~~~~~~~~~~~~~

Let's go over all the available properties on the GPU simulation settings object.

# Color over lifetime {#gpuParticles_a}
Equivalent to the **ParticleColor** evolver, modifies the particle color over its lifetime. Specified with @ref bs::ParticleGpuSimulationSettings::colorOverLifetime "ParticleGpuSimulationSettings::colorOverLifetime".

~~~~~~~~~~~~~{.cpp}
// Transition particle colors from white to back during their lifetime
gpuSimSettings.colorOverLifetime = ColorGradient(
{
    ColorGradientKey(Color::White, 0.0f),
    ColorGradientKey(Color::Black, 1.0f)
});
~~~~~~~~~~~~~

# Size scale over lifetime {#gpuParticles_b}
Similar to **ParticleSize** evolver, modifies the particle scale over its lifetime. Unlike **ParticleSize** this property will not override the existing scale, but rather use the provided value to scale the initially provided size. Scale can only be applied uniformly to all dimensions. Specified with @ref bs::ParticleGpuSimulationSettings::sizeScaleOverLifetime "ParticleGpuSimulationSettings::sizeScaleOverLifetime".

~~~~~~~~~~~~~{.cpp}
// Scale particles from 1 to 4 during their lifetime
gpuSimSettings.sizeScaleOverLifetime = TAnimationCurve<float>(
{
    TKeyframe<float>{1.0f, 0.0f, 1.0f, 0.0f},
    TKeyframe<float>{4.0f, 1.0f, 0.0f, 1.0f},
});
~~~~~~~~~~~~~

# Acceleration & drag {#gpuParticles_c}
Use @ref bs::ParticleGpuSimulationSettings::acceleration "ParticleGpuSimulationSettings::acceleration" to apply a constant acceleration during particle's lifetime. Use
@ref bs::ParticleGpuSimulationSettings::drag "ParticleGpuSimulationSettings::drag" to apply resistance in the direction opposite to particle's velocity (e.g. for simulating air resistance).

~~~~~~~~~~~~~{.cpp}
// Apply gravity
gpuSimSettings.acceleration = Vector3(0.0f, -9.81f, 0.0f);

// And simulate some air resistance
gpuSimSettings.drag = 0.1f;
~~~~~~~~~~~~~

# Collisions {#gpuParticles_d}
GPU simulated particles support a specialized depth collision mode for simulating world collisions. This is an extremely fast collision approach that supports collisions with arbitrary world geometry for thousands of particles with minimal performance impact. It uses visual geometry, not physical geometry, for determining collisions. Note this system performs collisions in screen-space, meaning it only has the information currently shown on the screen - collisions cannot occur with geometry not currently visible.

![Depth buffer collisions](depthBufferCollisions.gif)

All depth collision options are controlled through a @ref bs::ParticleDepthCollisionSettings "ParticleDepthCollisionSettings" that's available on @ref bs::ParticleGpuSimulationSettings::depthCollision "ParticleGpuSimulationSettings::depthCollision".

~~~~~~~~~~~~~{.cpp}
gpuSimSettings.depthCollision.enabled = true;
gpuSimSettings.depthCollision.restitution = 0.5f; // Controls bounciness
gpuSimSettings.depthCollision.dampening = 0.5; // Controls energy loss on collision
gpuSimSettings.depthCollision.radiusScale = 1.0f; // Scale of the collision radius (relative to particle size)
~~~~~~~~~~~~~

Toggle @ref bs::ParticleDepthCollisionSettings::enabled "ParticleDepthCollisionSettings::enabled" to turn depth collision on/off. 

@ref bs::ParticleDepthCollisionSettings::radiusScale "ParticleDepthCollisionSettings::radiusScale" can be used to control the physical size of the particles. The particles are approximated as spheres for the purpose of collisions and this value will be multiplied with visual particle size to determine the radius of the collision sphere.

Use @ref bs::ParticleDepthCollisionSettings::restitution "ParticleDepthCollisionSettings::restitution" to control how bouncy the collisions are (0 = no bounce, 1 = very bouncy), and @ref bs::ParticleDepthCollisionSettings::dampening "ParticleDepthCollisionSettings::dampening" to control how much energy is lost after a collision (slows down the particle after a collision).

# Vector field {#gpuParticles_e}
Vector fields are perhaps the most important feature of a GPU simulated particle system, giving you very detailed control of the behavior of particles in the system. Each vector field is represented as a 3D grid of vectors. This grid is placed relative to the particle system and as particles pass through the grid they will inherit force (and optionally velocity) from the grid cell the particle is located at. This gives the user a lot of control over the movement of the particles. This is especially useful for simulating things like fluids or smoke.

![Particle system with a vector field](vectorField.gif)  

All vector field properties are controlled through a @ref bs::ParticleVectorFieldSettings "ParticleVectorFieldSettings" object, accessible from @ref bs::TParticleGpuSimulationSettings<Core>::vectorField "ParticleGpuSimulationSettings::vectorField".

To enable the vector field you must assign a @ref bs::VectorField "VectorField" resource to @ref bs::ParticleVectorFieldSettings::vectorField "ParticleVectorFieldSettings::vectorField".

## Vector field resource {#gpuParticles_e_a}
Vector fields are represented through their own **VectorField** resource. A variety of third party tools exist that allow you to create your own vector fields, including Maya Fluid Effects or VectorayGen. The framework can import vector fields in the ".fga" (Fluid Grid ASCII) format.

~~~~~~~~~~~~~{.cpp}
HVectorField vectorField = gImporter().import<VectorField>("MyVectorField.fga");
gpuSimSettings.vectorField.vectorField = vectorField;
~~~~~~~~~~~~~

## Transform {#gpuParticles_e_b}
The vector field is always relative to its parent particle system, centered at its origin. Use @ref bs::ParticleVectorFieldSettings::offset "ParticleVectorFieldSettings::offset" to offset the field from the parent system origin.

Each vector field has a size in world units, as specified by the **VectorField** resource. You can modify this size by changing the @ref bs::ParticleVectorFieldSettings::scale "ParticleVectorFieldSettings::scale" property.

~~~~~~~~~~~~~{.cpp}
// Move slightly to the right
gpuSimSettings.vectorField.offset = Vector3(2.0f, 0.0f, 0.0f);

// Double the field size
gpuSimSettings.vectorField.scale = Vector3(2.0f, 2.0f, 2.0f);
~~~~~~~~~~~~~

## Rotation {#gpuParticles_e_c}
The vector field is by default aligned to the axes of the parent particle system. Use @ref bs::ParticleVectorFieldSettings::rotation "ParticleVectorFieldSettings::rotation" to change the initial rotation.

~~~~~~~~~~~~~{.cpp}
// Rotate by 45 degrees around the Y axis
gpuSimSettings.vectorField.rotation = Quaternion(0.0f, Degree(45.0f), 0.0f);
~~~~~~~~~~~~~

More importantly though you are able to animate the vector field rotation. This usually creates a lot more interesting effects than a static vector field. Use 
@ref bs::ParticleVectorFieldSettings::rotationRate "ParticleVectorFieldSettings::rotationRate" to specify the rate of rotation of the field for each axis in degrees.

~~~~~~~~~~~~~{.cpp}
// Rotate by 90 degrees each second
gpuSimSettings.vectorField.rotationRate = Vector3(0.0f, 90.0f, 0.0f);
~~~~~~~~~~~~~

## Intensity & tightness {#gpuParticles_e_d}
You scale down the intensity of the forces/velocities of the vector field through the 
@ref bs::ParticleVectorFieldSettings::intensity "ParticleVectorFieldSettings::intensity" field.

@ref bs::ParticleVectorFieldSettings::tightness "ParticleVectorFieldSettings::tightness" determines how closely should the particles follow the vector field. When set of 1 the particle velocity will be inherited directly from the vector field every frame of the simulation. When set to 0 the vector field will only apply force while the velocity will be integrated normally.

~~~~~~~~~~~~~{.cpp}
gpuSimSettings.vectorField.intensity = 3.0f;
gpuSimSettings.vectorField.tightness = 0.0f;
~~~~~~~~~~~~~

## Tiling {#gpuParticles_e_e}
Finally, you can make a vector field repeat infinitely along a certain axis. You can imagine this as a series of cubes placed to one another along the specified axis - once a particle leaves one cube it is immediately affected by another. Tiling in all three dimensions means the particle can never escape the effects of a vector field.

Use @ref bs::ParticleVectorFieldSettings::tilingX "ParticleVectorFieldSettings::tilingX", @ref bs::ParticleVectorFieldSettings::tilingY "ParticleVectorFieldSettings::tilingY", @ref bs::ParticleVectorFieldSettings::tilingZ "ParticleVectorFieldSettings::tilingZ" to enable/disable tiling.

~~~~~~~~~~~~~{.cpp}
// Tile along the ground plane
gpuSimSettings.vectorField.tilingX = true;
gpuSimSettings.vectorField.tilingZ = true;
~~~~~~~~~~~~~