//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Components/B3DLight.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"

namespace b3d
{
	namespace render
	{
		struct SceneInfo;
		class RendererViewGroup;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Maximum number of lights that can influence an object when basic forward rendering is used. */
		static constexpr u32 kStandardForwardMaxNumLights = 8;

		/** Information about a single light, as seen by the lighting shader. */
		struct LightData
		{
			Vector3 Position;
			float BoundsRadius;
			Vector3 Direction;
			float Luminance;
			Vector3 SpotAngles;
			float AttRadiusSqrdInv;
			Vector3 Color;
			float SrcRadius;
			Vector3 ShiftedLightPosition;
			float Padding;
		};

		/**	Renderer information specific to a single light. */
		class RendererLight
		{
		public:
			RendererLight(Light* light);

			/** Populates the structure with light parameters. */
			void GetParameters(LightData& output) const;

			/**
			 * Populates the provided uniform buffer with information about the light. Provided buffer's structure
			 * must match PerLightUniformDefinition.
			 */
			void PopulateUniformBuffer(SPtr<GpuBuffer>& buffer, u32 index = 0) const;

			/**
			 * Calculates the light position that is shifted in order to account for area spot lights. For non-spot lights
			 * this method will return normal light position. The position will be shifted back from the light direction,
			 * magnitude of the shift depending on the source radius.
			 */
			Vector3 GetShiftedLightPosition() const;

			Light* Internal;
		};

		/** Container for all GBuffer textures. */
		struct GBufferTextures
		{
			SPtr<Texture> Albedo;
			SPtr<Texture> Normals;
			SPtr<Texture> RoughMetal;
			SPtr<Texture> Depth;
		};

		/** Allows you to easily bind GBuffer textures to some material. */
		class GBufferParameterBinding
		{
		public:

			static constexpr const char* kAlbedoTextureName = "gGBufferATex";
			static constexpr const char* kNormalsTextureName = "gGBufferBTex";
			static constexpr const char* kRoughMetalTextureName = "gGBufferCTex";
			static constexpr const char* kDepthTextureName = "gDepthBufferTex";
			static constexpr const char* kDepthSamplerName = "gDepthBufferSamp";

			/** Initializes the required parameters. To be called once before use. */
			void Initialize(GpuDevice& gpuDevice, GpuProgramType type, const SPtr<GpuParameterSet>& gpuParams);

			/** Binds the GBuffer textures to the pipeline. */
			void Bind(const GBufferTextures& gbuffer);

			/** Assigns the provided GBuffer textures to the provided GPU parameters object. */
			static void Set(GpuDevice& gpuDevice, const SPtr<GpuParameterSet>& gpuParameters, const GBufferTextures& textures);

		private:
			SPtr<GpuParameterSet> mParams;

			GpuParameterSampledTexture mGBufferA;
			GpuParameterSampledTexture mGBufferB;
			GpuParameterSampledTexture mGBufferC;
			GpuParameterSampledTexture mGBufferDepth;
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
			void Populate(const SPtr<GpuParameterSet>& params, bool clustered);

			/** Binding indices representing where should lights param block buffer be bound to. */
			GpuParameterBinding GridParamsBinding;

			/**
			 * Parameter to which to bind a buffer containing light grid offsets and size, per grid cell. Used for forward
			 * rendering.
			 */
			GpuParameterStorageBuffer GridLightOffsetsAndSizeParam;

			/** Parameter to which to bind a buffer containing all light indices, as mapped by grid offsets & size. */
			GpuParameterStorageBuffer GridLightIndicesParam;

			/** Parameter to which to bind light buffer used for forward rendering. */
			GpuParameterStorageBuffer LightsBufferParam;

			/**
			 * Parameter to which to bind a buffer containing reflection probe grid offsets and size, per grid cell. Used for
			 * forward rendering.
			 */
			GpuParameterStorageBuffer GridProbeOffsetsAndSizeParam;

			/**
			 * Binding for a parameter block containing a list of lights influencing this object. Only used when standard
			 * (non-clustered) forward rendering is used.
			 */
			GpuParameterBinding LightsParamBlockBinding;

			/**
			 * Binding for a parameter block that contains the number of lights and reflection probes in the light/refl. probe
			 * parameter blocks. Only used when standard (non-clustered) forward rendering is used.
			 */
			GpuParameterBinding LightAndReflProbeParamsParamBlockBinding;
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
			void Update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup);

			/** Returns a GPU bindable buffer containing information about every light. */
			SPtr<GpuBuffer> GetLightBuffer() const { return mLightBuffer; }

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
			void GatherInfluencingLights(const Bounds& bounds, const LightData* (&output)[kStandardForwardMaxNumLights], Vector3I& counts) const;

			/** Returns the number of directional lights in the lights buffer. */
			u32 GetDirectionalLightCount() const { return mLightCounts[0]; }

			/** Returns the number of radial point lights in the lights buffer. */
			u32 GetRadialLightCount() const { return mLightCounts[1]; }

			/** Returns the number of spot point lights in the lights buffer. */
			u32 GetSpotLightCount() const { return mLightCounts[2]; }

			/** Returns the number of visible lights of the specified type. */
			u32 GetLightCount(LightType type) const { return mLightCounts[(u32)type]; }

			/** Returns the number of visible shadowed lights of the specified type. */
			u32 GetShadowedLightCount(LightType type) const { return mShadowedLightCounts[(u32)type]; }

			/** Returns the number of visible unshadowed lights of the specified type. */
			u32 GetUnshadowedLightCount(LightType type) const { return mLightCounts[(u32)type] - mShadowedLightCounts[(u32)type]; }

			/** Returns a list of all visible lights of the specified type. */
			const Vector<const RendererLight*>& GetLights(LightType type) const { return mVisibleLights[(u32)type]; }

		private:
			SPtr<GpuBuffer> mLightBuffer;

			u32 mLightCounts[(u32)LightType::Count];
			u32 mShadowedLightCounts[(u32)LightType::Count];

			// These are rebuilt every call to update()
			Vector<const RendererLight*> mVisibleLights[(u32)LightType::Count];
			Vector<LightData> mVisibleLightData;
		};

		B3D_UNIFORM_BUFFER_BEGIN(LightsUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER_ARRAY(LightData, gLights, kStandardForwardMaxNumLights)
		B3D_UNIFORM_BUFFER_END

		extern LightsUniformDefinition gLightsUniformDefinition;

		B3D_UNIFORM_BUFFER_BEGIN(LightAndReflProbeParamsUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4I, gLightOffsets)
			B3D_UNIFORM_BUFFER_MEMBER(int, gReflProbeCount)
		B3D_UNIFORM_BUFFER_END

		extern LightAndReflProbeParamsUniformDefinition gLightAndReflProbeParamsUniformDefinition;

		/** @} */
	} // namespace render
} // namespace b3d
