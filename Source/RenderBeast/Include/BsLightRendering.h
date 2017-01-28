//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRendererMaterial.h"
#include "BsParamBlocks.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Information about a single light, as seen by the lighting shader. */
	struct LightData
	{
		Vector3 position;
		float radius;
		Vector3 direction;
		float intensity;
		Vector3 spotAngles;
		float radiusSqrdInv;
		Vector3 color;
	};

	/**	Renderer information specific to a single light. */
	class RendererLight
	{
	public:
		RendererLight(Light* light);

		/** Populates the structure with light parameters. */
		void getParameters(LightData& output) const;

		/** Gets the internal light representation. */
		Light* getInternal() const { return mInternal; }

	private:
		Light* mInternal;
	};

	BS_PARAM_BLOCK_BEGIN(TiledLightingParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector3I, gNumLightsPerType)
	BS_PARAM_BLOCK_END

	extern TiledLightingParamDef gTiledLightingParamDef;

	/** Shader that performs a lighting pass over data stored in the Gbuffer. */
	class TiledDeferredLightingMat : public RendererMaterial<TiledDeferredLightingMat>
	{
		RMAT_DEF("TiledDeferredLighting.bsl");

	public:
		TiledDeferredLightingMat();

		/** Binds the material for rendering, sets up parameters and executes it. */
		void execute(const SPtr<RenderTargets>& gbuffer, const SPtr<GpuParamBlockBuffer>& perCamera);

		/** Binds all the active lights. */
		void setLights(const Vector<LightData> (&lightData)[3]);
	private:
		GpuParamTexture mGBufferA;
		GpuParamTexture mGBufferB;
		GpuParamTexture mGBufferDepth;

		GpuParamBuffer mDirLightBufferParam;
		GpuParamBuffer mPointLightBufferParam;
		GpuParamBuffer mSpotLightBufferParam;

		GpuParamLoadStoreTexture mOutputParam;

		SPtr<GpuParamBlockBuffer> mParamBuffer;
		SPtr<GpuBuffer> mLightBuffers[3];

		static const UINT32 TILE_SIZE;
	};

	/** @} */
}}