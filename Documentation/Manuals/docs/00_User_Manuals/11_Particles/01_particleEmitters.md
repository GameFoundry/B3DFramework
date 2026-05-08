---
title: Emitter shapes
---

**ParticleEmitter** object can be initialized with a variety of shapes, either primitives or meshes. Those shapes control the positions and normals (travel direction) for any spawned particles.

A shape is assigned to the emitter through the @b3d::ParticleEmitter::SetShape method. All shape types derive from @b3d::ParticleEmitterShape class. All shape classes have a **Create()** method that accepts a structure with various options allowing you to further customize the properties of the spawned particles.

~~~~~~~~~~~~~{.cpp}
TShared<ParticleEmitter> emitter = B3DMakeShared<ParticleEmitter>();

// An emitter with the sphere shape
ParticleSphereShapeSettings sphereShape;
sphereShape.Radius = 0.3f;
emitter->SetShape(ParticleEmitterSphereShape::Create(sphereShape));
~~~~~~~~~~~~~

In this manual we'll go over all the available shapes.

# Sphere
Emits particles from a sphere shape. Particles are spawned either on sphere surface, its shell or the entirety of the sphere volume. Particle normals are set to be outward facing from the sphere center.

Represented with the @b3d::ParticleEmitterSphereShape and initialization options provided through @b3d::ParticleSphereShapeSettings.

Use @b3d::ParticleSphereShapeSettings::Radius to control the size of the sphere, and @b3d::ParticleSphereShapeSettings::Thickness to control from which part of the sphere to spawn the particles. 0 means spawn from sphere surface, 1 means spawn on entire sphere volume, while values between represent an area of the sphere shell of some thickness.

~~~~~~~~~~~~~{.cpp}
ParticleSphereShapeSettings shapeSettings;
shapeSettings.Radius = 0.5f; // Size of the sphere
shapeSettings.Thickness = 0.1f; // Spawn particles on sphere shell (shell width being 10% of sphere radius)

TShared<ParticleEmitterSphereShape> shape = ParticleEmitterSphereShape::Create(shapeSettings);
~~~~~~~~~~~~~

# Hemisphere
Equivalent in every way to the sphere shape, except the particles are only emitted from one half of the sphere.

Represented with the @b3d::ParticleEmitterHemisphereShape and initialization options provided through @b3d::ParticleHemisphereShapeSettings.

~~~~~~~~~~~~~{.cpp}
ParticleHemisphereShapeSettings shapeSettings;
shapeSettings.Radius = 0.5f; // Size of the hemisphere
shapeSettings.Thickness = 0.1f; // Spawn particles on hemisphere shell (shell width being 10% of hemisphere radius)

TShared<ParticleEmitterHemisphereShape> shape = ParticleEmitterHemisphereShape::Create(shapeSettings);
~~~~~~~~~~~~~

# Cone
Emits particles from a cone or a conical frustum (cone with a top cut off). The particles can be spawned on the cone base (the pointy bit), entire cone volume, or cone shell (some percent of the volume).

Represented with the @b3d::ParticleEmitterConeShape and initialization options provided through @b3d::ParticleConeShapeSettings.

Use @b3d::ParticleConeShapeSettings::Angle and @b3d::ParticleConeShapeSettings::Length to control the basic shape of the cone.

~~~~~~~~~~~~~{.cpp}
ParticleConeShapeSettings shapeSettings;
shapeSettings.Angle = Degree(45.0f);
shapeSettings.Length = 1.0f;

TShared<ParticleEmitterConeShape> shape = ParticleEmitterConeShape::Create(shapeSettings);
~~~~~~~~~~~~~

Use @b3d::ParticleConeShapeSettings::Type to control should particles be spawned on the cone volume, or only its base.

~~~~~~~~~~~~~{.cpp}
shapeSettings.Type = ParticleEmitterConeType::Base; // Spawn only on the base
//shapeSettings.Type = ParticleEmitterConeType::Volume; // Or the entire volume
~~~~~~~~~~~~~
 
## Base
If spawning on the base, the length parameter of the cone is ignored, while the angle parameter only controls the direction (normals) of the particles.

Normally the cone base is pointy, meaning all your particles will spawn on the same spot. You can set a non-zero @b3d::ParticleConeShapeSettings::Radius parameter which will cut off the cone top, making a conical frustum. Your particles will then spawn anywhere on the base within the radius.

~~~~~~~~~~~~~{.cpp}
shapeSettings.Radius = 0.2f;
~~~~~~~~~~~~~

## Volume
If spawning on the volume the particles will spawn anywhere within the cone volume. Similar to the sphere, you can use @b3d::ParticleConeShapeSettings::Thickness parameter to limit spawning on an outer part of the volume (values between 0 and 1), entire volume (value 1) or just the surface (value 0).

Additionally you can also control the angular portion of the cone that can spawn particles through @b3d::ParticleConeShapeSettings::Arc (normal cone having a 360 degree angle).

~~~~~~~~~~~~~{.cpp}
shapeSettings.Thickness = 0.1f;
shapeSettings.Arc = Degree(180.0f); // Spawn only on one half of the cone
~~~~~~~~~~~~~

## Emission mode
Finally, you get to control the emission mode through @b3d::ParticleConeShapeSettings::Mode. Emission mode gives you more control over particle spawning by providing a way to spawn particles non-randomly (sequentially).

@b3d::ParticleEmissionMode::Type controls which of the emission mode to use. Use @b3d::ParticleEmissionModeType::Loop and @b3d::ParticleEmissionModeType::PingPong to spawn particles sequentially on the shape. Use @b3d::ParticleEmissionMode::Interval to control how far apart should the spawned particles be, and @b3d::ParticleEmissionMode::Speed to control how fast should the particles move around the shape.

Use @b3d::ParticleEmissionModeType::Random to spawn the particles randomly, which is the default behaviour. You also get extra control to limit the random spawns to a specific interval through **ParticleEmissionMode::Interval**, which means the particles will only spawn on certain positions on the shape.

Finally @b3d::ParticleEmissionModeType::Spread will spread out all particles spawned during a single burst over the entire shape, optionally using **ParticleEmissionMode::Interval** to limit the spawn positions.

~~~~~~~~~~~~~{.cpp}
// Spawn particles moving around the cone base
shapeSettings.Mode.Type = ParticleEmissionModeType::Loop;
shapeSettings.Mode.Speed = 90.0f; // 90 degrees per second
shapeSettings.Mode.Interval = 10.0f; // At 10 degree intervals
~~~~~~~~~~~~~

# Box
Emits particles from a box shape. Particles are spawned either on box volume, surface or its edges. Particle normals are set to the positive Z direction always.

Represented with the @b3d::ParticleEmitterBoxShape and initialization options provided through @b3d::ParticleBoxShapeSettings.

Use @b3d::ParticleBoxShapeSettings::Extents to specify the size of the box, and @b3d::ParticleBoxShapeSettings::Type to control from which part of the box should the particles be emitted from.

~~~~~~~~~~~~~{.cpp}
ParticleBoxShapeSettings shapeSettings;
shapeSettings.Extents = Vector3::kOne * 0.5f; // Unit-sized box
shapeSettings.Type = ParticleEmitterBoxType::Volume; // Spawn in the entire box volume

TShared<ParticleEmitterBoxShape> shape = ParticleEmitterBoxShape::Create(shapeSettings);
~~~~~~~~~~~~~

# Circle
Emits particles from a 2D circle shape. Particles are spawned either on circle edge, its entire surface or a circle shell. Particle normals are set to the positive Z direction always.

Represented with the @b3d::ParticleEmitterCircleShape and initialization options provided through @b3d::ParticleCircleShapeSettings.

Use @b3d::ParticleCircleShapeSettings::Radius to control the size of the circle, and @b3d::ParticleCircleShapeSettings::Thickness to control from which part of the circle to spawn the particles. 0 means spawn from circle edge, 1 means spawn on entire circle surface, while values between represent an area of the circle shell of some thickness.

Additionally you can also control the angular portion of the circle that can spawn particles through @b3d::ParticleCircleShapeSettings::Arc (normal circle having a 360 degree angle).

Finally you can control emission mode through @b3d::ParticleCircleShapeSettings::Mode. The description of mode parameter is the same as for the cone, except the particles will be spawned along the circle edge instead of the cone base, and the *speed/interval* parameters represent distance instead of an angle.

~~~~~~~~~~~~~{.cpp}
ParticleCircleShapeSettings shapeSettings;
shapeSettings.Radius = 1.0f;
shapeSettings.Arc = Degree(300.0f); // "Pie" shape (circle with a part cut out)
shapeSettings.Thickness = 0.0f; // Spawn only on circle edges

TShared<ParticleEmitterCircleShape> shape = ParticleEmitterCircleShape::Create(shapeSettings);
~~~~~~~~~~~~~

# Rectangle
Emits particles from a 2D rectangle shape. Particles are always spawned on rectangle surface. Particle normals are set to the positive Z direction always.

Represented with the @b3d::ParticleEmitterRectShape and initialization options provided through @b3d::ParticleRectShapeSettings.

Use @b3d::ParticleRectShapeSettings::Extents to control the size of the rectangle, which is also the only option provided by this shape.

~~~~~~~~~~~~~{.cpp}
ParticleRectShapeSettings shapeSettings;
shapeSettings.Extents = Vector2::kOne * 0.5f;

TShared<ParticleEmitterRectShape> shape = ParticleEmitterRectShape::Create(shapeSettings);
~~~~~~~~~~~~~

# Line
Emits particles from a line shape. Particle normals are set to the positive Z direction always.

Represented with the @b3d::ParticleEmitterLineShape and initialization options provided through @b3d::ParticleLineShapeSettings.

Use @b3d::ParticleLineShapeSettings::Length to control the length of the line. You can also control emission mode through @b3d::ParticleLineShapeSettings::Mode. The description of mode parameter is the same as for the cone and circle, except the particles will be spawned along the line instead of the cone/circle.

~~~~~~~~~~~~~{.cpp}
ParticleLineShapeSettings shapeSettings;
shapeSettings.Length = 1.0f;

TShared<ParticleEmitterLineShape> shape = ParticleEmitterLineShape::Create(shapeSettings);
~~~~~~~~~~~~~

# Static mesh
Emits particles from a surface of a static (non-animated) mesh. Particles can be emitted from anywhere on the surface, or limited to mesh vertices or edges. Particle normals are set to the normals specified by the mesh.

Represented with the @b3d::ParticleEmitterStaticMeshShape and initialization options provided through @b3d::ParticleStaticMeshShapeSettings.

Use @b3d::ParticleStaticMeshShapeSettings::Mesh to specify the mesh resource to emit from. This option is required. The mesh should ideally be imported with CPU caching enabled, so its data can be read by the particle system without having to do an expensive GPU read.

Use @b3d::ParticleStaticMeshShapeSettings::Type to control should the emission happen on mesh triangles (entire surface), edges or vertices.

Finally, enable @b3d::ParticleStaticMeshShapeSettings::Sequential if you want particle positions to be chosen in the order they are specified in the mesh, instead of randomly. This option is only relevant when emitting from mesh vertices and allows you to provide an exact list of vertices to emit particles at, for complete control over their spawning process.

~~~~~~~~~~~~~{.cpp}
HMesh mesh = ...; // Import or create a mesh

ParticleStaticMeshShapeSettings shapeSettings;
shapeSettings.Mesh = mesh;
shapeSettings.Type = ParticleEmitterMeshType::Triangle;

TShared<ParticleEmitterStaticMeshShape> shape = ParticleEmitterStaticMeshShape::Create(shapeSettings);
~~~~~~~~~~~~~

# Skinned mesh
Provides the exact same functionality as the static mesh shape, except the mesh can be animated.

Represented with the @b3d::ParticleEmitterSkinnedMeshShape and initialization options provided through @b3d::ParticleSkinnedMeshShapeSettings.

Accepts the same options as the static mesh shape, except for the *mesh* property it accepts a **Renderable** in @b3d::ParticleSkinnedMeshShapeSettings::Renderable.

~~~~~~~~~~~~~{.cpp}
HRenderable renderable = ...; // Set up the component and animate it

ParticleSkinnedMeshShapeSettings shapeSettings;
shapeSettings.Renderable = renderable;
shapeSettings.Type = ParticleEmitterMeshType::Triangle;

TShared<ParticleEmitterSkinnedMeshShape> shape = ParticleEmitterSkinnedMeshShape::Create(shapeSettings);
~~~~~~~~~~~~~
