Setting up a particle system 			{#particleSystem}
===============
[TOC]

Particles allow for rendering of objects that cannot be easily represented using normal static or animated meshes, such as liquids, smoke, flames or magic effects. Particles are normally rendered as dozens/hundreds/thousands of 2D billboards oriented towards the viewer, using some user-specified texture and material.

![An example of a particle system](ParticleSystem.gif)  

Particle system is represented through the @ref bs::CParticleSystem "CParticleSystem" component and consists of the following parts:
 - One or multiple **emitters** - Each emitter has a shape that specifies where should new particles appear, as well as a variety of rules that control the initial properties of the particles.
 - Zero or more **evolvers** - Evolvers allow you to control/modify particle behaviour during their lifetime.
 - Global **settings** - These include the material to render the particles with, particle orientation, render mode (2D or 3D) and more.
 
Thoughout these set of manuals we will go over all the available emitters, evolvers and settings, but for now lets set up a basic particle system. 

~~~~~~~~~~~~~{.cpp}
// Create a scene object and add a particle system component
HSceneObject particleSystemSO = SceneObject::create("ParticleSystem");
HParticleSystem particleSystem = particleSystemSO->addComponent<CParticleSystem>();

// Create a material to use for rendering the particles
HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::ParticlesUnlit);
HTexture texture = gBuiltinResources().getTexture(BuiltinTexture::White);

HMaterial material = Material::create(shader);
material->setTexture("gTexture", texture);

// Set the material to be used by the particle system
ParticleSystemSettings psSettings;
psSettings.material = particleMaterial;

particleSystem->setSettings(psSettings);

// Add an emitter that emits particles on the surface of a sphere
SPtr<ParticleEmitter> emitter = bs_shared_ptr_new<ParticleEmitter>();

PARTICLE_SPHERE_SHAPE_DESC sphereShape;
sphereShape.radius = 0.3f;
emitter->setShape(ParticleEmitterSphereShape::create(sphereShape));

particleSystem->getEmitters()->add(emitter);
~~~~~~~~~~~~~

As we see in the example above the basic system needs two things at minimum:
 - A material to render the particles with
 - A single particle emitter
 
# Material {#particleSystem_a}
You can set the material used to render the particles through the @ref bs::ParticleSystemSettings "ParticleSystemSettings" object. The settings can be applied to the particle system by calling @ref bs::CParticleSystem "CParticleSystem::setSettings()". We'll cover all the settings available eventually, but for now you can focus only on setting the material.

Multiple built-in shaders exist for the purpose of rendering particle systems:
 - @ref bs::BuiltinShader::ParticlesUnlit "BuiltinShader::ParticlesUnlit" - Renders particles without any lighting, and supports transparent particles. This is the most commonly used shader for rendering particles. Use the **gTexture** property to assign a texture to the material.
 - @ref bs::BuiltinShader::ParticlesLit "BuiltinShader::ParticlesLit" - Renders particles using the physically based shading model, while supporting transparent particles. Similar to the **BuiltinShader::Transparent** shader, supporting the same properties.
 - @ref bs::BuiltinShader::ParticlesLitOpaque "BuiltinShader::ParticlesLitOpaque" - Renders particles using the physically based shading model through the more feature-rich deferred rendering path, but without supporting particle transparency. Similar to the **BuiltinShader::Standard** shader, supporting the same properties.
 
~~~~~~~~~~~~~{.cpp}
// Create a material to use for rendering the particles
HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::ParticlesUnlit);
HTexture texture = gBuiltinResources().getTexture(BuiltinTexture::White);

HMaterial material = Material::create(particleShader);
material->setTexture("gTexture", texture);

// Set the material to be used by the particle system
ParticleSystemSettings psSettings;
psSettings.material = particleMaterial;

particleSystem->setSettings(psSettings);
~~~~~~~~~~~~~

You may also create your own particle shaders, as we will show later.

# Emitter {#particleSystem_b}

Emitters determine where are new particles spawned, along with other properties that are assigned to newly spawned particles. They are represented using the @ref bs::ParticleEmitter "ParticleEmitter" class.

Once created they can be registered with the particle system by retrieving the @ref bs::ParticleSystemEmitters "ParticleSystemEmitters" object from @ref bs::ParticleSystem::getEmitters "ParticleSystem::getEmitters()" and calling its @ref bs::ParticleSystemEmitters::add "ParticleSystemEmitters::add()" method.

~~~~~~~~~~~~~{.cpp}
SPtr<ParticleEmitter> emitter = bs_shared_ptr_new<ParticleEmitter>();
particleSystem->getEmitters()->add(emitter);
~~~~~~~~~~~~~

## Shape {#particleSystem_b_a}
An emitter has many properties, but the one essential property you must assign is the emitter shape. The shape controls where will the new particles spawn, as well as the particle initial travel direction (usually inheriting the shape normal). Various shapes are supported, from primitives like cubes and spheres to static and skinned meshes. We'll cover all the shape types in detail in a later manual.

The shape is assigned by calling @ref bs::ParticleEmitter::setShape "ParticleEmitter::setShape()".

~~~~~~~~~~~~~{.cpp}
PARTICLE_SPHERE_SHAPE_DESC sphereShape;
sphereShape.radius = 0.3f;
emitter->setShape(ParticleEmitterSphereShape::create(sphereShape));
~~~~~~~~~~~~~

## Other emitter properties {#particleSystem_b_b}

### Spawn rate {#particle_system_b_b_a}
Controls how many particles to spawn and when. There are two ways to set this property:
 - @ref bs::ParticleEmitter::setEmissionRate "ParticleEmitter::setEmissionRate()" - Sets up a continous emission by setting the number of particles emitted per second.
 - @ref bs::ParticleEmitter::setEmissionBursts "ParticleEmitter::setEmissionBursts()" - Sets up bursts of particles that trigger at a specific point in time, and optionally repeat at an interval. Each burst is defined through the @ref bs::ParticleBurst "ParticleBurst" structure.
 
~~~~~~~~~~~~~{.cpp}
// Continually emit 50 particles per second
emitter->setEmissionRate(50.0f);

// Spawn 200 particles in bulk every five seconds
emitter->setEmissionBursts(
{
    ParticleBurst(
     5.0f, // Time at which to trigger
     200.0f, // How many particles to spawn
     0, // How many times to repeat (0 = infinite)
     5.0f), // Interval between bursts
});
~~~~~~~~~~~~~

### Lifetime {#particle_system_b_b_b}
Controls how long should each individual particle live, in seconds. After the lifetime expires the particle disappears. Set it through @ref bs::ParticleEmitter::setInitialLifetime "ParticleEmitter::setInitialLifetime()".

~~~~~~~~~~~~~{.cpp}
// Each particle will stay alive for exactly 10 seconds
emitter->setInitialLifetime(10.0f);
~~~~~~~~~~~~~

### Size {#particle_system_b_b_c}
Controls the size of each individual particle. Size can be set uniformly for all axes, or separately for each axis:
 - @ref bs::ParticleEmitter::setInitialSize "ParticleEmitter::setInitialSize()" - Sets the uniform size of the particles. Only used if @ref bs::ParticleEmitter::setUse3DSize "ParticleEmitter::setUse3DSize()" is set to false (false being the default).
 - @ref bs::ParticleEmitter::setInitialSize3D "ParticleEmitter::setInitialSize3D()" - Sets the size of the particles with the ability to specify a different size for each axis. If using standard particles only X & Y components are used. If using 3D particles (which we'll discuss later) all three components are used. Only used if **ParticleEmitter::setUse3DSize()** is set to true.
 
~~~~~~~~~~~~~{.cpp}
// Set uniform size of individual particle to 0.1 meters
emitter->setInitialSize(0.1f);

// Or, enable 3D size and set the dimensions separately
emitter->setUse3DSize(true);
emitter->setInitialSize3D(Vector3(0.1f, 0.05f, 0.25f));
~~~~~~~~~~~~~

### Color {#particle_system_b_b_d}
Controls the color (RGB channels) and transparency (A channel) of particles. Set it through @ref bs::ParticleEmitter::setInitialColor "ParticleEmitter::setInitialColor()".

~~~~~~~~~~~~~{.cpp}
// Set color to bright red
emitter->setInitialColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
~~~~~~~~~~~~~

### Speed {#particle_system_b_b_e}
Controls the speed of particles, along their initial travel directions (determined by shape). Set to zero to keep the particles static. Set it through @ref bs::ParticleEmitter::setInitialSpeed "ParticleEmitter::setInitialSpeed()".

~~~~~~~~~~~~~{.cpp}
// Move particles at a rate of 5 m/s
emitter->setInitialSpeed(5.0f);
~~~~~~~~~~~~~

### Rotation {#particle_system_b_b_f}
Controls the initial orientation of the particles. It can be set as a single value, which rotates the particle around its Z axis, or as separate values for each axis separately. The values specified are in degrees.
 - @ref bs::ParticleEmitter::setInitialRotation "ParticleEmitter::setInitialRotation" - Sets the rotation around the Z axis in degrees. This should be used for 2D view-facing particles as orientation around other axes doesn't make sense. Used if @ref bs::ParticleEmitter::setUse3DRotation "ParticleEmitter::setUse3DRotation()" is disabled (disabled being the default).
 - @ref bs::ParticleEmitter::setInitialRotation3D "ParticleEmitter::setInitialRotation3D" - Sets rotation around each axis separately, in degrees (Euler angles). This can be used for 3D particles. Only used if **ParticleEmitter::setUse3DRotation()** is set to true.

~~~~~~~~~~~~~{.cpp}
// Spawn particles at a 45 degree angle
emitter->setInitialRotation(45.0f);

// Or, enable 3D rotation and set the rotation per axis
emitter->setUse3DRotation(true);
emitter->setInitialRotation3D(Vector3(0.0f, 45.0f, 90.0f));
~~~~~~~~~~~~~
 
### Random offset {#particle_system_b_b_g}
Use this value to apply a completely random position offset to each spawned particle in the specified radius. This offset is applied on top of the particle's position as determined by the emitter shape. Set it through @ref bs::ParticleEmitter::setRandomOffset "setRandomOffset()".

~~~~~~~~~~~~~{.cpp}
// Spawn particles on the emitter shape as normal, but then apply a random offset in a 0.2m radius sphere
emitter->setRandomOffset(0.2f);
~~~~~~~~~~~~~

### UV flip {#particle_system_b_b_h}
Particle texture coordinates (UV) can be randomlly flipped vertically or horizontally (or both), which can result in better visual variation of particles. Use @ref bs::ParticleEmitter::setFlipU "ParticleEmitter::setFlipU()" to set a flip chance for the horizontal axis and/or @ref bs::ParticleEmitter::setFlipV "ParticleEmitter::setFlipV()" to set a flip change for the vertical axis. Both methods accept a value in range [0, 1], where 0 specifies no flip chance, and 1 specifies 100% flip chance.

~~~~~~~~~~~~~{.cpp}
// Flip the horizontal or vertical UV coordinate 50% of the time
emitter->setFlipU(0.5f);
emitter->setFlipV(0.5f);
~~~~~~~~~~~~~

## Distributions {#particle_system_b_c}
Majority of the emitter properties described so far are defined as *distributions*. Distributions allow you to specify a value in the form a constant, a random range or a time varying curve. In the examples above we have only used constants for simplicity, but a lot more complex behaviour can be achieved by using random ranges or curves.

~~~~~~~~~~~~~{.cpp}
// Randomly vary the color of a particle between red and blue 
emitter->setInitialColor(ColorDistribution(
	Color(1.0f, 0.0f, 0.0f, 1.0f),
	Color(0.0f, 1.0f, 0.0f, 1.0f)
));

// Specify a time-varying curve that makes new particles bigger with time
emitter->setInitialSize(FloatDistribution(TAnimationCurve<float>(
{
	TKeyframe<float>{0.1f, 0.0f, 1.0f, 0.0f}, // Start at 0.1 size
	TKeyframe<float>{0.4f, 1.0f, 0.0f, 1.0f}, // And grow up to 0.4 size
})));
~~~~~~~~~~~~~

See the [animation curves](@ref animCurves), [color gradient](@ref colorGradient) and [distributions](@ref distributions) manuals for more information.

All the time-varying distributions are evaluated using the lifetime of the particle system. You can change the duration of a particle system by setting @ref bs::ParticleSystemSettings::duration "ParticleSystemSettings::duration". The particle system time will be normalized to [0, 1] range using the specified duration, and that normalized value will be used for sampling the curves in the time-varying distributions (therefore make sure those curves have times in the [0, 1] range). 
 
~~~~~~~~~~~~~{.cpp}
ParticleSystemSettings psSettings = particleSystem->getSettings();
psSettings.duration = 10.0f; // Evaluates time-varying distributions over the course of 10 seconds

particleSystem->setSettings(psSettings);
~~~~~~~~~~~~~