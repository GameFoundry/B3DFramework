---
title: Skybox
---
Skyboxes use a user-provided cubemap texture in order to display an image of the sky when the camera is looking at the scene when no other object is occluding the sky. The same image is also used to provide both specular reflections and indirect lighting on objects lit by the sky, but we will cover these effects later.

[TODO_IMAGE]()

Skybox is represented by the @b3d::Skybox component, which requires only a texture of the sky to work. The texture should ideally be in high dynamic range, unless your application is not using HDR. The skybox texture can be set through @b3d::Skybox::SetTexture.

~~~~~~~~~~~~~{.cpp}
// Import a sky cubemap from a cylindrical (panoramic) image
TShared<TextureImportOptions> textureImportOptions = TextureImportOptions::Create();
textureImportOptions->Cubemap = true;
textureImportOptions->CubemapSourceType = CubemapSourceType::Cylindrical;
textureImportOptions->Format = PF_RG11B10F; // Or the 16-bit floating point format

HTexture skyTexture = GetImporter().Import<Texture>("MySkybox.hdr", textureImportOptions);

// Set up the skybox
HSceneObject skyboxSceneObject = SceneObject::Create("Skybox");
HSkybox skybox = skyboxSceneObject->AddComponent<Skybox>();

skybox->SetTexture(skyTexture);
~~~~~~~~~~~~~

Note that importing a cubemap texture requires special texture import options @b3d::TextureImportOptions::Cubemap and @b3d::TextureImportOptions::CubemapSourceType. The second property expects you to provide the a @b3d::CubemapSourceType that defines in what format is the texture stored in. The formats are:
 - **CubemapSourceType::Cylindrical** - The source is a typical panoramic image. This is the most common format.
 - **CubemapSourceType::Spherical** - The source is an image captured off a surface of a sphere. This is an older format that is less commonly used today.
 - **CubemapSourceType::Single** - The source is a normal 2D texture. All cubemap faces will use the same image.
 - **CubemapSourceType::Faces** - The source image contains cubemap faces laid out in the "cross" pattern, either vertically or horizontally.

Aside from setting a texture you might also want to increase or decrease the brightness of the sky by calling @b3d::Skybox::SetBrightness. Note that this will not effect the visual appearance of the sky but will only affect the lighting cast by the sky on other surfaces.

~~~~~~~~~~~~~{.cpp}
HSceneObject skyboxSceneObject = SceneObject::Create("Skybox");
HSkybox skybox = skyboxSceneObject->AddComponent<Skybox>();

// Set the skybox texture
HTexture skyTexture = GetImporter().Import<Texture>("MySkybox.hdr", textureImportOptions);
skybox->SetTexture(skyTexture);

// Adjust the brightness multiplier for lighting calculations
skybox->SetBrightness(1.5f);
~~~~~~~~~~~~~

You can also query the current brightness setting using @b3d::Skybox::GetBrightness and retrieve the current texture using @b3d::Skybox::GetTexture.

~~~~~~~~~~~~~{.cpp}
HSkybox skybox = ...;

// Get the current brightness value
float brightness = skybox->GetBrightness();
B3D_LOG(Info, LogGeneric, "Skybox brightness: {0}", brightness);

// Get the current skybox texture
HTexture currentTexture = skybox->GetTexture();
~~~~~~~~~~~~~
