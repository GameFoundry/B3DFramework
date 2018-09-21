//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRendererMaterial.h"
#include "Renderer/BsParamBlocks.h"
#include "Renderer/BsLight.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"

namespace bs 
{
	class Bounds;

namespace ct
{
	struct SceneInfo;
	class RendererViewGroup;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Maximum number of lights that can influence an object when basic forward rendering is used. */
	static constexpr UINT32 STANDARD_FORWARD_MAX_NUM_LIGHTS = 8;

	/** Information about a single light, as seen by the lighting shader. */
	struct LightData
	{
		Vector3 position;
		float boundsRadius;
		Vector3 direction;
		float luminance;
		Vector3 spotAngles;
		float attRadiusSqrdInv;
		Vector3 color;
		float srcRadius;
		Vector3 shiftedLightPosition;
		float padding;
	};

	/**	Renderer information specific to a single light. */
	class RendererLight
	{
	public:
		RendererLight(Light* light);

		/** Populates the structure with light parameters. */
		void getParameters(LightData& output) const;

		/** 
		 * Populates the provided parameter block buffer with information about the light. Provided buffer's structure
		 * must match PerLightParamDef.
		 */
		void getParameters(SPtr<GpuParamBlockBuffer>& buffer) const;
		
		/** 
		 * Calculates the light position that is shifted in order to account for area spot lights. For non-spot lights
		 * this method will return normal light position. The position will be shifted back from the light direction, 
		 * magnitude of the shift depending on the source radius.
		 */
		Vector3 getShiftedLightPosition() const;

		Light* internal;
	};

	/** Container for all GBuffer textures. */
	struct GBufferTextures
	{
		SPtr<Texture> albedo;
		SPtr<Texture> normals;
		SPtr<Texture> roughMetal;
		SPtr<Texture> depth;
	};

	/** Allows you to easily bind GBuffer textures to some material. */
	class GBufferParams
	{
	public:
		GBufferParams(GpuProgramType type, const SPtr<GpuParams>& gpuParams);

		/** Binds the GBuffer textures to the pipeline. */
		void bind(const GBufferTextures& gbuffer);
	private:
		SPtr<GpuParams> mParams;

		GpuParamTexture mGBufferA;
		GpuParamTexture mGBufferB;
		GpuParamTexture mGBufferC;
		GpuParamTexture mGBufferDepth;
	};

	/** Helper struct containing all parameters required for forward lighting. */
	struct ForwardLightingParams
	{
		/** 
		 * Initializes the parameters from the provided parameters. 
		 *
		 * @param[in]	params		GPU parameters object to look for the parameters in.
		 * @param[in]	clustered	If true, set up parameters for clustered forward rendering. If false, set up parameters
		 *							for normal forward rendering.
		 */
		void populate(const SPtr<GpuParams>& params, bool clustered);

		/** Binding indices representing where should lights param block buffer be bound to. */
		GpuParamBinding gridParamsBindings[GPT_COUNT];

		/** 
		 * Parameter to which to bind a buffer containing light grid offsets and size, per grid cell. Used for forward
		 * rendering. 
		 */
		GpuParamBuffer gridLightOffsetsAndSizeParam;

		/** Parameter to which to bind a buffer containing all light indices, as mapped by grid offsets & size. */
		GpuParamBuffer gridLightIndicesParam;

		/** Parameter to which to bind light buffer used for forward rendering. */
		GpuParamBuffer lightsBufferParam;

		/** 
		 * Parameter to which to bind a buffer containing reflection probe grid offsets and size, per grid cell. Used for
		 * forward rendering. 
		 */
		GpuParamBuffer gridProbeOffsetsAndSizeParam;

		/** 
		 * Binding for a parameter block containing a list of lights influencing this object. Only used when standard
		 * (non-clustered) forward rendering is used. 
		 */
		GpuParamBinding lightsParamBlockBinding;

		/** 
		 * Binding for a parameter block that contains the number of lights and reflection probes in the light/refl. probe 
		 * parameter blocks. Only used when standard (non-clustered) forward rendering is used.
		 */
		GpuParamBinding lightAndReflProbeParamsParamBlockBinding;
	};

	/** 
	 * Contains lights that are visible from a specific set of views, determined by scene information provided to 
	 * setLights().
	 */
	class VisibleLightData
	{
	public:
		VisibleLightData();

		/** 
		 * Updates the internal buffers with a new set of lights. Before calling make sure that light visibility has
		 * been calculated for the provided view group.
		 */
		void update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup);

		/** Returns a GPU bindable buffer containing information about every light. */
		SPtr<GpuBuffer> getLightBuffer() const { return mLightBuffer; }

		/** 
		 * Scans the list of lights visible in the view frustum to find the ones influencing the object described by
		 * the provided bounds. A maximum number of STANDARD_FORWARD_MAX_NUM_LIGHTS will be output. If there are more
		 * influencing lights, only the most important ones will be returned. 
		 * 
		 * The lights will be output in the following order: directional, radial, spot. @p counts will contain the number
		 * of directional lights (component 'x'), number of radial lights (component 'y') and number of spot lights
		 * (component 'z');
		 * 
		 * update() must have been called with most recent scene/view information before calling this method.
		 */
		void gatherInfluencingLights(const Bounds& bounds, const LightData* (&output)[STANDARD_FORWARD_MAX_NUM_LIGHTS], 
			Vector3I& counts) const;

		/** Returns the number of directional lights in the lights buffer. */
		UINT32 getNumDirLights() const { return mNumLights[0]; }

		/** Returns the number of radial point lights in the lights buffer. */
		UINT32 getNumRadialLights() const { return mNumLights[1]; }

		/** Returns the number of spot point lights in the lights buffer. */
		UINT32 getNumSpotLights() const { return mNumLights[2]; }

		/** Returns the number of visible lights of the specified type. */
		UINT32 getNumLights(LightType type) const { return mNumLights[(UINT32)type]; }

		/** Returns the number of visible shadowed lights of the specified type. */
		UINT32 getNumShadowedLights(LightType type) const { return mNumShadowedLights[(UINT32)type]; }

		/** Returns the number of visible unshadowed lights of the specified type. */
		UINT32 getNumUnshadowedLights(LightType type) const { return mNumLights[(UINT32)type] - mNumShadowedLights[(UINT32)type]; }

		/** Returns a list of all visible lights of the specified type. */
		const Vector<const RendererLight*>& getLights(LightType type) const { return mVisibleLights[(UINT32)type]; }
	private:
		SPtr<GpuBuffer> mLightBuffer;

		UINT32 mNumLights[(UINT32)LightType::Count];
		UINT32 mNumShadowedLights[(UINT32)LightType::Count];

		// These are rebuilt every call to update()
		Vector<const RendererLight*> mVisibleLights[(UINT32)LightType::Count];
		Vector<LightData> mVisibleLightData;
	};

	BS_PARAM_BLOCK_BEGIN(LightsParamDef)
		BS_PARAM_BLOCK_ENTRY_ARRAY(LightData, gLights, STANDARD_FORWARD_MAX_NUM_LIGHTS)
	BS_PARAM_BLOCK_END

	extern LightsParamDef gLightsParamDef;

	BS_PARAM_BLOCK_BEGIN(LightAndReflProbeParamsParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector4I, gLightOffsets)
		BS_PARAM_BLOCK_ENTRY(int, gReflProbeCount)
	BS_PARAM_BLOCK_END

	extern LightAndReflProbeParamsParamDef gLightAndReflProbeParamsParamDef;

	/** @} */
}}
