Skybox						{#skybox}
===============

Skyboxes use a user-provided cubemap texture in order to display an image of the sky when the camera is looking at the scene when no other object is occluding the sky. The same image is also used to provide both specular reflections and indirect lighting on objects lit by the sky, but we will cover these effects later.

@ref TODO_IMAGE

Skybox is represented by the @ref bs::CSkybox "Skybox" component, which requires only a texture of the sky to work. The texture should ideally be in high dynamic range, unless your application is not using HDR. The skybox texture can be set through @ref bs::CSkybox::setTexture "CSkybox::setTexture()".

~~~~~~~~~~~~~{.cpp}
// Import a sky cubemap from a cylindrical (panoramic) image
SPtr<ImportOptions> tio = TextureImportOptions::create();
tio->cubemap = true;
tio->cubemapSourceType = CubemapSourceType::Cylindrical;
tio->format = PF_FLOAT_R11G11B10; // Or the 16-bit floating point format

HTexture skyTexture = gImporter().import<Texture>("MySkybox.hdr", tio);

// Set up the skybox
HSceneObject skyboxSO = SceneObject::create("Skybox");
HSkybox skybox = skyboxSO->addComponent<CSkybox>();

skybox->setTexture(skyTexture);
~~~~~~~~~~~~~

Note that importing a cubemap texture requires special texture import options @ref bs::TextureImportOptions::cubemap "TextureImportOptions::cubemap" and @ref bs::TextureImportOptions::cubemapSourceType "TextureImportOptions::cubemapSourceType". The second property expects you to provide the a @ref bs::CubemapSourceType "CubemapSourceType" that defines in what format is the texture stored in. The formats are:
 - **CubemapSourceType::Cylindrical** - The source is a typical panoramic image. This is the most common format.
 - **CubemapSourceType::Spherical** - The source is an image captured off a surface of a sphere. This is an older format that is less commonly used today.
 - **CubemapSourceType::Single** - The source is a normal 2D texture. All cubemap faces will use the same image.
 - **CubemapSourceType::Faces** - The source image contains cubemap faces laid out in the "cross" pattern, either vertically or horizontally.

Aside from setting a texture you might also want to increase or decrease the brightness of the sky by calling @ref bs::CSkybox::setBrightness "CSkybox::setBrightness()". Note that this will not effect the visual appearance of the sky but will only affect the lighting cast by the sky on other surfaces.
