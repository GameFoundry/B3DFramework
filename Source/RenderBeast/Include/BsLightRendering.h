//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRendererMaterial.h"
#include "BsParamBlocks.h"
#include "BsLight.h"

namespace bs { namespace ct
{
	struct SceneInfo;
	class RendererViewGroup;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Information about a single light, as seen by the lighting shader. */
	struct LightData
	{
		Vector3 position;
		float attRadius;
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
		
		Light* internal;
	};

	/** Allows you to easily bind GBuffer textures to some material. */
	class GBufferParams
	{
	public:
		GBufferParams(const SPtr<Material>& material, const SPtr<GpuParamsSet>& paramsSet);

		/** Binds the GBuffer textures to the pipeline. */
		void bind(const RenderTargets& renderTargets);

	private:
		MaterialParamTexture mGBufferA;
		MaterialParamTexture mGBufferB;
		MaterialParamTexture mGBufferC;
		MaterialParamTexture mGBufferDepth;
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

		// These are rebuilt every call to setLights()
		Vector<const RendererLight*> mVisibleLights[(UINT32)LightType::Count];

		// Helpers to avoid memory allocations
		Vector<LightData> mLightDataTemp;
	};

	BS_PARAM_BLOCK_BEGIN(TiledLightingParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector4I, gLightCounts)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gLightStrides)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gFramebufferSize)
	BS_PARAM_BLOCK_END

	extern TiledLightingParamDef gTiledLightingParamDef;

	/** Functionality common to all versions of TiledDeferredLightingMat<T>. */
	class TiledDeferredLighting
	{
	public:
		TiledDeferredLighting(const SPtr<Material>& material, const SPtr<GpuParamsSet>& paramsSet, UINT32 sampleCount);

		/** Binds the material for rendering, sets up parameters and executes it. */
		void execute(const SPtr<RenderTargets>& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera, bool noLighting,
			bool noShadows);

		/** Binds all the active lights. */
		void setLights(const VisibleLightData& lightData);

		static const UINT32 TILE_SIZE;
	private:
		UINT32 mSampleCount;
		SPtr<Material> mMaterial;
		SPtr<GpuParamsSet> mParamsSet;

		GBufferParams mGBufferParams;

		Vector4I mUnshadowedLightCounts;
		Vector4I mLightCounts;
		Vector2I mLightStrides;
		GpuParamBuffer mLightBufferParam;
		GpuParamLoadStoreTexture mOutputTextureParam;
		GpuParamBuffer mOutputBufferParam;

		SPtr<GpuParamBlockBuffer> mParamBuffer;
	};

	/** Interface implemented by all versions of TTiledDeferredLightingMat<T>. */
	class ITiledDeferredLightingMat
	{
	public:
		virtual ~ITiledDeferredLightingMat() {}

		/** @copydoc TiledDeferredLighting::execute() */
		virtual void execute(const SPtr<RenderTargets>& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera,
			bool noLighting, bool noShadows) = 0;

		/** @copydoc TiledDeferredLighting::setLights() */
		virtual void setLights(const VisibleLightData& lightData) = 0;
	};

	/** Shader that performs a lighting pass over data stored in the Gbuffer. */
	template<int MSAA_COUNT>
	class TTiledDeferredLightingMat : public ITiledDeferredLightingMat, public RendererMaterial<TTiledDeferredLightingMat<MSAA_COUNT>>
	{
		RMAT_DEF("TiledDeferredLighting.bsl");

	public:
		TTiledDeferredLightingMat();

		/** @copydoc ITiledDeferredLightingMat::execute() */
		void execute(const SPtr<RenderTargets>& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera,
			bool noLighting, bool noShaodws) override;

		/** @copydoc ITiledDeferredLightingMat::setLights() */
		void setLights(const VisibleLightData& lightData) override;
	private:
		TiledDeferredLighting mInternal;
	};

	/** Contains instances for all types of tile deferred lighting materials. */
	class TiledDeferredLightingMaterials
	{
	public:
		TiledDeferredLightingMaterials();
		~TiledDeferredLightingMaterials();

		/**
		 * Returns a version of the tile-deferred lighting material that matches the parameters.
		 * 
		 * @param[in]   msaa					Number of samples per pixel.
		 */
		ITiledDeferredLightingMat* get(UINT32 msaa);

	private:
		ITiledDeferredLightingMat* mInstances[4];
	};

	BS_PARAM_BLOCK_BEGIN(FlatFramebufferToTextureParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gFramebufferSize)
		BS_PARAM_BLOCK_ENTRY(INT32, gSampleCount)
	BS_PARAM_BLOCK_END

	/** Shader that copies a flattened framebuffer into a multisampled texture. */
	class FlatFramebufferToTextureMat : public RendererMaterial<FlatFramebufferToTextureMat>
	{
		RMAT_DEF("FlatFramebufferToTexture.bsl");

	public:
		FlatFramebufferToTextureMat();

		/** Binds the material for rendering, sets up parameters and executes it. */
		void execute(const SPtr<GpuBuffer>& flatFramebuffer, const SPtr<Texture>& target);
	private:
		GpuParamBuffer mInputParam;
		SPtr<GpuParamBlockBuffer> mParamBuffer;
	};

	/** @} */
}}
