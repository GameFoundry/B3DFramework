Surface & lighting shaders					{#surfaceShaders}
===============
[TOC]

So far we have shown how to create BSL shaders from scratch. This is useful when using the low-level rendering API and you have full control over rendering. But if you wish to create shaders to customize the behaviour of the default renderer (i.e. for use in **Renderable**) you must make sure those shaders comply to a set of rules that are expected by the renderer. Luckily bsf doesn't expect you to handle all those details by yourself, and instead allows you to easily override relevant parts of those shaders, keeping all the complex details hidden from the normal user.

Shader aspects that can be overriden can be split into different categories:
 - Vertex shaders - They control how are vertices of the rendered mesh transformed before being rasterized.
 - Surface shaders - They allow you to control what kind of information gets sent to the lighting evaluation methods, including the albedo color, normal, roughness, metalness, and others. You get to control various surface details but the lighting that evaluates them remains fixed.
 - Lighting shaders - They allow you to control how is surface BRDF evaluated, as well as completely override how is lighting evaluated. This allows you to fully control the rendering, creating stylized or otherwise different lighting models.

bsf internally uses two separate render pipelines: forward and deferred. The way shaders are handled for both of those pipelines are different, and therefore we will split this manual into two sections, one for each pipeline. When creating your shader you get to pick which pipeline you want to use for your material (deferred being the default).

Vertex shaders are handled the same regardless of the pipeline, so we will cover them first before continuing with pipeline specific shaders.

# Vertex {#surfaceShaders_a}
Vertex shaders allow you to transform object geometry before an object is rasterized. Its entry point must have the following signature:
 - `VStoFS vsmain(VertexInput input)`
 
Where:
 - *input* - Contains position of the vertex, UV coordinates, a normal and a tangent. If the rendered mesh is animated via skinning it will also include skinning weights and indices. If the renderer mesh is animated via morph shapes it will include delta position and normal.
 - Method returns a **VStoFS** structure that you will need to populate. The contents of the structure are displayed below.

~~~~~~~~~~~~~
struct VStoFS
{
	// Position in clip space (multiplied by world-view-projection matrix)
	float4 position : SV_Position;
	
	// Texture coordinates
	float2 uv0 : TEXCOORD0;
	
	// Position in world space (multiplied by the world matrix)
	float3 worldPosition : TEXCOORD1;
	
	// World space normal
	float3 tangentToWorldZ : NORMAL;
	
	// World space tangent, where w component represents tangent sign
	float4 tangentToWorldX : TANGENT;
};
~~~~~~~~~~~~~

It must include the following mixins:
 - **VertexInput** (from `VertexInput.bslinc`) - Contains a set of helper methods useful for transforming vertex inputs into outputs.
 - **PerCameraData** (from `PerCameraData.bslinc`) - Provides a variety of camera parameters, including the view-projection matrix, view direction and origin. Check the contents of `PerCameraData.bslinc` to see all the provided properties.
 - **PerObjectData** (from `PerObjectData.bslinc`) - Provides a variety of per-object paramters, including the world transform, world-view-projection matrix and object layer. Check the contents of `PerObjectData.bslinc` to see all the provided properties.

~~~~~~~~~~~~~
#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\PerObjectData.bslinc"
#include "$ENGINE$\VertexInput.bslinc"

shader VertexTransform
{
	mixin PerCameraData;
	mixin PerObjectData;
	mixin VertexInput;

	code
	{
		VStoFS vsmain(VertexInput input)
		{
			// ..
		}
	};
};
~~~~~~~~~~~~~

When populating **VStoFS** output structure you can use the following helper methods to make the job easier:
 - `VertexIntermediate getVertexIntermediate(VertexInput input)` - Calculates world space normal/tangent, as well as potentially other properties required for animation. Normally you do not need to read the returned value directly, it is instead provided to the methods below.
 - `float4 getVertexWorldPosition(VertexInput input, VertexIntermediate intermediate)` - Calculates the world space position of the vertex. It is preferable to call this method instead of calculating the world position yourself because the method will automatically account for any potential animation (skinned or morph).
 - `void populateVertexOutput(VertexInput input, VertexIntermediate intermediate, inout VStoFS result)` - Populates the UV and normal/tangent fields of the **VStoFS** structure.

You are of course not forced to use any of the methods above, as long as you properly populate the **VStoFS** structure. An example shader making use of these methods would look like so:

~~~~~~~~~~~~~
#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\PerObjectData.bslinc"
#include "$ENGINE$\VertexInput.bslinc"

shader VertexTransform
{
	mixin PerCameraData;
	mixin PerObjectData;
	mixin VertexInput;

	code
	{
		VStoFS vsmain(VertexInput input)
		{
			VStoFS output;
		
			VertexIntermediate intermediate = getVertexIntermediate(input);
			float4 worldPosition = getVertexWorldPosition(input, intermediate);
			
			output.worldPosition = worldPosition.xyz;
			output.position = mul(gMatViewProj, worldPosition);
			populateVertexOutput(input, intermediate, output);
						
			return output;
		}
	};
};
~~~~~~~~~~~~~

Once the shader is created, it can be used for creating a **Material**, which can then be assigned to a **Renderable** as normal.

# Deferred pipeline {#surfaceShaders_b}
This is the pipeline used by the *Standard* material, and is generally the preferable pipeline to use as it is usually faster, and supports a wider variety of features than the forward pipeline. 

Deferred pipeline splits the rendering process and the lighting process. When an object is rendered, all the relevant data is output to  a set of intermediate textures called the GBuffer. These textures contain all the information required for lighting, like the albedo color or the normal vectors. Lighting is then performed after all the objects have been rendered to the GBuffer. 

Due to the way its rendering works, this pipeline is incompatible with transparent materials and can only be used for rendering opaque objects.

## Surface {#surfaceShaders_b_a}
This shader is responsible for writing the surface data into the GBuffer. Its entry point must have the following signature:
 - `void fsmain(in VStoFS input, out float3 OutSceneColor : SV_Target0, out float4 OutGBufferA : SV_Target1, out float4 OutGBufferB : SV_Target2, out float2 OutGBufferC : SV_Target3, out uint OutGBufferD : SV_Target4)`
 
Where:
 - *input* - **VSToFS** structure that gets output from the vertex shader.
 - *OutSceneColor* - Use to write scene color directly. This will be blended as-is with results from the lighting calculations. Used for emissive materials.
 - *OutGBufferA* - RGB channels containing albedo color, while A contains opacity.
 - *OutGBufferB* - RGB channels containing encoded world-space normals, while A channel contains a non-zero value if anything was rendered to that location.
 - *OutGBufferC* - R channel contains roughness, G channels contains metalness. Used by physically based lighting.
 - *OutGBufferD* - R channel contains the layer of the rendered object. Layer can be used for masking on which surfaces things like decals render on.
 
The shader needs to include the following mixins:
 - **BasePass** (from `BasePass.bslinc`) - Performs the default per-vertex transformations and outputs data in the **VStoFS** structure. Optionally you can provide your own vertex evaluation code, as described earlier.
 - **GBufferOutput** (from `GBufferOutput.bslinc`) - Provides a helper structure and method for populating the GBuffer. 
 
~~~~~~~~~~~~~
#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\GBufferOutput.bslinc"

shader Surface
{
	mixin BasePass;
	mixin GBufferOutput;

	code
	{
		void fsmain(
			in VStoFS input, 
			out float4 OutSceneColor : SV_Target0,
			out float4 OutGBufferA : SV_Target1,
			out float4 OutGBufferB : SV_Target2,
			out float2 OutGBufferC : SV_Target3,
			out uint OutGBufferD : SV_Target4)
		{
			// ...
		}	
	};
};
~~~~~~~~~~~~~
 
In the shader you'll want to populate the **SurfaceData** structure, which has the following structure:
~~~~~~~~~~~~~
// Note that the 'depth' field doesn't need to be populated explicitly.
struct SurfaceData
{
	float4 albedo;
	float4 worldNormal;
	float depth;
	float roughness;
	float metalness;
	uint mask;
};
~~~~~~~~~~~~~

Once populated, you call **encodeGBuffer** method with the **SurfaceData** object, and the GBuffer textures to write the output to. The method then transfers the data written to **SurfaceData** into the GBuffer textures.
 
A complete example looks like below. For simplicity the shader only evaluates albedo from the texture, while it assumes the rest of the surface data parameters are constant.

~~~~~~~~~~~~~
#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\GBufferOutput.bslinc"

shader Surface
{
	mixin BasePass;
	mixin GBufferOutput;

	code
	{
		SamplerState gAlbedoSamp;
		Texture2D gAlbedoTex;
	
		void fsmain(
			in VStoFS input, 
			out float4 OutSceneColor : SV_Target0,
			out float4 OutGBufferA : SV_Target1,
			out float4 OutGBufferB : SV_Target2,
			out float2 OutGBufferC : SV_Target3,
			out uint OutGBufferD : SV_Target4)
		{
			SurfaceData surfaceData;
			surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, input.uv0);
			surfaceData.worldNormal.xyz = float3(0, 1, 0);
			surfaceData.roughness = 1.0f;
			surfaceData.metalness = 0.0f;
			surfaceData.mask = gLayer;
			
			encodeGBuffer(surfaceData, OutGBufferA, OutGBufferB, OutGBufferC, OutGBufferD);
			
			OutSceneColor = 0.0f;
		}	
	};
};
~~~~~~~~~~~~~

Once the shader is created, it can be used for creating a **Material**, which can then be assigned to a **Renderable** as normal.

## Lighting {#surfaceShaders_b_b}
This type of shader allows you to override how the renderer evaluates lighting. It involves creating a set of mixins that override the existing lighting functionality. For this purpose we use the concept of subshaders, which we explained earlier in the BSL manual.

In order to override lighting you must implement the **DeferredDirectLighting** sub-shader, and then override one or multiple of the following mixins:
 - **StandardBRDF** (from `DirectLighting.bslinc`) - This mixin allows you to change how is incoming light reflected off a point using the bidirectional reflectance distribution function (BRDF). In most cases when modifying the lighting model, this is the only mixin you will want to override.
 - **LuminanceRadial**, **LuminanceSpot**, **LuminanceDirectional** (from `DirectLighting.bslinc`) - This set of mixins allows you to evaluate how is lighting evaluated for radial, spot and directional lights, respectively. This is even more general than overriding the **StandardBRDF** mixin, as it allows you to control how is the lighting information evaluated before being passed along to the BRDF.
 
### Overriding the BRDF {#surfaceShaders_b_b_a}
**StandardBRDF** mixin contains a single method you must implement:
 - `float3 evaluateStandardBRDF(float3 V, float3 L, float specLobeEnergy, SurfaceData surfaceData)`

Where:
 - *V* - Normalized vector from the point being shaded, to the view origin.
 - *L* - Normalized vector from the point being shaded, to the light.
 - *specLobeEnergy* - Scaling factor to apply to the specular component of the BRDF.
 - *surfaceData* - Information about the surface point being shaded.
 - The method returns the color of the surface. This color will be multiplied with light color and intensity to generate the final color.
 
~~~~~~~~~~~~~
subshader DeferredDirectLighting
{
	// An example shader that implements the basic Lambert BRDF
	mixin StandardBRDF
	{
		code
		{	
			float3 evaluateStandardBRDF(float3 V, float3 L, float specLobeEnergy, SurfaceData surfaceData)
			{
				return surfaceData.albedo.rgb / 3.14f;
			}
		};
	};
};
~~~~~~~~~~~~~
 
### Overriding luminance calculations {#surfaceShaders_b_b_b}
**LuminanceRadial** mixin requires you to implement a single method with a following signature:
 - `float3 getLuminanceRadial(LightData lightData, float3 worldPos, float3 V, float3 R, float roughness2, SurfaceData surfaceData)`
 
Where:
 - *lightData* - A structure containing information about the light we're calculating luminance for. See below for its structure.
 - *worldPos* - World position of the point being shaded.
 - *V* - Normalized vector from the point being shaded, to the view origin.
 - *R* - Normalized vector from the point being shaded, to the point at which specular reflections should be evaluated.
 - *roughness2* - surfaceData.roughness * surfaceData.roughness
 - *surfaceData* - Information about the surface point being shaded.
 - This method returns the final color of the surface after direct lighting is applied.
 
~~~~~~~~~~~~~
struct LightData
{
	// Position of the light in world space
	float3 position;
	
	// Radius at which the light should attenuate
	float attRadius;
	
	// Normalized direction of the light
	float3 direction;
	
	// Luminance (intensity) of the light
	float luminance;
	
	// x - Spot-light angle in radians
	// y - Cosine of the spot-light angle
	// z - 1 / (cos(spotFalloffAngle) - cos(spotAngle))
	float3 spotAngles;
	
	// 1 / (attRadius*attRadius)
	float attRadiusSqrdInv;
	
	// Color of the light
	float3 color;
	
	// Source radius if it's an area light
	float srcRadius;
	
	// Shifted position used for evaluating spot area lights
	float3 shiftedLightPosition;
	
	// Unused
	float padding;
};
~~~~~~~~~~~~~

An example implementation:
~~~~~~~~~~~~~
subshader DeferredDirectLighting
{
	mixin LuminanceRadial
	{
		code
		{
			float3 getLuminanceRadial(LightData lightData, float3 worldPos, float3 V, float3 R, float roughness2, SurfaceData surfaceData)
			{
				float3 N = surfaceData.worldNormal.xyz;
				float3 toLight = lightData.position - worldPos;
				float distToLightSqrd = dot(toLight, toLight);
				float invDistToLight = rsqrt(distToLightSqrd);
				
				float3 L = toLight * invDistToLight;
				float NoL = saturate(dot(N, L));
				
				float illuminance = (lightData.luminance * NoL) / max(distToLightSqrd, 0.01f*0.01f);
				float attenuation = distToLightSqrd * lightData.attRadiusSqrdInv;
				float3 surfaceShading = evaluateStandardBRDF(V, L, 1.0f, surfaceData);
					
				return lightData.color * illuminance * attenuation * surfaceShading;
			}
		};
	};
};
~~~~~~~~~~~~~

**LuminanceSpot** similarily requires you to implement a single method with the following signature:
 - `float3 getLuminanceSpot(LightData lightData, float3 worldPos, float3 V, float3 R, float roughness2, SurfaceData surfaceData)`
 
**LuminanceDirectional** also requires a single method with the following signature:
 - `float3 getLuminanceDirectional(LightData lightData, float3 worldPos, float3 V, float3 R, SurfaceData surfaceData)`

The meaning of the parametes is the same across all three functions, and they can be implemented similarily to **getLuminanceRadial**. 

Once the shader is created you need to apply it for rendering. Because the deferred pipeline performs lighting in a separate stage from normal rendering, you cannot simply assign this material to a **Renderable**. The shader must instead be applied by calling @ref bs::ct::Renderer::setGlobalShaderOverride "Renderer::setGlobalShaderOverride()". As the name implies this will apply the shader globally, meaning it will effect lighting of all objects using the deferred pipeline. 

~~~~~~~~~~~~~
SPtr<Shader> customShader = ...; // Import the shader we created above

// Apply the shader
gRenderer().setGlobalShaderOverride(customShader);
~~~~~~~~~~~~~ 
 
# Forward {#surfaceShaders_c}
When it comes to the forward pipeline, vertex, surface and lighting aspects are all handled in the same shader. Its entry point must have the following signature:
 - `float4 fsmain(in VStoFS input) : SV_Target0`
 
Where:
 - *input* - **VStoFS** structure that was output from the vertex shader.
 - Returns the final color of the rendered pixel.
 
The shader needs to include the following mixins:
 - **BasePass** (from `BasePass.bslinc`) - Performs the default per-vertex transformations and outputs data in the **VStoFS** structure. Optionally you can provide your own vertex evaluation code, as described earlier.
 - **ForwardLighting** (from `ForwardLighting.bslinc`) - Contains all relevant lighting evaluation and calculation functionality. Also tells the renderer that the material will be using the forward rendering pipeline.
 
~~~~~~~~~~~~~
#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\ForwardLighting.bslinc"

shader Forward
{
	mixin BasePass;
	mixin ForwardLighting;

	code
	{
		float4 fsmain(in VStoFS input) : SV_Target0
		{
		{
			// ...
		}	
	};
};
~~~~~~~~~~~~~

Your shader then needs to populate the **SurfaceData** structure. This is the same structure as when we talked about deferred pipeline surface shaders. This structure should then be passed to the following method:
 - `float3 calcLighting(float3 worldPosition, float3 screenPosition, float2 uv, SurfaceData surfaceData)`
 
Where:
 - *worldPosition* - World position of the point being shaded.
 - *screenPosition* - Screen position (in pixels) of the point being shaded. z component must contain the depth in [0, 1] range.
 - *uv* - UV coordinates of the point being shaded.
 - *surfaceData* - Populated **SurfaceData** structure.
 - Returns the final pixel color.

~~~~~~~~~~~~~
#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\ForwardLighting.bslinc"

shader Surface
{
	mixin BasePass;
	mixin ForwardLighting;

	code
	{
		SamplerState gAlbedoSamp;
		Texture2D gAlbedoTex;
		
		float4 fsmain(in VStoFS input) : SV_Target0
		{
			// For simplicity we only read the albedo from the texture, and assume other properties are constant
			SurfaceData surfaceData;
			surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, input.uv0);
			surfaceData.worldNormal.xyz = float3(0, 1, 0);
			surfaceData.roughness = 1.0f;
			surfaceData.metalness = 0.0f;
			
			float3 lighting = calcLighting(input.worldPosition.xyz, input.position, input.uv0, surfaceData);
			return float4(lighting, 1.0f);
		}	
	};
};
~~~~~~~~~~~~~

In order to override BRDF and luminance functions, you simply override the same mixins as for the deferred pipeline. Note that in this case we don't need to use subshaders, we just override the mixins directly.

~~~~~~~~~~~~~
#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\ForwardLighting.bslinc"

shader Surface
{
	mixin BasePass;
	mixin ForwardLighting;

	code
	{
		// Code omitted for brevity
	};
};

// Override the BRDF with a basic Lambert BRDF
mixin StandardBRDF
{
	code
	{
		float3 evaluateStandardBRDF(float3 V, float3 L, float specLobeEnergy, SurfaceData surfaceData)
		{
			return surfaceData.albedo.rgb / 3.14f;
		}
	};
};
~~~~~~~~~~~~~

Once the shader is created, it can be used for creating a **Material**, which can then be assigned to a **Renderable** as normal.