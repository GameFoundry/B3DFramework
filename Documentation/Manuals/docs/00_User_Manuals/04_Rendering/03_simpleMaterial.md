---
title: Materials
---

Materials are resources that control how meshes are rendered. They are represented using the @b3d::Material class. Each material must have one @b3d::Shader object, and zero or more parameters.

A shader is a set of GPU programs and render states that tell the GPU how a mesh is meant to be rendered. Generally these GPU programs have parameters that can control what they output (for example, which texture to use). A material allows you to set those parameters. You can think of shaders as templates, and materials as instances of shaders - similar as you would think of a *class* vs. *object* relationship in a programming language.

# Retrieving a shader
Before we can create a material we first need to pick a shader to use as a basis. The framework allows you to create fully custom shaders, but this is an advanced topic and is left for a later chapter. For the majority of purposes when rendering 3D geometry you can use either of the following two shaders:
 - Standard - Physically based shader for opaque 3D geometry
 - Transparent - Physically based shader for transparent 3D geometry

Both of those shaders can be accessed through @b3d::BuiltinResources::GetBuiltinShader using the values @b3d::BuiltinShader::Standard and @b3d::BuiltinShader::Transparent respectively.

~~~~~~~~~~~~~{.cpp}
// Get the standard PBR shader
HShader shader = GetBuiltinResources().GetBuiltinShader(BuiltinShader::Standard);
~~~~~~~~~~~~~

Both of these shaders provide physically based shading and expect the following parameters (see below on how to set parameters):
 - **gAlbedoTex** - RGBA texture representing the color of the object's surface. If using the transparent shader, alpha channel determines the amount of transparency.
 - **gNormalTex** - Normal map (texture containing surface normals encoded into RGB channels)
 - **gRoughnessTex** - Single-channel texture that determines the roughness of the surface. Values closer to 1 mean a more rough (less reflective) surface, while values closer to 0 mean less rough (more reflective, mirror like) surface.
 - **gMetalnessTex** - Single-channel texture that determines if the part of the surface is a metal or a dieletric. This texture should only generally contain values 1 (metal) or 0 (dieletric). Metal surfaces are reflective while dieletric ones are not.
 - **gEmissiveMaskTex** - Single-channel texture that determines which parts of the surface emit light. Black values specify no light is emitted, while white values specify light at full brightness is emitted.
 - **gEmissiveColor** - Color and intensity of the emitted light, for areas marked by **gEmissiveMaskTex**.
 - **gUVOffset** - 2D vector value that allows you to change the starting offset at which textures are sampled. By default (0, 0).
 - **gUVTile** - 2D vector that allows you to specify how many times should the texture repeat, used for tiling textures. By default (1, 1).

At minimum you need to provide the albedo texture, while others can be left as default (or be assigned pure white, or pure black textures) if not required.

# Material creation
To create a material use the @b3d::Material::Create method, which expects a **Shader** as a parameter.

~~~~~~~~~~~~~{.cpp}
// Create a material based on the shader we retrieved above
HMaterial material = Material::Create(shader);
~~~~~~~~~~~~~

# Setting parameters
As we mentioned, the main purpose of a material is to provide a way to set various parameters exposed by the shader. In the example below we show how to set the albedo texture parameter.

~~~~~~~~~~~~~{.cpp}
HTexture texture = GetImporter().Import<Texture>("myTexture.png");

// Set the texture for the "gAlbedoTex" parameter.
material->SetTexture("gAlbedoTex", texture);
~~~~~~~~~~~~~

After the texture has been set, anything rendered with that material will now have that particular texture applied. Different shaders will accept different parameters of different types.

In this particular example we have a parameter named "gAlbedoTex" that accepts a **Texture** resource. We set such a parameter by calling @b3d::Material::SetTexture. There are other parameter types like floats, ints, colors, as well as multi-dimensional types like vectors and matrices which can be set by calling @b3d::Material::SetFloat, @b3d::Material::SetColor, @b3d::Material::SetVec4 and similar.

~~~~~~~~~~~~~{.cpp}
// Assuming our material has some more parameters, for purposes of the example
material->SetColor("color", Color::White);
material->SetFloat("time", 30.0f);
material->SetVec3("position", Vector3(0, 15.0f, 10.0f));
material->SetMat4("someTransform", Matrix4::kIdentity);
~~~~~~~~~~~~~

## Array parameters
If a shader parameter is an array, you can set individual array elements by providing an array index:

~~~~~~~~~~~~~{.cpp}
// Set individual elements of an array parameter
material->SetFloat("values", 1.0f, 0); // First element
material->SetFloat("values", 2.0f, 1); // Second element
material->SetFloat("values", 3.0f, 2); // Third element
~~~~~~~~~~~~~

## Retrieving parameters
You can also retrieve parameter values that have been set:

~~~~~~~~~~~~~{.cpp}
// Get parameter values
float floatValue = material->GetFloat("time");
Color colorValue = material->GetColor("color");
Vector3 vec3Value = material->GetVec3("position");
Matrix4 mat4Value = material->GetMat4("someTransform");

// Get array element
float arrayElement = material->GetFloat("values", 1);
~~~~~~~~~~~~~

## Animated parameters
Certain material parameters can be animated, meaning they will change as time passes. The types of animable parameters are:
 - **float** - Instead of calling **Material::SetFloat()** call @b3d::Material::SetFloatCurve and pass a @b3d::TAnimationCurve<float> object as the parameter. Animation curve consists of a set of key-frames that get interpolated between depending on the time the curve gets sampled at.
 - **Color** - Instead of calling **Material::SetColor()** call @b3d::Material::SetColorGradient and pass a @b3d::ColorGradientHDR object as the parameter. Similarly to animation curves the **ColorGradientHDR** contains a set of key-frames, each containing a color, which then get interpolated between depending on the time that's used to evaluate them.
 - **Texture** - Instead of calling **Material::SetTexture()** call @b3d::Material::SetSpriteImage, which accepts a **SpriteImage** object. Sprite images allow you to provide texture animation and as time passes different frames of texture animation will be presented to the user. Sprite images are explained in more detail later on.

An example using all three types of animated parameters:

~~~~~~~~~~~~~{.cpp}
// Create an animation curve with three keys:
// [0] - Value 1 at time 0.0s
// [1] - Value 2 at time 0.5s
// [2] - Value 1 at time 1.0s
// The curve starts at value of 1, goes to 2 and then back to 1, in the duration of one second.
// (Middle two values of each keyframe represent tangents that allow finer control
// of the curve, but you can leave them at zero)
Vector<TKeyframe<float>> keyframes =
{
    TKeyframe<float>(0.0f, 1.0f, 0.0f, 0.0f), // time, value, inTangent, outTangent
    TKeyframe<float>(0.5f, 2.0f, 0.0f, 0.0f),
    TKeyframe<float>(1.0f, 1.0f, 0.0f, 0.0f)
};

TAnimationCurve<float> curve(keyframes);
material->SetFloatCurve("gScale", curve);

// Create a color gradient with three keys
// [0] - Red color at time 0.0s
// [1] - Blue color at time 2.5s
// [2] - Red color at time 5.0s
// The gradient starts with red color, interpolates towards blue and then back to red,
// in the duration of five seconds.
ColorGradientHDR gradient;
gradient.SetKeys({
    ColorGradientHDRKey(Color::Red, 0.0f),
    ColorGradientHDRKey(Color::Blue, 2.5f),
    ColorGradientHDRKey(Color::Red, 5.0f)
});

material->SetColorGradient("gTint", gradient);

// Create a sprite image with sprite sheet animation (explained below)
HTexture texture = GetImporter().Import<Texture>("spriteSheet.png");
HSpriteImage spriteImage = SpriteImage::Create(texture);

SpriteSheetGridAnimation animation;
animation.RowCount = 3;
animation.ColumnCount = 3;
animation.FrameCount = 8;
animation.FramesPerSecond = 8;

spriteImage->SetAnimation(animation);
spriteImage->SetAnimationPlayback(SpriteAnimationPlayback::Loop);

material->SetSpriteImage("gAlbedoTex", spriteImage);
~~~~~~~~~~~~~

You can retrieve animated parameters as well:

~~~~~~~~~~~~~{.cpp}
// Get curve (returns empty curve if not set)
const TAnimationCurve<float>& curve = material->GetFloatCurve("gScale");

// Get gradient (returns empty gradient if not set)
const ColorGradientHDR& gradient = material->GetColorGradient("gTint");
~~~~~~~~~~~~~

## Sampler states
Sampler states are a special type of parameters that can be set by calling @b3d::Material::SetSamplerState. These states are used to control how a texture is read in a shader. For example they control what type of filtering to use, how to handle out of range texture coordinates and similar. In most cases you don't need to set sampler states as the default one should be adequate.

Sampler states are created by calling @b3d::SamplerState::Create, while previously filling out the @b3d::SamplerStateCreateInformation structure.

As an example, lets set up a sampler state that enables trilinear filtering for a texture using it, and then assign it to a material.

~~~~~~~~~~~~~{.cpp}
SamplerStateCreateInformation description;
description.MinFilter = FO_LINEAR;
description.MagFilter = FO_LINEAR;
description.MipFilter = FO_LINEAR;

TShared<SamplerState> samplerState = SamplerState::Create(description);

// "gAlbedoSamp" is a sampler state parameter provided by the standard shader we
// used in the example above. It controls options for the texture set on the gAlbedoTex
// parameter.
material->SetSamplerState("gAlbedoSamp", samplerState);
~~~~~~~~~~~~~

### Common sampler state options
Here are some common sampler state configurations:

~~~~~~~~~~~~~{.cpp}
// Point filtering (nearest neighbor, blocky look)
SamplerStateCreateInformation pointFilter;
pointFilter.MinFilter = FO_POINT;
pointFilter.MagFilter = FO_POINT;
pointFilter.MipFilter = FO_POINT;

// Bilinear filtering (smooth but can be blurry at angles)
SamplerStateCreateInformation bilinearFilter;
bilinearFilter.MinFilter = FO_LINEAR;
bilinearFilter.MagFilter = FO_LINEAR;
bilinearFilter.MipFilter = FO_POINT;

// Trilinear filtering (smoothest, best for most cases)
SamplerStateCreateInformation trilinearFilter;
trilinearFilter.MinFilter = FO_LINEAR;
trilinearFilter.MagFilter = FO_LINEAR;
trilinearFilter.MipFilter = FO_LINEAR;

// Anisotropic filtering (best quality for textures at oblique angles)
SamplerStateCreateInformation anisotropicFilter;
anisotropicFilter.MinFilter = FO_ANISOTROPIC;
anisotropicFilter.MagFilter = FO_ANISOTROPIC;
anisotropicFilter.MipFilter = FO_ANISOTROPIC;
anisotropicFilter.MaxAnisotropy = 16;

// Texture wrapping modes
SamplerStateCreateInformation wrapMode;
wrapMode.AddressMode.U = TAM_WRAP;   // Repeat texture
wrapMode.AddressMode.V = TAM_CLAMP;  // Clamp to edge
wrapMode.AddressMode.W = TAM_MIRROR; // Mirror texture
~~~~~~~~~~~~~