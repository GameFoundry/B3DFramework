Materials 						{#simpleMaterial}
===============

Materials are objects that control how are meshes rendered. They are represented using the @ref bs::Material "Material" class. Each material must have one @ref bs::Shader "Shader" object, and zero or more parameters.

A shader is a set of GPU programs and render states that tell the GPU how is a mesh meant to be rendered. Most GPU programs in the shader have parameters that can be used for cutomizing the shader output. The primary use of the material is to allow the user to set those parameters. You can think of shaders as templates, and materials as instances of shaders - similar as you would think of a *class* vs. *object* relationship in a programming language.

# Retrieving a shader
Before we can create a material we first need to pick a shader to use as a basis. Banshee allows you to create fully custom shaders, but this is an advanced topic and is left for a later chapter. For the majority of purposes when rendering 3D geometry you can use either of the following two shaders:
 - Standard - Physically based shader for opaque 3D geometry
 - Transparent - Physically based shader for transparent 3D geometry

Both of those shaders can be accessed through @ref bs::BuiltinResources::getBuiltinShader "BuiltinResources::getBuiltinShader()" using the values @ref bs::BuiltinShader::Standard "BuiltinShader::Standard" and @ref bs::BuiltinShader::Transparent "BuiltinShader::Transparent" respectively. @ref bs::BuiltinResources "BuiltinResources" can be globally accessed through @ref bs::BuiltinResources::instance() "BuiltinResources::instance()" and aside from shaders provides a variety of other resources that are always available.

~~~~~~~~~~~~~{.cpp}
// Get the standard built-in shader
HShader shader = BuiltinResources::instance().getBuiltinShader(BuiltinShader::Standard);
~~~~~~~~~~~~~

Both of these shaders provide physically based shading and expect four different parameters (see below on how to set parameters):
 - **gAlbedoTex** - RGBA texture representing the color of the object's surface. If using the transparent shader, alpha channels determines the amount of transparency.
 - **gNormalTex** - Normal map (texture containing surface normals)
 - **gRoughnessTex** - 1D texture that determines the roughness of the surface. Values closer to 1 mean a more rough (less reflective) surface, while values closer to 0 mean less rough (more reflective, mirror like) surface.
 - **gMetalnessTex** - 1D texture that determines if the part of the surface is a metal or a dieletric. This texture should only generally contain values 1 (metal) or 0 (dieletric). Metal surfaces are reflective reflective while dieletric ones are not.

At minimum you need to provide the albedo texture, while others can be left as default (or be assigned pure white, or pure black textures) if not required. 
 
# Material creation
To create a material use the @ref bs::Material::create "Material::create()" method, which expects a **Shader** as a parameter.

~~~~~~~~~~~~~{.cpp}
// Create a material based on the shader we retrieved above
HMaterial material = Material::create(shader);
~~~~~~~~~~~~~

# Setting parameters
As we mentioned, the main purpose of a material is to provide a way to set various parameters exposed by the shader. In the example below we show how to set the albedo texture parameter.

~~~~~~~~~~~~~{.cpp}
HTexture texture = gImporter().import<Texture>("myTexture.png");

// Set the texture for the "gAlbedoTex" parameter.
material->setTexture("gAlbedoTex", texture);
~~~~~~~~~~~~~

After the texture has been set, anything rendered with that material will now have that particular texture applied. Different shaders will accept different parameters of different types.

In this particular example we have a parameter named "gAlbedoTex" that accepts a **Texture** resource. We set such a parameter by calling @ref bs::Material::setTexture "Material::setTexture()". There are other parameter types like floats, ints, colors, as well as multi-dimensional types like vectors and matrices which can be set by calling @ref bs::Material::setFloat "Material::setFloat()", @ref bs::Material::setColor "Material::setColor()", @ref bs::Material::setVec4 "Material::setVec4()" and similar.

~~~~~~~~~~~~~{.cpp}
// Assuming our material has some more parameters, for purposes of the example
material->setColor("color", Color::White);
material->setFloat("time", 30.0f);
material->setVec3("position", Vector3(0, 15.0f, 10.0f));
material->setMat4("someTransform", Matrix4::IDENTITY);
~~~~~~~~~~~~~

## Sampler states
Sampler states are a special type of parameters that can be set by calling @ref bs::Material::setSamplerState "Material::setSamplerState()". These states are used to control how is a texture read in a shader - for example they control what type of filtering to use, how to handle out of range texture coordinates and similar. Sampler states are created by calling @ref bs::SamplerState::create "SamplerState::create()", while previously filling out the @ref bs::SAMPLER_STATE_DESC "SAMPLER_STATE_DESC" structure.

In most cases you don't need to set sampler states as the default one should be adequate. Shaders can also read textures without samplers by directly accessing their pixels, but this is not wanted for normal rendering as it ruins image quality due to the lack of filtering and it may not be as performance efficient. 

As an example, lets set up a sampler state that enables trilinear filtering for a texture using it, and then assign it to a material. 

~~~~~~~~~~~~~{.cpp}
SAMPLER_STATE_DESC desc;
desc.minFilter = FO_LINEAR;
desc.magFilter = FO_LINEAR;
desc.mipFilter = FO_LINEAR;

SPtr<SamplerState> samplerState = SamplerState::create(desc);

// "gAlbedoSamp" is a sampler state parameter provided by the standard shader we
// used in the example above. It controls options for the texture set on the gAlbedoTex
// parameter.
material->setSamplerState("gAlbedoSamp", samplerState);
~~~~~~~~~~~~~

# Using a material for rendering
Once a material is created and parameters are set, it can be used for rendering by attaching it to a **Renderable** component. We will cover them in the next chapter.