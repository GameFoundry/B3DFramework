---
title: Particle evolvers
---

Many of the particle properties can be controlled directly from the **ParticleEmitter**, such as particle color, size and velocity, but those properties will only affect the initial state of the particle. If you need to modify the particle state during its lifetime you must use evolvers. 

Evolvers can provide various functionality such as:
 - Changing the particle color, size, rotation, velocity over the lifetime
 - Applying gravity or arbitrary forces
 - Colliding particles with the world
 - Animating particle textures

All evolvers derive from @b3d::ParticleEvolver. They are created and registered with the particle system similarly to emitters. Use @b3d::ParticleSystem::SetEvolvers() to set a list of evolvers to use for a particular particle system. All evolver types have a **Create()** method and accept a settings structure that allows you to customize their properties.

~~~~~~~~~~~~~{.cpp}
// An example creating and registering an evolver that modifies particle size over lifetime
ParticleSizeSettings settings;
settings.Size = TAnimationCurve<float>(
{
	TKeyframe<float>{1.0f, 0.0f, 1.0f, 0.0f},
	TKeyframe<float>{4.0f, 1.0f, 0.0f, 1.0f},
});

SPtr<ParticleEvolver> evolver = ParticleSize::Create(settings);
particleSystem->SetEvolvers({evolver});
~~~~~~~~~~~~~

Query the current evolvers:

~~~~~~~~~~~~~{.cpp}
Vector<SPtr<ParticleEvolver>> evolvers = particleSystem->GetEvolvers();
~~~~~~~~~~~~~

Lets go over all the available evolver types.

# Size over lifetime
Modifies the particle size over the particle lifetime.

Represented with the @b3d::ParticleSize and initialization options provided through @b3d::ParticleSizeSettings.

Uniform size is provided through @b3d::ParticleSizeSettings::Size in the form of a **FloatDistribution**. If @b3d::ParticleSizeSettings::Use3DSize is enabled you may instead provide a separate size value for all three axes in the @b3d::ParticleSizeSettings::Size3D field.

Note that all evolver distributions containing curves will be evaluated in range [0, 1], 0 representing the beginning of the particle lifetime and 1 representing the end.

~~~~~~~~~~~~~{.cpp}
// Scales the particle by 4 over the entirety of the particle's lifetime
ParticleSizeSettings settings;
settings.Size = TAnimationCurve<float>(
{
    TKeyframe<float>{1.0f, 0.0f, 1.0f, 0.0f},
    TKeyframe<float>{4.0f, 1.0f, 0.0f, 1.0f},
});

SPtr<ParticleSize> evolver = ParticleSize::Create(settings);
~~~~~~~~~~~~~

# Color over lifetime
Modifies the particle color over the particle lifetime.

Represented with the @b3d::ParticleColor and initialization options provided through @b3d::ParticleColorSettings.

Color is provided through @b3d::ParticleColorSettings::Color in the form of a **ColorDistribution**, and is the only available property for this evolver.

~~~~~~~~~~~~~{.cpp}
// Fades from white to black over the particle's lifetime
ParticleColorSettings settings;
settings.Color = ColorGradient(
{
    ColorGradientKey(Color::kWhite, 0.0f),
    ColorGradientKey(Color::kBlack, 1.0f)
});

SPtr<ParticleColor> evolver = ParticleColor::Create(settings);
~~~~~~~~~~~~~

# Rotation over lifetime
Rotates the particle over the particle lifetime.

Represented with the @b3d::ParticleRotation and initialization options provided through @b3d::ParticleRotationSettings.

Rotation around Z axis is provided through @b3d::ParticleRotationSettings::Rotation in the form of a **FloatDistribution**, containing angle in degrees. If @b3d::ParticleRotationSettings::Use3DRotation is enabled you may instead provide a separate rotation value for each of the three axes in the @b3d::ParticleRotationSettings::Rotation3D field.

~~~~~~~~~~~~~{.cpp}
// Rotates the particle by 180 degrees over its lifetime
ParticleRotationSettings settings;
settings.Rotation = TAnimationCurve<float>(
{
     TKeyframe<float>{0.0f, 0.0f, 1.0f, 0.0f},
     TKeyframe<float>{180.0f, 1.0f, 0.0f, 1.0f},
});

SPtr<ParticleRotation> evolver = ParticleRotation::Create(settings);
~~~~~~~~~~~~~

# Gravity
Applies the force of gravity to particles.

Represented with the @b3d::ParticleGravity and initialization options provided through @b3d::ParticleGravitySettings.

The gravity force is inherited from the physics system, but can be scaled for the purposes of this evolver by setting @b3d::ParticleGravitySettings::Scale.

~~~~~~~~~~~~~{.cpp}
// Applies the default gravity
ParticleGravitySettings settings;
settings.Scale = 1.0f;

SPtr<ParticleGravity> evolver = ParticleGravity::Create(settings);
~~~~~~~~~~~~~

# Force
Applies an arbitrary force to particles.

Represented with the @b3d::ParticleForce and initialization options provided through @b3d::ParticleForceSettings.

Use @b3d::ParticleForceSettings::Force to specify the force direction and intensity. If @b3d::ParticleForceSettings::WorldSpace is true the force direction is assumed to be provided in world space, otherwise it is assumed to be relative to the transform of the parent particle system.

~~~~~~~~~~~~~{.cpp}
// Applies a force to particles, pushing them towards the positive X axis
ParticleForceSettings settings;
settings.Force = TAnimationCurve<Vector3>(
{
    TKeyframe<Vector3>{Vector3::kZero, Vector3::kZero, Vector3::kOne, 0.0f},
    TKeyframe<Vector3>{Vector3(100.0f, 0.0f, 0.0f), -Vector3::kOne, Vector3::kZero, 0.5f},
});
settings.WorldSpace = true;

SPtr<ParticleForce> evolver = ParticleForce::Create(settings);
~~~~~~~~~~~~~

# Velocity
Sets the velocity of particles over their lifetime.

![Velocity evolver](../../Images/velocityEvolver.gif)

Represented with the @b3d::ParticleVelocity and initialization options provided through @b3d::ParticleVelocitySettings.

Use @b3d::ParticleVelocitySettings::Velocity to specify the velocity direction and intensity. If @b3d::ParticleVelocitySettings::WorldSpace is true the velocity direction is assumed to be provided in world space, otherwise it is assumed to be relative to the transform of the parent particle system.

~~~~~~~~~~~~~{.cpp}
// Moves the particles upwards
ParticleVelocitySettings settings;
settings.Velocity = Vector3(0.0f, 0.2f, 0.0f);

SPtr<ParticleVelocity> evolver = ParticleVelocity::Create(settings);
~~~~~~~~~~~~~

# Orbit
Sets angular velocity of particles as if they were orbiting a point in space.

Represented with the @b3d::ParticleOrbit and initialization options provided through @b3d::ParticleOrbitSettings.

Use @b3d::ParticleOrbitSettings::Center to specify a point to orbit around. If @b3d::ParticleOrbitSettings::WorldSpace is true the point is assumed to be provided in world space, otherwise it is assumed to be relative to the transform of the parent particle system.

Specify the speed of rotation by setting @b3d::ParticleOrbitSettings::Velocity which allows you to set the speed of rotation for each axis separately, in rotations per second. Evaluated in local or world space according to **ParticleOrbitSettings::WorldSpace**.

By default the particles will orbit at a fixed distance from the point, but you can also make them move away or be drawn to the point by setting the @b3d::ParticleOrbitSettings::Radial field to a positive (move away) or negative (move closer) value.

~~~~~~~~~~~~~{.cpp}
// Rotates the particles over the local Y axis and slowly moves them away from the center
ParticleOrbitSettings settings;
settings.Center = Vector3(0.0f, 0.0f, 0.0f);
settings.Velocity = Vector3(0.0f, 1.2f, 0.0f); // In rotations/second
settings.Radial = 0.4f;

SPtr<ParticleOrbit> evolver = ParticleOrbit::Create(settings);
~~~~~~~~~~~~~

# Collision
Makes the particles collide with the world or a user-provided set of planes.

![Particle collisions](../../Images/particleCollision.gif)

Represented with the @b3d::ParticleCollisions and initialization options provided through @b3d::ParticleCollisionSettings.

This evolver comes with two modes, settable through @b3d::ParticleCollisionSettings::Mode:
 - @b3d::ParticleCollisionMode::World - Particles will collide with all world geometry as defined by the physics components placed in the world. Optionally use bitmask @b3d::ParticleCollisionSettings::Layer to collide only with physics objects with a matching layer.
 - @b3d::ParticleCollisionMode::Plane - This is a secondary collision mode that makes the particles collide only with a user-defined set of planes. This mode is cheaper performance-wise than world collisions. Use @b3d::ParticleCollisions::SetPlanes to provide an array of planes to collide with.

Additionally, the following properties are used in either collision mode. @b3d::ParticleCollisionSettings::Radius specifies the size of individual particle (defined as a sphere). @b3d::ParticleCollisionSettings::Restitution determines how much will particles bounce after a collision, and @b3d::ParticleCollisionSettings::Dampening determines how much energy will particles lose after a collision.

Sometimes it is useful to reduce particle lifetime after a collision, or kill them entirely. Use @b3d::ParticleCollisionSettings::LifetimeLoss to specify how much lifetime is lost (in range [0, 1]) after a collision. Lifetime loss of 1 will destroy the particle upon collision.

~~~~~~~~~~~~~{.cpp}
// Create a plane collider
ParticleCollisionSettings settings;
settings.Mode = ParticleCollisionMode::Plane;
settings.Radius = 0.2f;

SPtr<ParticleCollisions> evolver = ParticleCollisions::Create(settings);

// Add a single plane to collide with
evolver->SetPlanes({Plane(Vector3::kUnitY, 0.0f)});
~~~~~~~~~~~~~

~~~~~~~~~~~~~{.cpp}
// Create a world collider
ParticleCollisionSettings settings;
settings.Mode = ParticleCollisionMode::World;
settings.Radius = 0.2f;

SPtr<ParticleCollisions> evolver = ParticleCollisions::Create(settings);
~~~~~~~~~~~~~

# Texture animation
Texture animation evolver allows you to animate the texture applied to the particle.

Represented with the @b3d::ParticleTextureAnimation and initialization options provided through @b3d::ParticleTextureAnimationSettings.

Texture animation evolver only works if you have provided a **SpriteTexture** with animation to the **Material** the particle is rendered with. You can find more about sprite texture animation in [material](../Rendering/simpleMaterial) manual.

Use @b3d::ParticleTextureAnimationSettings::CycleCount to specify how many times should the animation loop during particle's lifetime. Enable @b3d::ParticleTextureAnimationSettings::RandomizeRow if you want every particle to pick a random row from the relevant sprite texture. This allows you to provide different textures and animations to different particles.

~~~~~~~~~~~~~{.cpp}
HTexture texture = ...; // Import a texture (or create one)

// Create a sprite texture with animation
HSpriteTexture spriteTexture = SpriteTexture::Create(texture);
spriteTexture->SetTexture(texture);

// 4x4 grid with a total of 16 frames, running at 16 frames per second
SpriteSheetGridAnimation gridAnimation(4, 4, 16, 16);
spriteTexture->SetAnimation(gridAnimation);
spriteTexture->SetAnimationPlayback(SpriteAnimationPlayback::Normal);

// Assign the sprite texture to the particle material
HShader shader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::ParticlesUnlit);
HMaterial material = Material::Create(shader);
material->SetSpriteTexture("gTexture", spriteTexture);

// Set the material to be used by the particle system
ParticleSystemSettings particleSystemSettings;
particleSystemSettings.Material = material;

particleSystem->SetSettings(particleSystemSettings);

// Create and add a texture animation evolver
ParticleTextureAnimationSettings settings;
settings.CycleCount = 5;

SPtr<ParticleTextureAnimation> evolver = ParticleTextureAnimation::Create(settings);
particleSystem->SetEvolvers({evolver});
~~~~~~~~~~~~~
