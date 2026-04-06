---
title: GPU particles
---

GPU simulated particles move all your particle simulation to the GPU, freeing up your CPU for other things and allowing much higher particle counts. While it's generally not suggested to have more than 5000 particles in a CPU simulated system, a GPU simulated one can handle dozens or even hundreds of thousands of particles.

To enable GPU simulation set the @b3d::ParticleSystemSettings::GpuSimulation field to true.

~~~~~~~~~~~~~{.cpp}
// Enable GPU simulation
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.GpuSimulation = true;

HParticleSystem particleSystem = ...;
particleSystem->SetSettings(particleSystemSettings);
~~~~~~~~~~~~~

GPU simulation does not support particle evolvers and instead the simulation is controlled through a fixed set of parameters provided on @b3d::ParticleGpuSimulationSettings, which can be set on the particle system through @b3d::ParticleSystem::SetGpuSimulationSettings. This makes the GPU simulated system more limited than a CPU simulated one, at the benefit of better performance.

~~~~~~~~~~~~~{.cpp}
ParticleGpuSimulationSettings gpuSimulationSettings;
// ... set some properties ...

particleSystem->SetGpuSimulationSettings(gpuSimulationSettings);
~~~~~~~~~~~~~

Query the current GPU simulation settings:

~~~~~~~~~~~~~{.cpp}
ParticleGpuSimulationSettings gpuSimulationSettings = particleSystem->GetGpuSimulationSettings();
~~~~~~~~~~~~~

Let's go over all the available properties on the GPU simulation settings object.

# Color over lifetime
Equivalent to the **ParticleColor** evolver, modifies the particle color over its lifetime. Specified with @b3d::ParticleGpuSimulationSettings::ColorOverLifetime.

~~~~~~~~~~~~~{.cpp}
// Transition particle colors from white to black during their lifetime
gpuSimulationSettings.ColorOverLifetime = ColorGradient(
{
    ColorGradientKey(Color::kWhite, 0.0f),
    ColorGradientKey(Color::kBlack, 1.0f)
});
~~~~~~~~~~~~~

# Size scale over lifetime
Similar to **ParticleSize** evolver, modifies the particle scale over its lifetime. Unlike **ParticleSize** this property will not override the existing scale, but rather use the provided value to scale the initially provided size. Scale can only be applied uniformly to all dimensions. Specified with @b3d::ParticleGpuSimulationSettings::SizeScaleOverLifetime.

~~~~~~~~~~~~~{.cpp}
// Scale particles from 1 to 4 during their lifetime
gpuSimulationSettings.SizeScaleOverLifetime = TAnimationCurve<float>(
{
    TKeyframe<float>{1.0f, 0.0f, 1.0f, 0.0f},
    TKeyframe<float>{4.0f, 1.0f, 0.0f, 1.0f},
});
~~~~~~~~~~~~~

# Acceleration & drag
Use @b3d::ParticleGpuSimulationSettings::Acceleration to apply a constant acceleration during particle's lifetime. Use @b3d::ParticleGpuSimulationSettings::Drag to apply resistance in the direction opposite to particle's velocity (e.g. for simulating air resistance).

~~~~~~~~~~~~~{.cpp}
// Apply gravity
gpuSimulationSettings.Acceleration = Vector3(0.0f, -9.81f, 0.0f);

// And simulate some air resistance
gpuSimulationSettings.Drag = 0.1f;
~~~~~~~~~~~~~

# Collisions
GPU simulated particles support a specialized depth collision mode for simulating world collisions. This is an extremely fast collision approach that supports collisions with arbitrary world geometry for thousands of particles with minimal performance impact. It uses visual geometry, not physical geometry, for determining collisions. Note this system performs collisions in screen-space, meaning it only has the information currently shown on the screen - collisions cannot occur with geometry not currently visible.

![Depth buffer collisions](../../Images/depthBufferCollisions.gif)

All depth collision options are controlled through a @b3d::ParticleDepthCollisionSettings that's available on @b3d::ParticleGpuSimulationSettings::DepthCollision.

~~~~~~~~~~~~~{.cpp}
gpuSimulationSettings.DepthCollision.Enabled = true;
gpuSimulationSettings.DepthCollision.Restitution = 0.5f; // Controls bounciness
gpuSimulationSettings.DepthCollision.Dampening = 0.5f; // Controls energy loss on collision
gpuSimulationSettings.DepthCollision.RadiusScale = 1.0f; // Scale of the collision radius (relative to particle size)
~~~~~~~~~~~~~

Toggle @b3d::ParticleDepthCollisionSettings::Enabled to turn depth collision on/off.

@b3d::ParticleDepthCollisionSettings::RadiusScale can be used to control the physical size of the particles. The particles are approximated as spheres for the purpose of collisions and this value will be multiplied with visual particle size to determine the radius of the collision sphere.

Use @b3d::ParticleDepthCollisionSettings::Restitution to control how bouncy the collisions are (0 = no bounce, 1 = very bouncy), and @b3d::ParticleDepthCollisionSettings::Dampening to control how much energy is lost after a collision (slows down the particle after a collision).

# Vector field
Vector fields are perhaps the most important feature of a GPU simulated particle system, giving you very detailed control of the behavior of particles in the system. Each vector field is represented as a 3D grid of vectors. This grid is placed relative to the particle system and as particles pass through the grid they will inherit force (and optionally velocity) from the grid cell the particle is located at. This gives the user a lot of control over the movement of the particles. This is especially useful for simulating things like fluids or smoke.

![Particle system with a vector field](../../Images/vectorField.gif)  

All vector field properties are controlled through a @b3d::ParticleVectorFieldSettings object, accessible from @b3d::ParticleGpuSimulationSettings::VectorField.

To enable the vector field you must assign a @b3d::VectorField resource to @b3d::ParticleVectorFieldSettings::VectorField.

## Vector field resource
Vector fields are represented through their own **VectorField** resource. A variety of third party tools exist that allow you to create your own vector fields, including Maya Fluid Effects or VectorayGen. The framework can import vector fields in the ".fga" (Fluid Grid ASCII) format.

~~~~~~~~~~~~~{.cpp}
HVectorField vectorField = GetImporter().Import<VectorField>("MyVectorField.fga");
gpuSimulationSettings.VectorField.VectorField = vectorField;
~~~~~~~~~~~~~

## Transform
The vector field is always relative to its parent particle system, centered at its origin. Use @b3d::ParticleVectorFieldSettings::Offset to offset the field from the parent system origin.

Each vector field has a size in world units, as specified by the **VectorField** resource. You can modify this size by changing the @b3d::ParticleVectorFieldSettings::Scale property.

~~~~~~~~~~~~~{.cpp}
// Move slightly to the right
gpuSimulationSettings.VectorField.Offset = Vector3(2.0f, 0.0f, 0.0f);

// Double the field size
gpuSimulationSettings.VectorField.Scale = Vector3(2.0f, 2.0f, 2.0f);
~~~~~~~~~~~~~

## Rotation
The vector field is by default aligned to the axes of the parent particle system. Use @b3d::ParticleVectorFieldSettings::Rotation to change the initial rotation.

~~~~~~~~~~~~~{.cpp}
// Rotate by 45 degrees around the Y axis
gpuSimulationSettings.VectorField.Rotation = Quaternion(0.0f, Degree(45.0f), 0.0f);
~~~~~~~~~~~~~

More importantly though you are able to animate the vector field rotation. This usually creates a lot more interesting effects than a static vector field. Use @b3d::ParticleVectorFieldSettings::RotationRate to specify the rate of rotation of the field for each axis in degrees.

~~~~~~~~~~~~~{.cpp}
// Rotate by 90 degrees each second
gpuSimulationSettings.VectorField.RotationRate = Vector3(0.0f, 90.0f, 0.0f);
~~~~~~~~~~~~~

## Intensity & tightness
You can scale down the intensity of the forces/velocities of the vector field through the @b3d::ParticleVectorFieldSettings::Intensity field.

@b3d::ParticleVectorFieldSettings::Tightness determines how closely should the particles follow the vector field. When set to 1 the particle velocity will be inherited directly from the vector field every frame of the simulation. When set to 0 the vector field will only apply force while the velocity will be integrated normally.

~~~~~~~~~~~~~{.cpp}
gpuSimulationSettings.VectorField.Intensity = 3.0f;
gpuSimulationSettings.VectorField.Tightness = 0.0f;
~~~~~~~~~~~~~

## Tiling
Finally, you can make a vector field repeat infinitely along a certain axis. You can imagine this as a series of cubes placed next to one another along the specified axis - once a particle leaves one cube it is immediately affected by another. Tiling in all three dimensions means the particle can never escape the effects of a vector field.

Use @b3d::ParticleVectorFieldSettings::TilingX, @b3d::ParticleVectorFieldSettings::TilingY, @b3d::ParticleVectorFieldSettings::TilingZ to enable/disable tiling.

~~~~~~~~~~~~~{.cpp}
// Tile along the ground plane
gpuSimulationSettings.VectorField.TilingX = true;
gpuSimulationSettings.VectorField.TilingZ = true;
~~~~~~~~~~~~~
