---
title: Setting up a particle system
---

Particles allow for rendering of objects that cannot be easily represented using normal static or animated meshes, such as liquids, smoke, flames or magic effects. Particles are normally rendered as dozens/hundreds/thousands of 2D billboards oriented towards the viewer, using some user-specified texture and material.

![An example of a particle system](../../Images/ParticleSystem.gif)  

Particle system is represented through the @b3d::ParticleSystem component and consists of the following parts:
 - One or multiple **emitters** - Each emitter has a shape that specifies where should new particles appear, as well as a variety of rules that control the initial properties of the particles.
 - Zero or more **evolvers** - Evolvers allow you to control/modify particle behaviour during their lifetime.
 - Global **settings** - These include the material to render the particles with, particle orientation, render mode (2D or 3D) and more.
 
Thoughout these set of manuals we will go over all the available emitters, evolvers and settings, but for now lets set up a basic particle system. 

~~~~~~~~~~~~~{.cpp}
// Create a scene object and add a particle system component
HSceneObject particleSystemSceneObject = SceneObject::Create("ParticleSystem");
HParticleSystem particleSystem = particleSystemSceneObject->AddComponent<ParticleSystem>();

// Create a material to use for rendering the particles
HShader shader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::ParticlesUnlit);
HTexture texture = GetBuiltinResources().GetTexture(BuiltinTexture::White);

HMaterial material = Material::Create(shader);
material->SetTexture("gTexture", texture);

// Set the material to be used by the particle system
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.Material = material;

particleSystem->SetSettings(particleSystemSettings);

// Add an emitter that emits particles on the surface of a sphere
TShared<ParticleEmitter> emitter = B3DMakeShared<ParticleEmitter>();

ParticleSphereShapeSettings sphereShape;
sphereShape.Radius = 0.3f;
emitter->SetShape(ParticleEmitterSphereShape::Create(sphereShape));

particleSystem->SetEmitters({emitter});
~~~~~~~~~~~~~

As we see in the example above the basic system needs two things at minimum:
 - A material to render the particles with
 - A single particle emitter
 
# Material
You can set the material used to render the particles through the @b3d::ParticleSystemSettings object. The settings can be applied to the particle system by calling @b3d::ParticleSystem::SetSettings(). We'll cover all the settings available eventually, but for now you can focus only on setting the material.

Multiple built-in shaders exist for the purpose of rendering particle systems:
 - @b3d::BuiltinShader::ParticlesUnlit - Renders particles without any lighting, and supports transparent particles. This is the most commonly used shader for rendering particles. Use the **gTexture** property to assign a texture to the material.
 - @b3d::BuiltinShader::ParticlesLit - Renders particles using the physically based shading model, while supporting transparent particles. Similar to the **BuiltinShader::Transparent** shader, supporting the same properties.
 - @b3d::BuiltinShader::ParticlesLitOpaque - Renders particles using the physically based shading model through the more feature-rich deferred rendering path, but without supporting particle transparency. Similar to the **BuiltinShader::Standard** shader, supporting the same properties.

**BuiltinShader::ParticlesUnlit** and **BuiltinShader::ParticlesLit** also support an optional **SOFT** variation that blends the particles softly with any intersecting surfaces, making such intersections less noticeable (see below for an example).
 
~~~~~~~~~~~~~{.cpp}
// Create a material to use for rendering the particles
HShader shader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::ParticlesUnlit);
HTexture texture = GetBuiltinResources().GetTexture(BuiltinTexture::White);

HMaterial material = Material::Create(shader);
material->SetTexture("gTexture", texture);

// Use the soft variation
material->SetVariation(
	ShaderVariation({ ShaderVariation::Param("SOFT", true) }));

// Set the material to be used by the particle system
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.Material = material;

particleSystem->SetSettings(particleSystemSettings);
~~~~~~~~~~~~~

Query the current settings:

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings particleSystemSettings = particleSystem->GetSettings();
~~~~~~~~~~~~~

You may also create your own particle shaders, as we will show later.

![Left - Normal (non-soft) shader, Right - soft shader](../../Images/softParticles.gif)  

# Emitter

Emitters determine where are new particles spawned, along with other properties that are assigned to newly spawned particles. They are represented using the @b3d::ParticleEmitter class.

Once created they can be registered with the particle system by setting a list of emitters through @b3d::ParticleSystem::SetEmitters.

~~~~~~~~~~~~~{.cpp}
TShared<ParticleEmitter> emitter = B3DMakeShared<ParticleEmitter>();
particleSystem->SetEmitters({emitter});
~~~~~~~~~~~~~

Query the current emitters:

~~~~~~~~~~~~~{.cpp}
Vector<TShared<ParticleEmitter>> emitters = particleSystem->GetEmitters();
~~~~~~~~~~~~~

## Shape
An emitter has many properties, but the one essential property you must assign is the emitter shape. The shape controls where will the new particles spawn, as well as the particle initial travel direction (usually inheriting the shape normal). Various shapes are supported, from primitives like cubes and spheres to static and skinned meshes. We'll cover all the shape types in detail in a later manual.

The shape is assigned by calling @b3d::ParticleEmitter::SetShape.

~~~~~~~~~~~~~{.cpp}
ParticleSphereShapeSettings sphereShape;
sphereShape.Radius = 0.3f;
emitter->SetShape(ParticleEmitterSphereShape::Create(sphereShape));
~~~~~~~~~~~~~

Query the shape:

~~~~~~~~~~~~~{.cpp}
TShared<ParticleEmitterShape> shape = emitter->GetShape();
~~~~~~~~~~~~~

## Other emitter properties

### Spawn rate
Controls how many particles to spawn and when. There are two ways to set this property:
 - @b3d::ParticleEmitter::SetEmissionRate - Sets up a continuous emission by setting the number of particles emitted per second.
 - @b3d::ParticleEmitter::SetEmissionBursts - Sets up bursts of particles that trigger at a specific point in time, and optionally repeat at an interval. Each burst is defined through the @b3d::ParticleBurst structure.

~~~~~~~~~~~~~{.cpp}
// Continually emit 50 particles per second
emitter->SetEmissionRate(50.0f);

// Spawn 200 particles in bulk every five seconds
emitter->SetEmissionBursts(
{
    ParticleBurst(
     5.0f, // Time at which to trigger
     200.0f, // How many particles to spawn
     0, // How many times to repeat (0 = infinite)
     5.0f), // Interval between bursts
});
~~~~~~~~~~~~~

Query the emission rate and bursts:

~~~~~~~~~~~~~{.cpp}
float emissionRate = emitter->GetEmissionRate();
Vector<ParticleBurst> emissionBursts = emitter->GetEmissionBursts();
~~~~~~~~~~~~~

![Emission bursts](../../Images/emissionBurst.gif)  

### Lifetime
Controls how long should each individual particle live, in seconds. After the lifetime expires the particle disappears. Set it through @b3d::ParticleEmitter::SetInitialLifetime.

~~~~~~~~~~~~~{.cpp}
// Each particle will stay alive for exactly 10 seconds
emitter->SetInitialLifetime(10.0f);
~~~~~~~~~~~~~

Query the initial lifetime:

~~~~~~~~~~~~~{.cpp}
FloatDistribution lifetime = emitter->GetInitialLifetime();
~~~~~~~~~~~~~

### Size
Controls the size of each individual particle. Size can be set uniformly for all axes, or separately for each axis:
 - @b3d::ParticleEmitter::SetInitialSize - Sets the uniform size of the particles. Only used if @b3d::ParticleEmitter::SetUse3DSize is set to false (false being the default).
 - @b3d::ParticleEmitter::SetInitialSize3D - Sets the size of the particles with the ability to specify a different size for each axis. If using standard particles only X & Y components are used. If using 3D particles (which we'll discuss later) all three components are used. Only used if **ParticleEmitter::SetUse3DSize()** is set to true.

~~~~~~~~~~~~~{.cpp}
// Set uniform size of individual particle to 0.1 meters
emitter->SetInitialSize(0.1f);

// Or, enable 3D size and set the dimensions separately
emitter->SetUse3DSize(true);
emitter->SetInitialSize3D(Vector3(0.1f, 0.05f, 0.25f));
~~~~~~~~~~~~~

Query the initial size and 3D size settings:

~~~~~~~~~~~~~{.cpp}
FloatDistribution size = emitter->GetInitialSize();
Vector3Distribution size3D = emitter->GetInitialSize3D();
bool use3DSize = emitter->GetUse3DSize();
~~~~~~~~~~~~~

### Color
Controls the color (RGB channels) and transparency (A channel) of particles. Set it through @b3d::ParticleEmitter::SetInitialColor.

~~~~~~~~~~~~~{.cpp}
// Set color to bright red
emitter->SetInitialColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
~~~~~~~~~~~~~

Query the initial color:

~~~~~~~~~~~~~{.cpp}
ColorDistribution color = emitter->GetInitialColor();
~~~~~~~~~~~~~

### Speed
Controls the speed of particles, along their initial travel directions (determined by shape). Set to zero to keep the particles static. Set it through @b3d::ParticleEmitter::SetInitialSpeed.

~~~~~~~~~~~~~{.cpp}
// Move particles at a rate of 5 m/s
emitter->SetInitialSpeed(5.0f);
~~~~~~~~~~~~~

Query the initial speed:

~~~~~~~~~~~~~{.cpp}
FloatDistribution speed = emitter->GetInitialSpeed();
~~~~~~~~~~~~~

### Rotation
Controls the initial orientation of the particles. It can be set as a single value, which rotates the particle around its Z axis, or as separate values for each axis separately. The values specified are in degrees.
 - @b3d::ParticleEmitter::SetInitialRotation - Sets the rotation around the Z axis in degrees. This should be used for 2D view-facing particles as orientation around other axes doesn't make sense. Used if @b3d::ParticleEmitter::SetUse3DRotation is disabled (disabled being the default).
 - @b3d::ParticleEmitter::SetInitialRotation3D - Sets rotation around each axis separately, in degrees (Euler angles). This can be used for 3D particles. Only used if **ParticleEmitter::SetUse3DRotation()** is set to true.

~~~~~~~~~~~~~{.cpp}
// Spawn particles at a 45 degree angle
emitter->SetInitialRotation(45.0f);

// Or, enable 3D rotation and set the rotation per axis
emitter->SetUse3DRotation(true);
emitter->SetInitialRotation3D(Vector3(0.0f, 45.0f, 90.0f));
~~~~~~~~~~~~~

Query the initial rotation and 3D rotation settings:

~~~~~~~~~~~~~{.cpp}
FloatDistribution rotation = emitter->GetInitialRotation();
Vector3Distribution rotation3D = emitter->GetInitialRotation3D();
bool use3DRotation = emitter->GetUse3DRotation();
~~~~~~~~~~~~~
 
### Random offset
Use this value to apply a completely random position offset to each spawned particle in the specified radius. This offset is applied on top of the particle's position as determined by the emitter shape. Set it through @b3d::ParticleEmitter::SetRandomOffset.

~~~~~~~~~~~~~{.cpp}
// Spawn particles on the emitter shape as normal, but then apply a random offset in a 0.2m radius sphere
emitter->SetRandomOffset(0.2f);
~~~~~~~~~~~~~

Query the random offset:

~~~~~~~~~~~~~{.cpp}
float randomOffset = emitter->GetRandomOffset();
~~~~~~~~~~~~~

### UV flip
Particle texture coordinates (UV) can be randomly flipped vertically or horizontally (or both), which can result in better visual variation of particles. Use @b3d::ParticleEmitter::SetFlipU to set a flip chance for the horizontal axis and/or @b3d::ParticleEmitter::SetFlipV to set a flip chance for the vertical axis. Both methods accept a value in range [0, 1], where 0 specifies no flip chance, and 1 specifies 100% flip chance.

~~~~~~~~~~~~~{.cpp}
// Flip the horizontal or vertical UV coordinate 50% of the time
emitter->SetFlipU(0.5f);
emitter->SetFlipV(0.5f);
~~~~~~~~~~~~~

Query the UV flip settings:

~~~~~~~~~~~~~{.cpp}
float flipU = emitter->GetFlipU();
float flipV = emitter->GetFlipV();
~~~~~~~~~~~~~

## Distributions
Majority of the emitter properties described so far are defined as *distributions*. Distributions allow you to specify a value in the form a constant, a random range or a time varying curve. In the examples above we have only used constants for simplicity, but a lot more complex behaviour can be achieved by using random ranges or curves.

~~~~~~~~~~~~~{.cpp}
// Randomly vary the color of a particle between red and blue
emitter->SetInitialColor(ColorDistribution(
	Color(1.0f, 0.0f, 0.0f, 1.0f),
	Color(0.0f, 1.0f, 0.0f, 1.0f)
));

// Specify a time-varying curve that makes new particles bigger with time
emitter->SetInitialSize(FloatDistribution(TAnimationCurve<float>(
{
	TKeyframe<float>{0.1f, 0.0f, 1.0f, 0.0f}, // Start at 0.1 size
	TKeyframe<float>{0.4f, 1.0f, 0.0f, 1.0f}, // And grow up to 0.4 size
})));
~~~~~~~~~~~~~

See the [animation curves](../Utilities/animCurves), [color gradient](../Utilities/colorGradient) and [distributions](../Utilities/distributions) manuals for more information.

All the time-varying distributions are evaluated using the lifetime of the particle system. You can change the duration of a particle system by setting @b3d::ParticleSystemSettings::Duration. The particle system time will be normalized to [0, 1] range using the specified duration, and that normalized value will be used for sampling the curves in the time-varying distributions (therefore make sure those curves have times in the [0, 1] range).

~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings particleSystemSettings = particleSystem->GetSettings();
particleSystemSettings.Duration = 10.0f; // Evaluates time-varying distributions over the course of 10 seconds

particleSystem->SetSettings(particleSystemSettings);
~~~~~~~~~~~~~

# ECS fragments

The **ParticleSystem** component stores its data internally as ECS fragments, enabling the renderer and simulation systems to batch-process all particle systems in the scene efficiently. Particle systems use more fragments than other components due to the complexity of particle simulation.

## Data fragment

The primary fragment is @b3d::ecs::ParticleSystem, which stores the particle system's configuration:
 - **Settings** - General purpose particle system settings (material, orientation, render mode, duration, etc.)
 - **GpuSimulationSettings** - Settings for GPU-based particle simulation
 - **Layer** - Layer bitfield for camera visibility filtering
 - **Emitters** - Array of particle emitters that control particle generation
 - **Evolvers** - Array of particle evolvers that modify particles during their lifetime, sorted by priority
 - **Id** - Unique identifier for this particle system instance

## Simulation fragment

In addition to the configuration data, each particle system has an @b3d::ecs::ParticleSimulation fragment that stores the runtime simulation state:
 - **Particles** - Active particle data set, lazily allocated on the first simulation frame
 - **Time** - Current simulation time
 - **Seed** - Random seed for deterministic simulation
 - **Rng** - Random number generator seeded with the seed value
 - **State** - Current simulation state (uninitialized, stopped, paused, or playing)

When using raw ECS fragments, the `ecs::CreateParticleSystem` helper creates this fragment automatically.

## ID fragment

Each particle system also has an @b3d::ecs::ParticleSystemId fragment that stores a persistent renderer ID used by the @b3d::RendererObjectStorage system for mapping to the packed render-thread representation.

## Using raw ECS fragments

You can bypass the **ParticleSystem** component and create `ecs::ParticleSystem` fragments directly for maximum performance. Helper functions @b3d::ecs::CreateParticleSystem and @b3d::ecs::DestroyParticleSystem handle fragment creation (including the simulation fragment), world transform, renderer ID allocation, and cleanup. Use @b3d::ecs::ParticleSystemECSUtility to mark dirty after property changes.

~~~~~~~~~~~~~{.cpp}
const TShared<SceneInstance>& scene = SceneManager::Instance().GetMainScene();
ecs::Registry& registry = scene->GetECSRegistry();
const TShared<RendererScene>& rendererScene = scene->GetRendererScene();

// Create an entity with all particle system fragments, a world transform, and a renderer ID
ecs::Entity entity = registry.CreateEntity();
ecs::ParticleSystem& fragment = ecs::CreateParticleSystem(registry, entity, rendererScene, myTransform);

// Configure the particle system
ParticleSystemSettings settings;
settings.Material = myParticleMaterial;
fragment.Settings = settings;
fragment.Layer = 1;

// Set up emitters
TShared<ParticleEmitter> emitter = B3DMakeShared<ParticleEmitter>();
ParticleSphereShapeSettings sphereShape;
sphereShape.Radius = 0.3f;
emitter->SetShape(ParticleEmitterSphereShape::Create(sphereShape));
fragment.Emitters = { emitter };
~~~~~~~~~~~~~

When modifying the fragment after creation, mark it dirty so the change is synced to the render thread:

~~~~~~~~~~~~~{.cpp}
ecs::ParticleSystem& fragment = registry.GetComponents<ecs::ParticleSystem>(entity);
fragment.Settings.Material = newMaterial;
ecs::ParticleSystemECSUtility::MarkDirty(registry, entity);

// For transform-only changes
registry.GetComponents<ecs::WorldTransform>(entity) = ecs::WorldTransform(newTransform);
ecs::ParticleSystemECSUtility::MarkTransformDirty(registry, entity);
~~~~~~~~~~~~~

When destroying the entity, call `ecs::DestroyParticleSystem` which removes fragments. Cleanup of the renderer ID and dirty tags is handled by the associated RendererScene:

~~~~~~~~~~~~~{.cpp}
ecs::DestroyParticleSystem(registry, entity);
registry.EraseEntity(entity);
~~~~~~~~~~~~~
