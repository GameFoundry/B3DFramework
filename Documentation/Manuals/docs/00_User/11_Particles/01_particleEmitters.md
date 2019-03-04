Emitter shapes 			{#particleEmitters}
===============
[TOC]

**ParticleEmitter** object can be initialized with a variety of shapes, either primitives or meshes. Those shapes control the positions and normals (travel direction) for any spawned particles. 

A shape is assigned to the emitter through the @ref bs::ParticleEmitter::setShape "ParticleEmitter::setShape()" method. All shape types derive from @ref bs::ParticleEmitterShape "ParticleEmitterShape" class. All shape classes have a **create()** method that accepts a structure with various options allowing you to further customize the properties of the spawned particles.

~~~~~~~~~~~~~{.cpp}
SPtr<ParticleEmitter> emitter = bs_shared_ptr_new<ParticleEmitter>();

// An emitter with the sphere shape
PARTICLE_SPHERE_SHAPE_DESC sphereShape;
sphereShape.radius = 0.3f;
emitter->setShape(ParticleEmitterSphereShape::create(sphereShape));
~~~~~~~~~~~~~

In this manual we'll go over all the available shapes.

# Sphere {#particleEmitters_a}
Emits particles from a sphere shape. Particles are spawned either on sphere surface, its shell or the entirety of the sphere volume. Particle normals are set to be outward facing from the sphere center. 

Represented with the @ref bs::ParticleEmitterSphereShape "ParticleEmitterSphereShape" and initialization options provided through @ref bs::PARTICLE_SPHERE_SHAPE_DESC "PARTICLE_SPHERE_SHAPE_DESC". 

Use @ref bs::PARTICLE_SPHERE_SHAPE_DESC::radius "PARTICLE_SPHERE_SHAPE_DESC::radius" to control the size of the sphere, and @ref bs::PARTICLE_SPHERE_SHAPE_DESC::thickness "PARTICLE_SPHERE_SHAPE_DESC::thickness" to control from which part of the sphere to spawn the particles. 0 means spawn from sphere surface, 1 means spawn on entire sphere volume, while values between represent an area of the sphere shell of some thickness.

~~~~~~~~~~~~~{.cpp}
PARTICLE_SPHERE_SHAPE_DESC shapeDesc;
shapeDesc.radius = 0.5f; // Size of the sphere
shapeDesc.thickness = 0.1f; // Spawn particles on sphere shell (shell width being 10% of sphere radius)

auto shape = ParticleEmitterSphereShape::create(shapeDesc);
~~~~~~~~~~~~~

# Hemisphere {#particleEmitters_b}
Equivalent in every way to the sphere shape, except the particles are only emitted from one half of the sphere.

Represented with the @ref bs::ParticleEmitterHemisphereShape "ParticleEmitterHemisphereShape" and initialization options provided through @ref bs::PARTICLE_HEMISPHERE_SHAPE_DESC "PARTICLE_HEMISPHERE_SHAPE_DESC". 

~~~~~~~~~~~~~{.cpp}
PARTICLE_HEMISPHERE_SHAPE_DESC shapeDesc;
shapeDesc.radius = 0.5f; // Size of the hemisphere
shapeDesc.thickness = 0.1f; // Spawn particles on hemisphere shell (shell width being 10% of hemisphere radius)

auto shape = ParticleEmitterHemisphereShape::create(shapeDesc);
~~~~~~~~~~~~~

# Cone {#particleEmitters_c}
Emits particles from a cone or a conical frustum (cone with a top cut off). The particles can be spawned on the cone base (the pointy bit), entire cone volume, or cone shell (some percent of the volume). 

Represented with the @ref bs::ParticleEmitterConeShape "ParticleEmitterConeShape" and initialization options provided through @ref bs::PARTICLE_CONE_SHAPE_DESC "PARTICLE_CONE_SHAPE_DESC". 

Use @ref bs::PARTICLE_CONE_SHAPE_DESC::angle "bs::PARTICLE_CONE_SHAPE_DESC::angle" and @ref bs::PARTICLE_CONE_SHAPE_DESC::length "PARTICLE_CONE_SHAPE_DESC::length" to control the basic shape of the cone. 

~~~~~~~~~~~~~{.cpp}
PARTICLE_CONE_SHAPE_DESC shapeDesc;
shapeDesc.angle = Degree(45.0f);
shapeDesc.length = 1.0f;

auto shape = ParticleEmitterConeShape::create(shapeDesc);
~~~~~~~~~~~~~

Use @ref bs::PARTICLE_CONE_SHAPE_DESC::type "PARTICLE_CONE_SHAPE_DESC::type" to control should particles be spawned on the cone volume, or only its base.

~~~~~~~~~~~~~{.cpp}
shapeDesc.type = ParticleEmitterConeType::Base; // Spawn only on the base
//shapeDesc.type = ParticleEmitterConeType::Volume; // Or the entire volume
~~~~~~~~~~~~~
 
## Base {#particleEmitters_c_a}
If spawning on the base, the length parameter of the cone is ignored, while the angle parameter only controls the direction (normals) of the particles.

Normally the cone base is pointy, meaning all your particles will spawn on the same spot. You can set a non-zero @ref bs::PARTICLE_CONE_SHAPE_DESC::radius "PARTICLE_CONE_SHAPE_DESC::radius" parameter which will cut off the cone top, making a conical frustum. Your particles will then spawn anywhere on the base within the radius.

~~~~~~~~~~~~~{.cpp}
shapeDesc.radius = 0.2f;
~~~~~~~~~~~~~

## Volume {#particleEmitters_c_b}
If spawning on the volume the particles will spawn anywhere within the cone volume. Similar to the sphere, you can use @ref bs::PARTICLE_CONE_SHAPE_DESC::thickness "PARTICLE_CONE_SHAPE_DESC::thickness" parameter to limit spawning on an outer part of the volume (values between 0 and 1), entire volume (value 1) or just the surface (value 0).

Additionally you can also control the angular portion of the cone that can spawn particles through @ref bs::PARTICLE_CONE_SHAPE_DESC::arc "PARTICLE_CONE_SHAPE_DESC::arc" (normal cone having a 360 degree angle). 

~~~~~~~~~~~~~{.cpp}
shapeDesc.thickness = 0.1f;
shapeDesc.arc = Degree(180.0f); // Spawn only on one half of the cone
~~~~~~~~~~~~~

## Emission mode {#particleEmitters_c_c}
Finally, you get to control the emission mode through @ref bs::PARTICLE_CONE_SHAPE_DESC::mode "PARTICLE_CONE_SHAPE_DESC::mode". Emission mode gives you more control over particle spawning by providing a way to spawn particles non-randomly (sequentially).

@ref bs::ParticleEmissionMode::type "ParticleEmissionMode::type" controls which of the emission mode to use. Use @ref bs::ParticleEmissionModeType::Loop "ParticleEmissionModeType::Loop" and @ref bs::ParticleEmissionModeType::PingPong "ParticleEmissionModeType::PingPong" to spawn particles sequentially on the shape. Use @ref bs::ParticleEmissionMode::interval "ParticleEmissionMode::interval" to control how far apart should the spawned particles be, and @ref bs::ParticleEmissionMode::speed "ParticleEmissionMode::speed" to control how fast should the particles move around the shape.

Use @ref bs::ParticleEmissionModeType::Random "ParticleEmissionModeType::Random" to spawn the particles randomly, which is the default behaviour. You also get extra control to limit the random spawns to a specific interval through **ParticleEmissionMode::interval**, which means the particles will only spawn on certain positions on the shape.

Finally @ref bs::ParticleEmissionModeType::Spread "ParticleEmissionModeType::Spread" will spread out all particles spawned during a single burst over the entire shape, optionally using **ParticleEmissionMode::interval** to limit the spawn positions.

~~~~~~~~~~~~~{.cpp}
// Spawn particles moving around the cone base
shapeDesc.mode.type = ParticleEmissionMode::Type::Loop;
shapeDesc.mode.speed = 90.0f; // 90 degrees per second
shapeDesc.mode.interval = 10.0f; // At 10 degree intervals
~~~~~~~~~~~~~

# Box {#particleEmitters_d}
Emits particles from a box shape. Particles are spawned either on box volume, surface or its edges. Particle normals are set to the positive Z direction always.

Represented with the @ref bs::ParticleEmitterBoxShape "ParticleEmitterBoxShape" and initialization options provided through @ref bs::PARTICLE_BOX_SHAPE_DESC "PARTICLE_BOX_SHAPE_DESC". 

Use @ref bs::PARTICLE_BOX_SHAPE_DESC::extents "PARTICLE_BOX_SHAPE_DESC::extents" to specify the size of the box, and @ref bs::PARTICLE_BOX_SHAPE_DESC::type "PARTICLE_BOX_SHAPE_DESC::type" to control from which part of the box should the particles be emitted from.

~~~~~~~~~~~~~{.cpp}
PARTICLE_BOX_SHAPE_DESC shapeDesc;
shapeDesc.extents = Vector3::ONE * 0.5f; // Unit-sized box
shapeDesc.type = ParticleEmitterBoxType::Volume; // Spawn in the entire box volume

auto shape = ParticleEmitterBoxShape::create(shapeDesc);
~~~~~~~~~~~~~

# Circle {#particleEmitters_e}
Emits particles from a 2D circle shape. Particles are spawned either on circle edge, its entire surface or a circle shell. Particle normals are set to the positive Z direction always.

Represented with the @ref bs::ParticleEmitterCircleShape "ParticleEmitterCircleShape" and initialization options provided through @ref bs::PARTICLE_CIRCLE_SHAPE_DESC "PARTICLE_CIRCLE_SHAPE_DESC". 

Use @ref bs::PARTICLE_CIRCLE_SHAPE_DESC::radius "PARTICLE_CIRCLE_SHAPE_DESC::radius" to control the size of the circle, and @ref bs::PARTICLE_CIRCLE_SHAPE_DESC::thickness "PARTICLE_CIRCLE_SHAPE_DESC::thickness" to control from which part of the circle to spawn the particles. 0 means spawn from circle edge, 1 means spawn on entire circle surface, while values between represent an area of the circle shell of some thickness.

Additionally you can also control the angular portion of the circle that can spawn particles through @ref bs::PARTICLE_CONE_SHAPE_DESC::arc "PARTICLE_CONE_SHAPE_DESC::arc" (normal circle having a 360 degree angle). 

Finally you can control emission mode through @ref bs::PARTICLE_CIRCLE_SHAPE_DESC::mode "PARTICLE_CIRCLE_SHAPE_DESC::mode". The description of mode parameter is the same as for the cone, except the particles will be spawned along the circle edge instead of the cone base, and the *speed/interval* parameters represent distance instead of an angle.

~~~~~~~~~~~~~{.cpp}
PARTICLE_CIRCLE_SHAPE_DESC shapeDesc;
shapeDesc.radius = 1.0f;
shapeDesc.arc = Degree(300.0f); // "Pie" shape (circle with a part cut out)
shapeDesc.thickness = 0.0f; // Spawn only on circle edges

auto shape = ParticleEmitterCircleShape::create(shapeDesc);
~~~~~~~~~~~~~

# Rectangle {#particleEmitters_f}
Emits particles from a 2D rectangle shape. Particles are always spawned on rectangle surface. Particle normals are set to the positive Z direction always.

Represented with the @ref bs::ParticleEmitterRectShape "ParticleEmitterRectShape" and initialization options provided through @ref bs::PARTICLE_RECT_SHAPE_DESC "PARTICLE_RECT_SHAPE_DESC". 

Use @ref bs::PARTICLE_RECT_SHAPE_DESC::extents "PARTICLE_RECT_SHAPE_DESC::extents" to control the size of the rectangle, which is also the only option provided by this shape.

~~~~~~~~~~~~~{.cpp}
PARTICLE_RECT_SHAPE_DESC shapeDesc;
shapeDesc.extents = Vector2::ONE * 0.5f;

auto shape = ParticleEmitterRectShape::create(shapeDesc);
~~~~~~~~~~~~~

# Line {#particleEmitters_g}
Emits particles from a line shape. Particle normals are set to the positive Z direction always.

Represented with the @ref bs::ParticleEmitterLineShape "ParticleEmitterLineShape" and initialization options provided through @ref bs::PARTICLE_LINE_SHAPE_DESC "PARTICLE_LINE_SHAPE_DESC". 

Use @ref bs::PARTICLE_LINE_SHAPE_DESC::length "PARTICLE_RECT_SHAPE_DESC::length" to control the length of the line. You can also control emission mode through @ref bs::PARTICLE_LINE_SHAPE_DESC::mode "PARTICLE_LINE_SHAPE_DESC::mode". The description of mode parameter is the same as for the cone and circle, except the particles will be spawned along the line instead of the cone/circle.

~~~~~~~~~~~~~{.cpp}
PARTICLE_LINE_SHAPE_DESC shapeDesc;
shapeDesc.length = 1.0f;

auto shape = ParticleEmitterLineShape::create(shapeDesc);
~~~~~~~~~~~~~

# Static mesh {#particleEmitters_h}
Emits particles from a surface of a static (non-animated) mesh. Particles can be emitted from anywhere on the surface, or limited to mesh vertices or edges. Particle normals are set to the normals specified by the mesh.

Represented with the @ref bs::ParticleEmitterStaticMeshShape "ParticleEmitterStaticMeshShape" and initialization options provided through @ref bs::PARTICLE_STATIC_MESH_SHAPE_DESC "PARTICLE_STATIC_MESH_SHAPE_DESC". 

Use @ref bs::PARTICLE_STATIC_MESH_SHAPE_DESC::mesh "PARTICLE_STATIC_MESH_SHAPE_DESC::mesh" to specify the mesh resource to emit from. This option is required. The mesh should ideally be imported with CPU caching enabled, so its data can be read by the particle system without having to do an expensive GPU read.

Use @ref bs::PARTICLE_STATIC_MESH_SHAPE_DESC::type "PARTICLE_STATIC_MESH_SHAPE_DESC::type" to control should the emission happen on mesh triangles (entire surface), edges or vertices.

Finally, enable @ref bs::PARTICLE_STATIC_MESH_SHAPE_DESC::sequential "PARTICLE_STATIC_MESH_SHAPE_DESC::sequential" if you want particle positions to be chosen in the order they are specified in the mesh, instead of randomly. This option is only relevant when emitting from mesh vertices and allows you to provide an exact list of vertices to emit particles at, for complete control over their spawning process.

~~~~~~~~~~~~~{.cpp}
HMesh mesh = ...; // Import or create a mesh

PARTICLE_STATIC_MESH_SHAPE_DESC shapeDesc;
shapeDesc.mesh = mesh;
shapeDesc.type = ParticleEmitterMeshType::Triangle;

auto shape = ParticleEmitterStaticMeshShape::create(shapeDesc);
~~~~~~~~~~~~~

# Skinned mesh {#particleEmitters_i}
Provides the exact same functionality as the static mesh shape, except the mesh can be animated.

Represented with the @ref bs::ParticleEmitterSkinnedMeshShape "ParticleEmitterSkinnedMeshShape" and initialization options provided through @ref bs::PARTICLE_SKINNED_MESH_SHAPE_DESC "PARTICLE_SKINNED_MESH_SHAPE_DESC". 

Accepts the same options as the static mesh shape, except for the *mesh* property it accepts a **Renderable** in @ref bs::PARTICLE_SKINNED_MESH_SHAPE_DESC::renderable "PARTICLE_SKINNED_MESH_SHAPE_DESC::renderable".

~~~~~~~~~~~~~{.cpp}
HRenderable renderable = ...; // Set up the component and animate it

PARTICLE_SKINNED_MESH_SHAPE_DESC shapeDesc;
shapeDesc.renderable = renderable;
shapeDesc.type = ParticleEmitterMeshType::Triangle;

auto shape = ParticleEmitterSkinnedMeshShape::create(shapeDesc);
~~~~~~~~~~~~~