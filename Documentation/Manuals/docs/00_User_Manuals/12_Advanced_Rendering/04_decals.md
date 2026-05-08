---
title: Decals
---

Decals allow you to project textures onto visible geometry. During gameplay they can be used for dynamic effects such as explosion markings, bullet holes or blood spatter. During level design they can be used to add extra detail to the level and provide detailed surfaces without needing to use large textures.

![Decal](../../Images/decal.png)

> Decals will only project onto surfaces rendered using the deferred rendering pipeline. This includes the built-in standard material, but excludes any materials with transparency or any custom materials built to use the forward rendering pipeline.

Decals are represented with the @b3d::Decal component.

~~~~~~~~~~~~~{.cpp}
HSceneObject decalSceneObject = SceneObject::Create("Decal");
HDecal decal = decalSceneObject->AddComponent<Decal>();
~~~~~~~~~~~~~

# Material

You must assign a **Material** to render the decal with. This is done by calling @b3d::Decal::SetMaterial. You may create one using the built-in decal shader available from **BuiltinResources::GetBuiltinShader()** by using the @b3d::BuiltinShader::Decal enum.

~~~~~~~~~~~~~{.cpp}
// Create the material
HShader decalShader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::Decal);
HMaterial decalMaterial = Material::Create(decalShader);

// Assign material textures
HTexture decalAlbedoTexture = ...;
HTexture decalNormalTexture = ...;

decalMaterial->SetTexture("gAlbedoTex", decalAlbedoTexture);
decalMaterial->SetTexture("gNormalTex", decalNormalTexture);

// Orient the decal
HSceneObject decalSceneObject = SceneObject::Create("Decal");
decalSceneObject->SetPosition(Vector3(0.0f, 6.0f, 0.0f));
decalSceneObject->LookAt(Vector3(0.0f, 0.0f, 0.0f));

// Create the component and set the material
HDecal decal = decalSceneObject->AddComponent<Decal>();
decal->SetMaterial(decalMaterial);
~~~~~~~~~~~~~

The default material accepts the same parameters as the standard surface material. It uses the opacity from the albedo texture's alpha channel, or from a separate single-channel texture available under the **gOpacity** parameter. The opacity is used to determine how are the decal textures blended with the underlying surface.

You may also enable different variants of the material through the **BLEND_MODE** shader variation parameter. Four values are supported:
 - 0 - Transparent - This is the default mode where a full complement of PBR textures is provided and blended with the underlying surface.
 - 1 - Stain - Similar to Transparent except the albedo color is multiplied (modulated) with the underlying albedo. This makes it for suitable for stain-like decals that modify the existing color, rather than replace it.
 - 2 - Normal - Only the normal map is projected. This allows the decal to be used for effects such a footsteps in the snow.
 - 3 - Emissive - Only the emissive texture is projected. Useful for making surfaces appear as emitting light.

~~~~~~~~~~~~~{.cpp}
// Create the material
HShader decalShader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::Decal);
HMaterial decalMaterial = Material::Create(decalShader);

// Enable normal-only blend mode
decalMaterial->SetVariation(ShaderVariation(
	{
		ShaderVariation::Param("BLEND_MODE", 2)
	})
);

// Assign normal texture
HTexture decalNormalTexture = ...;
decalMaterial->SetTexture("gNormalTex", decalNormalTexture);

// Apply the material
HDecal decal = ...;
decal->SetMaterial(decalMaterial);
~~~~~~~~~~~~~

You can retrieve the currently assigned material using @b3d::Decal::GetMaterial.

~~~~~~~~~~~~~{.cpp}
HDecal decal = ...;

// Get the current material
HMaterial currentMaterial = decal->GetMaterial();
~~~~~~~~~~~~~

# Size and transform

Use the **SceneObject** transform to position and orient the decal. The decal will always project towards the negative Z axis. Set the size of the projected decal in world space by calling @b3d::Decal::SetSize.

~~~~~~~~~~~~~{.cpp}
HDecal decal = ...;

// Make the projected decal 2x2 meters
decal->SetSize(Vector2(2.0f, 2.0f));
~~~~~~~~~~~~~

You can query the current size using @b3d::Decal::GetSize.

~~~~~~~~~~~~~{.cpp}
HDecal decal = ...;

// Get the current decal size
Vector2 decalSize = decal->GetSize();
B3D_LOG(Info, LogRenderer, "Decal size: {0}", decalSize);
~~~~~~~~~~~~~

For performance reasons decal will not project infinitely. You can set the maximum projection distance by calling @b3d::Decal::SetMaxDistance. You want to keep this as low as possible in order to reduce the rendering cost of the decal. For example if a decal is projecting onto a flat surface (e.g. a floor or wall) you'll want to position it close to the surface and keep the maximum distance low.

~~~~~~~~~~~~~{.cpp}
HDecal decal = ...;

// Set the project distance to 20 centimeters
decal->SetMaxDistance(0.2f);
~~~~~~~~~~~~~

You can query the current maximum distance using @b3d::Decal::GetMaxDistance.

~~~~~~~~~~~~~{.cpp}
HDecal decal = ...;

// Get the current maximum projection distance
float maxDistance = decal->GetMaxDistance();
B3D_LOG(Info, LogRenderer, "Max distance: {0}", maxDistance);
~~~~~~~~~~~~~

# Layers and masking

Use @b3d::Decal::SetLayerMask to control onto which surfaces should a decal be rendered onto. If a mask matches the layer of the surface the decal will be visible on the surface.

~~~~~~~~~~~~~{.cpp}
HDecal decal = ...;

// Only project onto surfaces rendered with layers 0 to 15
decal->SetLayerMask(0xF);

// Set layer 1 on renderable (decal will project on it)
HRenderable renderable = ...;
renderable->SetLayer(1 << 1);

// Set layer 20 on renderable (decal will NOT project on it)
HRenderable renderable2 = ...;
renderable2->SetLayer(1 << 20);
~~~~~~~~~~~~~

You can query the current layer mask using @b3d::Decal::GetLayerMask.

~~~~~~~~~~~~~{.cpp}
HDecal decal = ...;

// Get the current layer mask
u32 layerMask = decal->GetLayerMask();
B3D_LOG(Info, LogRenderer, "Layer mask: {0}", layerMask);
~~~~~~~~~~~~~

Additionally, decals have their own layer that determines whether they are visible to a specific camera. Use @b3d::Decal::SetLayer to set the decal's layer, which must match the camera's layer bitfield for the decal to be rendered.

~~~~~~~~~~~~~{.cpp}
HDecal decal = ...;

// Set the decal to layer 1
decal->SetLayer(1 << 1);
~~~~~~~~~~~~~

You can query the current layer using @b3d::Decal::GetLayer.

~~~~~~~~~~~~~{.cpp}
HDecal decal = ...;

// Get the current layer
u64 layer = decal->GetLayer();
B3D_LOG(Info, LogRenderer, "Decal layer: {0}", layer);
~~~~~~~~~~~~~

![Decal with masking](../../Images/DecalMask.png)

# ECS fragments

The **Decal** component stores its data internally as ECS fragments, enabling the renderer to batch-process all decals in the scene efficiently.

## Data fragment

The primary fragment is @b3d::ecs::Decal, which stores the decal's visual properties:
 - **Material** - Material used to render the decal
 - **Size** - Width and height of the projected decal in world space
 - **MaxDistance** - Maximum projection distance from the decal's origin
 - **Layer** - Layer bitfield for camera visibility filtering
 - **LayerMask** - Bitfield controlling which surfaces the decal projects onto

When you call setter methods like @b3d::Decal::SetMaterial or @b3d::Decal::SetSize, the component modifies this fragment and marks the entity as dirty for synchronization with the render thread.

## ID fragment

Each decal also has an @b3d::ecs::DecalId fragment that stores a persistent renderer ID used by the @b3d::RendererObjectStorage system for mapping to the packed render-thread representation.

## Using raw ECS fragments

You can bypass the **Decal** component and create `ecs::Decal` fragments directly for maximum performance. Helper functions @b3d::ecs::CreateDecal and @b3d::ecs::DestroyDecal handle fragment creation, world transform, renderer ID allocation, and cleanup. Use @b3d::ecs::DecalECSUtility to mark dirty after property changes.

~~~~~~~~~~~~~{.cpp}
const TShared<SceneInstance>& scene = SceneManager::Instance().GetMainScene();
ecs::Registry& registry = scene->GetECSRegistry();
const TShared<RendererScene>& rendererScene = scene->GetRendererScene();

// Create an entity with all decal fragments, a world transform, and a renderer ID
ecs::Entity entity = registry.CreateEntity();
ecs::Decal& fragment = ecs::CreateDecal(registry, entity, rendererScene, myTransform);

// Configure the decal
fragment.Material = myDecalMaterial;
fragment.Size = Vector2(2.0f, 2.0f);
fragment.MaxDistance = 0.2f;
~~~~~~~~~~~~~

When modifying the fragment after creation, mark it dirty so the change is synced to the render thread:

~~~~~~~~~~~~~{.cpp}
ecs::Decal& fragment = registry.GetComponents<ecs::Decal>(entity);
fragment.Size = Vector2(4.0f, 4.0f);
ecs::DecalECSUtility::MarkDirty(registry, entity);

// For transform-only changes
registry.GetComponents<ecs::WorldTransform>(entity) = ecs::WorldTransform(newTransform);
ecs::DecalECSUtility::MarkTransformDirty(registry, entity);
~~~~~~~~~~~~~

When destroying the entity, call `ecs::DestroyDecal` which removes fragments. Cleanup of the renderer ID and dirty tags is handled by the associated RendererScene:

~~~~~~~~~~~~~{.cpp}
ecs::DestroyDecal(registry, entity);
registry.EraseEntity(entity);
~~~~~~~~~~~~~
