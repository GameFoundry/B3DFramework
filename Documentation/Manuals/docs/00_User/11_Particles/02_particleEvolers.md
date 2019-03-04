Particle evolvers 			{#particleEvolvers}
===============
[TOC]

Many of the particle properties can be controlled directly from the **ParticleEmitter**, such as particle color, size and velocity, but those properties will only affect the initial state of the particle. If you need to modify the particle state during its lifetime you must use evolvers. 

Evolvers can provide various functionality such as:
 - Changing the particle color, size, rotation, velocity over the lifetime
 - Applying gravity or arbitrary forces
 - Colliding particles with the world
 - Animating particle textures

All evolvers derive from @ref bs::ParticleEvolver "ParticleEvolver". They are created and registered with the particle system similarly to emitters. Use @ref bs::ParticleSystem::setEvolvers() "ParticleSystem::setEvolvers()" to set a list of evolvers to use for a particular particle system. All evolver types have a **create()** method and accept a descriptor structure that allows you to customize their properties.

~~~~~~~~~~~~~{.cpp}
// An example creating and registering an evolver that modifies particle size over lifetime
PARTICLE_SIZE_DESC desc;
desc.size = TAnimationCurve<float>(
{
	TKeyframe<float>{1.0f, 0.0f, 1.0f, 0.0f},
	TKeyframe<float>{4.0f, 1.0f, 0.0f, 1.0f},
});

SPtr<ParticleEvolver> evolver = ParticleSize::create(desc);
particleSystem->setEvolvers({evolver});
~~~~~~~~~~~~~

Lets go over all the available evolver types.

# Size over lifetime {#particleEvolvers_a}
Modifies the particle size over the particle lifetime.

Represented with the @ref bs::ParticleSize "ParticleSize" and initialization options provided through @ref bs::PARTICLE_SIZE_DESC "PARTICLE_SIZE_DESC". 

Uniform size is provided through @ref bs::PARTICLE_SIZE_DESC::size "PARTICLE_SIZE_DESC::size" in the form of a **FloatDistribution**. If @ref bs::PARTICLE_SIZE_DESC::use3DSize "PARTICLE_SIZE_DESC::use3DSize" is enabled you may instead a separate size value for all three axes in the @ref bs::PARTICLE_SIZE_DESC::size3D "PARTICLE_SIZE_DESC::size3D" field.

Note that all evolver distributions containing curves will be evaluated in range [0, 1], 0 representing the beginning of the particle lifetime and 1 representing the end.

~~~~~~~~~~~~~{.cpp}
// Scales the particle by 4 over the entirety of the particle's lifetime
PARTICLE_SIZE_DESC desc;
desc.size = TAnimationCurve<float>(
{
    TKeyframe<float>{1.0f, 0.0f, 1.0f, 0.0f},
    TKeyframe<float>{4.0f, 1.0f, 0.0f, 1.0f},
});

auto evolver = ParticleSize::create(desc);
~~~~~~~~~~~~~

# Color over lifetime {#particleEvolvers_b}
Modifies the particle color over the particle lifetime.

Represented with the @ref bs::ParticleColor "ParticleColor" and initialization options provided through @ref bs::PARTICLE_COLOR_DESC "PARTICLE_COLOR_DESC". 

Color is provided through @ref bs::PARTICLE_COLOR_DESC::color "PARTICLE_COLOR_DESC::color" in the form of a **ColorDistribution**, and is the only available property for this evolver.

~~~~~~~~~~~~~{.cpp}
// Fades from white to black over the particle's lifetime
PARTICLE_COLOR_DESC desc;
desc.color = ColorGradient(
{
    ColorGradientKey(Color::White, 0.0f),
    ColorGradientKey(Color::Black, 1.0f)
});

auto evolver = ParticleColor::create(desc);
~~~~~~~~~~~~~

# Rotation over lifetime {#particleEvolvers_c}
Rotates the particle over the particle lifetime.

Represented with the @ref bs::ParticleRotation "ParticleRotation" and initialization options provided through @ref bs::PARTICLE_ROTATION_DESC "PARTICLE_ROTATION_DESC". 

Rotation around Z axis is provided through @ref bs::PARTICLE_ROTATION_DESC::rotation "PARTICLE_SIZE_DESC::rotation" in the form of a **FloatDistribution**, containing angle in degrees. If @ref bs::PARTICLE_ROTATION_DESC::use3DRotation "PARTICLE_ROTATION_DESC::use3DRotation" is enabled you may instead a separate rotation value for each of the three axes in the @ref bs::PARTICLE_ROTATION_DESC::rotation3D "PARTICLE_ROTATION_DESC::rotation3D" field.

~~~~~~~~~~~~~{.cpp}
// Rotates the particle by 180 degrees over its lifetime
PARTICLE_ROTATION_DESC desc;
desc.rotation = TAnimationCurve<float>(
{
     TKeyframe<float>{0.0f, 0.0f, 1.0f, 0.0f},
     TKeyframe<float>{180.0f, 1.0f, 0.0f, 1.0f},
});

auto evolver = ParticleRotation::create(desc);
~~~~~~~~~~~~~

# Gravity {#particleEvolvers_d}
Applies the force of gravity to particles.

Represented with the @ref bs::ParticleGravity "ParticleGravity" and initialization options provided through @ref bs::PARTICLE_GRAVITY_DESC "PARTICLE_GRAVITY_DESC".

The gravity force is inherited from the physics system, but can be scaled for the purposes of this evolver by setting @ref bs::PARTICLE_GRAVITY_DESC::scale "PARTICLE_GRAVITY_DESC::scale".

~~~~~~~~~~~~~{.cpp}
// Applies the default gravity
PARTICLE_GRAVITY_DESC desc;
desc.scale = 1.0f;

auto evolver = ParticleGravity::create(desc);
~~~~~~~~~~~~~

# Force {#particleEvolvers_e}
Applies an arbitrary force to particles.

Represented with the @ref bs::ParticleForce "ParticleForce" and initialization options provided through @ref bs::PARTICLE_FORCE_DESC "PARTICLE_FORCE_DESC".

Use @ref bs::PARTICLE_FORCE_DESC::force "PARTICLE_FORCE_DESC::force" to specify the force direction and intensity. If @ref bs::PARTICLE_FORCE_DESC::worldSpace "PARTICLE_FORCE_DESC::worldSpace" is true the force direction is assumed to be provided in world space, otherwise it is assumed to be relative to the transform of the parent particle system.

~~~~~~~~~~~~~{.cpp}
// Applies a force to particles, pushing them towards the positive X axis
PARTICLE_FORCE_DESC desc;
desc.force = TAnimationCurve<Vector3>(
{
    TKeyframe<Vector3>{Vector3::ZERO, Vector3::ZERO, Vector3::ONE, 0.0f},
    TKeyframe<Vector3>{Vector3(100.0f, 0.0f, 0.0f), -Vector3::ONE, Vector3::ZERO, 0.5f},
});
desc.worldSpace = true;

auto evolver = ParticleForce::create(desc);
~~~~~~~~~~~~~

# Velocity {#particleEvolvers_f}
Sets the velocity of particles over their lifetime.

![Velocity evolver](velocityEvolver.gif)  

Represented with the @ref bs::ParticleVelocity "ParticleVelocity" and initialization options provided through @ref bs::PARTICLE_VELOCITY_DESC "PARTICLE_VELOCITY_DESC".

Use @ref bs::PARTICLE_VELOCITY_DESC::velocity "PARTICLE_VELOCITY_DESC::velocity" to specify the velocity direction and intensity. If @ref bs::PARTICLE_VELOCITY_DESC::worldSpace "PARTICLE_VELOCITY_DESC::worldSpace" is true the velocity direction is assumed to be provided in world space, otherwise it is assumed to be relative to the transform of the parent particle system.

~~~~~~~~~~~~~{.cpp}
// Moves the particles upwards
PARTICLE_VELOCITY_DESC desc;
desc.velocity = Vector3(0.0f, 0.2f, 0.0f);

auto evolver = ParticleVelocity::create(velocityDesc);
~~~~~~~~~~~~~

# Orbit {#particleEvolvers_g}
Sets angular velocity of particles as if they were orbiting a point in space.

Represented with the @ref bs::ParticleOrbit "ParticleOrbit" and initialization options provided through @ref bs::PARTICLE_ORBIT_DESC "PARTICLE_ORBIT_DESC".

Use @ref bs::PARTICLE_ORBIT_DESC::center "PARTICLE_ORBIT_DESC::center" to a specify a point to orbit around. If @ref bs::PARTICLE_ORBIT_DESC::worldSpace "PARTICLE_ORBIT_DESC::worldSpace" is true the point assumed to be provided in world space, otherwise it is assumed to be relative to the transform of the parent particle system.

Specify the speed of rotation by setting @ref bs::PARTICLE_ORBIT_DESC::velocity "PARTICLE_ORBIT_DESC::velocity" which allows to set the speed of rotation for each axis separately, in rotations per second. Evaluated in local or world space according to **PARTICLE_ORBIT_DESC::worldSpace**.

By default the particles will orbit at a fixed distance from the point, buy you can also make them move away or be drawn to the point by setting the @ref bs::PARTICLE_ORBIT_DESC::radial "PARTICLE_ORBIT_DESC::radial" field to a positive (move away) or negative (move closer) value.

~~~~~~~~~~~~~{.cpp}
// Rotates the particles over the local Y axis and slowly moves them away from the center
PARTICLE_ORBIT_DESC desc;
desc.center = Vector3(0.0f, 0.0f, 0.0f);
desc.velocity = Vector3(0.0f, 1.2f, 0.0f); // In rotations/second
desc.radial = 0.4f;

auto evolver = ParticleOrbit::create(desc);
~~~~~~~~~~~~~

# Collision {#particleEvolvers_h}
Makes the particles collide with the world or a user-provided set of planes.

![Particle collisions](particleCollision.gif)  

Represented with the @ref bs::ParticleCollisions "ParticleCollisions" and initialization options provided through @ref bs::PARTICLE_COLLISIONS_DESC "PARTICLE_COLLISIONS_DESC".

This evolver comes with two modes, settable through @ref bs::PARTICLE_COLLISIONS_DESC::mode "PARTICLE_COLLISIONS_DESC::mode":
 - @ref bs::ParticleCollisionMode::World "ParticleCollisionMode::World" - Particles will collide with all world geometry as defined by the physics components placed in the world. Optionally use bitmask @ref bs::PARTICLE_COLLISIONS_DESC::layer "PARTICLE_COLLISIONS_DESC::layer" to collide only with physics objects with a matching layer.
 - @ref bs::ParticleCollisionMode::World "ParticleCollisionMode::Plane" - This is a secondary collision mode that makes the particles collide only with a user-defined set of planes. This mode is cheaper performance-wise than world collisions. Use @ref bs::ParticleCollisions::setPlanes "ParticleCollisions::setPlanes()" to provide an array of planes to collide with.
 
Additionally, the following properties are used in either collision mode. @ref bs::PARTICLE_COLLISIONS_DESC::radius "PARTICLE_COLLISIONS_DESC::radius" specifies the size of individual particle (defined as a sphere). @ref bs::PARTICLE_COLLISIONS_DESC::restitution "PARTICLE_COLLISIONS_DESC::restitution" determines how much will be particles bounce after a collision, and @ref bs::PARTICLE_COLLISIONS_DESC::dampening "PARTICLE_COLLISIONS_DESC::dampening" determines how much energy will particles lose after a collision.

Sometimes it is useful to reduce particle lifetime after a collision, or kill them entirely. Use @ref bs::PARTICLE_COLLISIONS_DESC::lifetimeLoss "PARTICLE_COLLISIONS_DESC::lifetimeLoss" to specify how much lifetime is lost (in range [0, 1]) after a collision. Lifetime loss of 1 will destroy the particle upon collision.

~~~~~~~~~~~~~{.cpp}
// Create a plane collider
PARTICLE_COLLISONS_DESC desc;
desc.mode = ParticleCollisionMode::Plane;
desc.radius = 0.2f;

auto evolver = ParticleCollisions::create(desc);

// Add a single plane to collide with
evolver->setPlanes( { Plane(Vector3::UNIT_Y, 0.0f)});
~~~~~~~~~~~~~

~~~~~~~~~~~~~{.cpp}
// Create a world collider
PARTICLE_COLLISONS_DESC desc;
desc.mode = ParticleCollisionMode::World;
desc.radius = 0.2f;

auto evolver = ParticleCollisions::create(desc);
~~~~~~~~~~~~~

# Texture animation {#particleEvolvers_i}
Texture animation evolver allows you to animate the texture applied to the particle.

Represented with the @ref bs::ParticleTextureAnimation "ParticleTextureAnimation" and initialization options provided through @ref bs::PARTICLE_TEXTURE_ANIMATION_DESC "PARTICLE_TEXTURE_ANIMATION_DESC".

Texture animation evolver only works if you have provided a **SpriteTexture** with animation to the **Material** the particle is rendered with. You can find more about sprite texture animation in [material](@ref simpleMaterial) manual.

Use @ref bs::PARTICLE_TEXTURE_ANIMATION_DESC::numCycles "PARTICLE_TEXTURE_ANIMATION_DESC::numCycles" to specify how many times should the animation loop during particle's lifetime. Enable @ref bs::PARTICLE_TEXTURE_ANIMATION_DESC::randomizeRow "PARTICLE_TEXTURE_ANIMATION_DESC::randomizeRow" if you want every particle to pick a random row from the relevant sprite texture. This allows you to provide different textures and animations to different particles.

~~~~~~~~~~~~~{.cpp}
HTexture texture = ...; // Import a texture (or create one)

// Create a sprite texture with animation
HSpriteTexture spriteTexture = SpriteTexture::create(texture);
spriteTexture->setTexture(particleSpriteSheetTex);

// 4x4 grid with a total of 16 frames, running at 16 frames per second
SpriteSheetGridAnimation gridAnim(4, 4, 16, 16);
spriteTexture->setAnimation(gridAnim);
spriteTexture->setAnimationPlayback(SpriteAnimationPlayback::Normal)

// Assign the sprite texture to the particle material
HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::ParticlesUnlit);		
HMaterial material = Material::create(shader);
material->setSpriteTexture("gTexture", spriteTexture);

// Set the material to be used by the particle system
ParticleSystemSettings psSettings;
psSettings.material = particleMaterial;

particleSystem->setSettings(psSettings);

// Create and add a texture animation evolver
PARTICLE_TEXTURE_ANIMATION_DESC desc;
desc.numCycles = 5;

auto evolver = ParticleTextureAnimation::create(desc);
particleSystem->setEvolvers({evolver});
~~~~~~~~~~~~~