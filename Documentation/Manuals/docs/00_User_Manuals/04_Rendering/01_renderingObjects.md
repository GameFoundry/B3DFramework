---
title: Renderable object
---

As the name implies, the renderable object allows you to display an object in the scene. It is represented by the @b3d::Renderable component. The component requires you to assign a **Mesh** to render, as well as a **Material** to render the mesh with. Both of these are resources, and we'll explain them in the following chapters.

# Creating a renderable
**Renderable** is created as any component, and requires no additional parameters.

~~~~~~~~~~~~~{.cpp}
HSceneObject renderableSceneObject = SceneObject::Create("3D object");
HRenderable renderable = renderableSceneObject->AddComponent<Renderable>();
~~~~~~~~~~~~~

# Setting it up
Once created you must assign it a **Mesh** to render, and a **Material** to render it with. Use @b3d::Renderable::SetMesh and @b3d::Renderable::SetMaterial.

~~~~~~~~~~~~~{.cpp}
// Create a standard PBR material
HShader shader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::Standard);
HMaterial material = Material::Create(shader);

// Import and assign a texture to the material
HTexture texture = GetImporter().Import<Texture>("myTexture.png");
material->SetTexture("gAlbedoTex", texture);

// Import a mesh
HMesh mesh = GetImporter().Import<Mesh>("myMesh.fbx");

// Create a renderable
HSceneObject renderableSceneObject = SceneObject::Create("3D object");
HRenderable renderable = renderableSceneObject->AddComponent<Renderable>();

// Assign material and mesh to the renderable
renderable->SetMesh(mesh);
renderable->SetMaterial(material);

// Optionally position the renderable in the scene
renderableSceneObject->SetPosition(Vector3(0.0f, 15.0f, 30.0f));
~~~~~~~~~~~~~

> Note that even though we always import resources in these examples, in production code you should load previously saved resources instead of importing them every time.

After the renderable has been set up, it will now be displayed in your camera view (if the camera is facing the direction of the renderable object).

# Multiple materials
In the example above we use a single material for a single mesh, but it can sometimes be useful to use different materials for different parts of the mesh.

It is up to the artist (creator of the mesh) to specify the regions of the mesh that will use separate materials. Upon mesh import those regions will be recognized as sub-meshes.

You can assign a different material to a sub-mesh by calling a @b3d::Renderable::SetMaterial(u32, HMaterial) overload which accepts an additional index parameter, specifying which sub-mesh to apply the material on.

~~~~~~~~~~~~~{.cpp}
HMesh mesh = GetImporter().Import<Mesh>("myMesh.fbx");
renderable->SetMesh(mesh);

// Count the number of sub-meshes
const auto& meshProperties = mesh->GetProperties();
u32 subMeshCount = (u32)meshProperties.SubMeshes.size();

// ... create necessary materials ...

// Assign a different material on every submesh
for(u32 i = 0; i < subMeshCount; i++)
    renderable->SetMaterial(i, materials[i]);
~~~~~~~~~~~~~

Alternatively, you can set all materials at once using @b3d::Renderable::SetMaterials:

~~~~~~~~~~~~~{.cpp}
// Set all materials at once
Vector<HMaterial> allMaterials = { material0, material1, material2 };
renderable->SetMaterials(allMaterials);

// Retrieve all assigned materials
const Vector<HMaterial>& materials = renderable->GetMaterials();
~~~~~~~~~~~~~

# Layer masks
Renderables can be assigned to specific layers, allowing cameras to selectively render them:

~~~~~~~~~~~~~{.cpp}
// Assign renderable to layer 1 (bit 0)
renderable->SetLayer(1 << 0);

// Assign to multiple layers (layers 0, 1, and 3)
renderable->SetLayer((1 << 0) | (1 << 1) | (1 << 3));

// Get current layer
u64 layer = renderable->GetLayer();
~~~~~~~~~~~~~

Make sure the renderable's layer matches the camera's layer mask for the object to be visible. See the [Cameras](00_cameras.md#layer-masks) manual for more information.

# Bounds and culling
Renderables have bounds that are used for frustum culling and other visibility tests:

~~~~~~~~~~~~~{.cpp}
// Get the world-space bounds of the renderable
Bounds bounds = renderable->GetBounds();

// Override bounds manually (useful for animated or deforming meshes)
AABox customBounds(Vector3(-10, -10, -10), Vector3(10, 10, 10));
renderable->SetOverrideBounds(customBounds);
renderable->SetUseOverrideBounds(true);

// Disable override to use mesh bounds again
renderable->SetUseOverrideBounds(false);
~~~~~~~~~~~~~

Override bounds are useful when:
- The mesh bounds don't accurately represent the visible geometry
- The mesh deforms significantly during animation
- You want to force an object to always/never be culled

## Cull distance
You can control how distance-based culling affects individual renderables:

~~~~~~~~~~~~~{.cpp}
// Make this renderable visible from twice the normal distance
renderable->SetCullDistanceFactor(2.0f);

// Make it cull earlier (at half the distance)
renderable->SetCullDistanceFactor(0.5f);

// Get current factor
float factor = renderable->GetCullDistanceFactor();
~~~~~~~~~~~~~

The cull distance factor is multiplied with the camera's cull distance setting to determine when this specific renderable is culled.

# Velocity writing
Renderables can write per-pixel velocity information, which is required for certain rendering effects:

~~~~~~~~~~~~~{.cpp}
// Enable velocity writing (enabled by default)
renderable->SetWriteVelocity(true);

// Disable for performance if not using motion blur or TAA
renderable->SetWriteVelocity(false);

// Check if velocity writing is enabled
bool writesVelocity = renderable->GetWriteVelocity();
~~~~~~~~~~~~~

Velocity writing is required for:
- **Temporal anti-aliasing (TAA)** - Reduces aliasing by using previous frames
- **Motion blur** - Creates blur based on object movement

If you're not using these effects, disabling velocity writing can provide a minor performance improvement.

# Animation
Renderables can be animated using skeletal animation or morph targets:

~~~~~~~~~~~~~{.cpp}
// Check if the renderable is animated
if (renderable->IsAnimated())
{
    // Get the animation component
    const HAnimation& animation = renderable->GetAnimation();
}
~~~~~~~~~~~~~

Animation is typically set up automatically when you add an **Animation** component to the same scene object. See the [Animation](../10_Animation/01_animation.md) manual for details.

# Using builtin meshes
The framework provides several builtin meshes for testing and prototyping:

~~~~~~~~~~~~~{.cpp}
// Use a builtin mesh instead of importing
HMesh sphereMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Sphere);
renderable->SetMesh(sphereMesh);

// Other available builtin meshes
HMesh boxMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Box);
HMesh coneMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Cone);
HMesh cylinderMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Cylinder);
HMesh quadMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Quad);
HMesh discMesh = GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Disc);
~~~~~~~~~~~~~

# Using builtin shaders
The framework also provides builtin shaders for common rendering scenarios:

~~~~~~~~~~~~~{.cpp}
// Standard PBR shader for opaque objects
HShader standardShader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::Standard);
HMaterial standardMat = Material::Create(standardShader);

// Transparent shader for glass, water, etc.
HShader transparentShader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::Transparent);
HMaterial transparentMat = Material::Create(transparentShader);

// Decal shader for projecting textures onto geometry
HShader decalShader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::Decal);
HMaterial decalMat = Material::Create(decalShader);
~~~~~~~~~~~~~

See the [Materials](03_simpleMaterial.md) manual for more information on working with materials and shaders.

# Transform and world matrix
Renderables use the transform of their parent scene object:

~~~~~~~~~~~~~{.cpp}
// Position the renderable
renderableSceneObject->SetPosition(Vector3(10.0f, 0.0f, 5.0f));
renderableSceneObject->SetRotation(Quaternion(Degree(45), Vector3::kUnitY));
renderableSceneObject->SetScale(Vector3(2.0f, 2.0f, 2.0f));

// Get the world transform matrix (includes scale)
Matrix4 worldMatrix = renderable->GetWorldTransformMatrix();

// Get world transform without scale (useful for normal transformations)
Matrix4 worldMatrixNoScale = renderable->GetWorldTransformMatrixWithoutScale();
~~~~~~~~~~~~~

The transform matrices are cached and updated automatically when the scene object's transform changes.

# Complete example
Here's a complete example showing how to create a simple rendered scene:

~~~~~~~~~~~~~{.cpp}
// Create the camera
HSceneObject cameraSceneObject = SceneObject::Create("Camera");
cameraSceneObject->SetPosition(Vector3(0.0f, 5.0f, 10.0f));
cameraSceneObject->LookAt(Vector3(0.0f, 0.0f, 0.0f));

HCamera camera = cameraSceneObject->AddComponent<Camera>();
camera->SetMain(true);

// Create a material
HShader shader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::Standard);
HMaterial material = Material::Create(shader);

// Optional: Set a texture on the material
HTexture texture = GetImporter().Import<Texture>("wood.png");
material->SetTexture("gAlbedoTex", texture);

// Create a renderable with a builtin sphere mesh
HSceneObject sphereSceneObject = SceneObject::Create("Sphere");
sphereSceneObject->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

HRenderable renderable = sphereSceneObject->AddComponent<Renderable>();
renderable->SetMesh(GetBuiltinResources().GetBuiltinMesh(BuiltinMesh::Sphere));
renderable->SetMaterial(material);

// Optional: Add a light to see the sphere better
HSceneObject lightSceneObject = SceneObject::Create("Light");
lightSceneObject->SetPosition(Vector3(5.0f, 5.0f, 5.0f));

HLight light = lightSceneObject->AddComponent<Light>();
light->SetLightType(LightType::Directional);
light->SetColor(Color::White);
~~~~~~~~~~~~~

This will create a simple scene with a textured sphere, a camera looking at it, and a directional light illuminating it.

# ECS fragments

The **Renderable** component stores its data internally as ECS fragments rather than as direct member variables. This allows the renderer to efficiently iterate and batch-process all renderables in the scene.

## Data fragment

The primary fragment is @b3d::ecs::Renderable, which stores all the visual data for the renderable:
 - **Mesh** - The mesh resource to render
 - **Materials** - Array of materials, one per sub-mesh
 - **Layer** - Layer bitfield for camera visibility filtering
 - **OverrideBounds** - Custom bounding box when override bounds are enabled
 - **UseOverrideBounds** - Whether to use the custom bounding box
 - **WriteVelocity** - Whether to write per-pixel velocity for TAA/motion blur
 - **CullDistanceFactor** - Multiplier for the camera's cull distance
 - **AnimType** - Type of animation applied (none, skinned, morph, or both)

When you call methods like @b3d::Renderable::SetMesh or @b3d::Renderable::SetMaterial, the component modifies this fragment directly and marks the entity as dirty for synchronization with the render thread.

## ID fragment

Each renderable also has an @b3d::ecs::RenderableId fragment that stores a persistent renderer ID. This ID is used by the @b3d::RendererObjectStorage system to map the renderable to its packed render-thread representation. The ID is allocated when the component is created and deallocated when it is destroyed.

## Dirty tags

The renderer uses two ECS tag types to track which renderables need synchronization with the render thread:
 - `ecs::RenderableDirty` - Added when any property of the renderable changes (mesh, materials, layer, etc.). Triggers a full sync to the render thread.
 - `ecs::RenderableTransformDirty` - Added when only the transform changes. Triggers a lighter transform-only sync.

When using the **Renderable** component these tags are managed automatically. When working with raw ECS fragments you must add these tags yourself after modifying fragment data, otherwise the render thread will not see your changes.

## Using raw ECS fragments

For maximum performance you can bypass the **Renderable** component entirely and create `ecs::Renderable` fragments directly on the ECS registry. This avoids the overhead of `SceneObject`, `Component`, and the `CoreObject` system, but requires you to manage the renderer ID, world transform, and dirty tags manually.

~~~~~~~~~~~~~{.cpp}
// Get the scene's ECS registry and renderer scene
const SPtr<SceneInstance>& scene = SceneManager::Instance().GetMainScene();
ecs::Registry& registry = scene->GetECSRegistry();
const SPtr<RendererScene>& rendererScene = scene->GetRendererScene();

// Create an entity and add the renderable data fragment
ecs::Entity entity = registry.CreateEntity();
ecs::Renderable& fragment = registry.AddComponent<ecs::Renderable>(entity);
fragment.Mesh = myMesh;
fragment.Materials = { myMaterial };
fragment.Layer = 1;

// Add a world transform fragment — the renderer reads this to position the object
registry.AddComponent<ecs::WorldTransform>(entity, ecs::WorldTransform(myTransform));

// Allocate a persistent renderer ID — this registers the entity with the
// RendererObjectStorage so it gets a packed slot on the render thread
rendererScene->AllocateRenderableId(registry, entity);

// Mark as dirty so the data is synced to the render thread on the next frame
registry.AddTag<ecs::RenderableDirty>(entity);
~~~~~~~~~~~~~

When modifying the fragment after creation, add the appropriate dirty tag:

~~~~~~~~~~~~~{.cpp}
// Modify fragment data
ecs::Renderable& fragment = registry.GetComponents<ecs::Renderable>(entity);
fragment.Materials = { newMaterial };

// Mark dirty so the change is synced
registry.AddTag<ecs::RenderableDirty>(entity);

// For transform-only changes
registry.GetComponents<ecs::WorldTransform>(entity) = ecs::WorldTransform(newTransform);
registry.AddTag<ecs::RenderableTransformDirty>(entity);
~~~~~~~~~~~~~

When destroying the entity, deallocate the renderer ID first:

~~~~~~~~~~~~~{.cpp}
// Deallocate the renderer ID before destroying the entity
rendererScene->DeallocateRenderableId(registry, entity);

// Clean up tags and fragments
registry.RemoveComponents<ecs::RenderableDirty>(entity);
registry.RemoveComponents<ecs::RenderableTransformDirty>(entity);
registry.RemoveComponents<ecs::Renderable>(entity);
registry.EraseEntity(entity);
~~~~~~~~~~~~~
