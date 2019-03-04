Decals						{#decals}
===============
[TOC]

Decals allow you to project textures onto visible geometry. During gameplay they can be used for dynamic effects such as explosion markings, bullet holes or blood spatter. During level design they can be used to add extra detail to the level and provide detailed surfaces without needing to use large textures.

![Decal](decal.png)  

> Decals will only project onto surfaces rendered using the deferred rendering pipeline. This includes the built-in standard material, but excludes any materials with transparency or any custom materials built to use the forward rendering pipeline.

Decals are represented with the @ref bs::CDecal "Decal" component.

~~~~~~~~~~~~~{.cpp}
HSceneObject decalSO = SceneObject::create("Decal");
HDecal decal = decalSO->addComponent<CDecal>();
~~~~~~~~~~~~~

# Material {#decals_a}

You must assign a **Material** to render the decal with. This is done by calling @ref bs::CDecal::setMaterial "CDecal::setMaterial()". You may create one using the built-in decal shader available from **BuiltinResources::getBuiltinShader()** by using the @ref bs::BuiltinShader::Decal "BuiltinShader::Decal" enum.

~~~~~~~~~~~~~{.cpp}
// Create the material
HShader decalShader = gBuiltinResources().getBuiltinShader(BuiltinShader::Decal);
HMaterial decalMaterial = Material::create(decalShader);

// Assign material textures
HTexture decalAlbedoTex = ...;
HTexture decalNormalTex = ...;

decalMaterial->setTexture("gAlbedoTex", decalAlbedoTex);
decalMaterial->setTexture("gNormalTex", decalNormalTex);

// Orient the decal
HSceneObject decalSO = SceneObject::create("Decal");
decalSO->setPosition(Vector3(0.0f, 6.0f, 0.0f));
decalSO->lookAt(Vector3(0.0f, 0.0f, 0.0f));

// Create the component and set the material
HDecal decal = decalSO->addComponent<CDecal>();
decal->setMaterial(decalMaterial);
~~~~~~~~~~~~~

The default material accepts the same parameters as the standard surface material. It uses the opacity from the albedo texture's alpha channel, or from a separate single-channel texture available under the **gOpacity** parameter. The opacity is used to determine how are the decal textures blended with the underlying surface.

You may also enable different variants of the material through the **BLEND_MODE** shader variation parameter. Four values are supported:
 - 0 - Transparent - This is the default mode where a full complement of PBR textures is provided and blended with the underlying surface.
 - 1 - Stain - Similar to Transparent except the albedo color is multiplied (modulated) with the underlying albedo. This makes it for suitable for stain-like decals that modify the existing color, rather than replace it.
 - 2 - Normal - Only the normal map is projected. This allows the decal to be used for effects such a footsteps in the snow.
 - 3 - Emissive - Only the emissive texture is projected. Useful for making surfaces appear as emitting light.
 
~~~~~~~~~~~~~{.cpp}
// Create the material
HShader decalShader = gBuiltinResources().getBuiltinShader(BuiltinShader::Decal);
HMaterial decalMaterial = Material::create(decalShader);

// Enable normal-only blend mode
decalMaterial->setVariation(ShaderVariation(
	{
		ShaderVariation::Param("BLEND_MODE", 2)
	})
);

// Assign normal texture
HTexture decalNormalTex = ...;
decalMaterial->setTexture("gNormalTex", decalNormalTex);

// Apply the material
decal->setMaterial(decalMaterial);
~~~~~~~~~~~~~

# Size and transform {#decals_b}

Use the **SceneObject** transform to position and orient the decal. The decal will always project towards the negative Z axis. Set the size of the projected decal in world space by calling @ref bs::CDecal::setSize "CDecal::setSize()".

~~~~~~~~~~~~~{.cpp}
// Make the projected decal 2x2 meters
decal->setSize(Vector2(2.0f, 2.0f));
~~~~~~~~~~~~~

For performance reasons decal will not project infinitely. You can set the maximum projection distance by calling @ref bs::CDecal::setMaxDistance "CDecal::setMaxDistance()". You want to keep this as low as possible in order to reduce the rendering cost of the decal. For example if a decal is projecting onto a flat surface (e.g. a floor or wall) you'll want to position it close to the surface and keep the maximum distance low.

~~~~~~~~~~~~~{.cpp}
// Set the project distance to 20 centimeters
decal->setMaxDistance(0.2f);
~~~~~~~~~~~~~

# Layers and masking {#decals_c}

Use @ref bs::CDecal::setLayerMask "CDecal::setLayerMask()" to control onto which surfaces should a decal be rendered onto. If a mask matches the layer of the surface the decal will be visible on the surface.

~~~~~~~~~~~~~{.cpp}
// Only project onto surfaces rendered with layers 0 to 15
decal->setLayerMask(0xF);

// Set layer 1 on renderable (decal will project on it)
HRenderable renderable = ...;
renderable->setLayer(1 << 1)

// Set layer 20 on renderable (decal will NOT project on it)
HRenderable renderable2 = ...;
renderable2->setLayer(1 << 20)
~~~~~~~~~~~~~

![Decal with masking](DecalMask.png)  