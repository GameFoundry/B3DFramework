//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRenderElement.h"
#include "Renderer/BsParamBlocks.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "Material/BsShaderVariation.h"
#include "Particles/BsParticleSystem.h"
#include "Allocators/BsPoolAlloc.h"
#include "Renderer/BsRendererMaterial.h"

namespace bs { struct ParticleCPUSimulationData; }

namespace bs { namespace ct
{
	class GpuParticleSystem;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(ParticlesParamDef)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gWorldTfrm)
		BS_PARAM_BLOCK_ENTRY(Vector4, gSubImageSize)
		BS_PARAM_BLOCK_ENTRY(Vector2, gUVOffset)
		BS_PARAM_BLOCK_ENTRY(Vector2, gUVScale)
		BS_PARAM_BLOCK_ENTRY(Vector3, gAxisUp)
		BS_PARAM_BLOCK_ENTRY(INT32, gTexSize)
		BS_PARAM_BLOCK_ENTRY(Vector3, gAxisRight)
	BS_PARAM_BLOCK_END

	extern ParticlesParamDef gParticlesParamDef;

	/** 
	 * Returns a specific particle rendering shader variation.
	 *
	 * @tparam ORIENT	Particle orientiation mode.
	 * @tparam LOCK_Y	If true, billboard rotation will be locked around the Y axis, otherwise the rotation is free.
	 * @tparam GPU		If true, the particle shader expects input from the GPU simulation instead of the CPU simulation.
	 */
	template<ParticleOrientation ORIENT, bool LOCK_Y, bool GPU>
	static const ShaderVariation& getParticleShaderVariation()
	{
		static ShaderVariation variation = ShaderVariation(
			Vector<ShaderVariation::Param>{
				ShaderVariation::Param("ORIENT", (UINT32)ORIENT),
				ShaderVariation::Param("LOCK_Y", LOCK_Y),
				ShaderVariation::Param("GPU", GPU),
		});

		return variation;
	}

	/**
	 * Returns a particle material variation matching the provided parameters.
	 * 
	 * @param[in]	orient	Determines in which direction are billboard particles oriented.
	 * @param[in]	lockY	If true, billboard rotation will be locked around the Y axis, otherwise the rotation is free.
	 * @param[in]	gpu		If true, the particle shader expects input from the GPU simulation instead of the CPU 
	 *						simulation.
	 * @return				Object that can be used for looking up the variation technique in the material. 
	 */
	const ShaderVariation& getParticleShaderVariation(ParticleOrientation orient, bool lockY, bool gpu);

	/** Contains information required for rendering a single particle system. */
	class ParticlesRenderElement : public RenderElement
	{
	public:
		/** Parameters relevant for rendering the outputs of the particle CPU simulation. */
		struct CPUSimulationParams
		{
			/** Binding spot for the texture containing position and rotation information. */
			GpuParamTexture positionAndRotTexture;

			/** Binding spot for the texture containing color information. */
			GpuParamTexture colorTexture;

			/** Binding spot for the texture containing size and sub-image index information. */
			GpuParamTexture sizeAndFrameIdxTexture;
		};

		/** Parameters relevant for rendering the outputs of the particle GPU simulation. */
		struct GPUSimulationParams
		{
			/** Binding spot for the texture containing position and time information. */
			GpuParamTexture positionAndTimeTexture;
		};

		/** Binding locations for the per-camera param block buffer. */
		GpuParamBinding perCameraBindings[GPT_COUNT];

		/** Binding spot for the buffer containing instance id -> particle index mapping. */
		GpuParamBuffer indicesBuffer;

		/** Parameters relevant for rendering the outputs of the particle CPU simulation. */
		CPUSimulationParams paramsCPU;

		/** Parameters relevant for rendering the outputs of the particle GPU simulation. */
		GPUSimulationParams paramsGPU;

		/** Number of particles to render. */
		UINT32 numParticles = 0;
	};

	/** Contains information about a ParticleSystem, used by the Renderer. */
	struct RendererParticles
	{
		/** Owner particle system. */
		ParticleSystem* particleSystem = nullptr;

		/** Variant of the particle system used for simulating the particles on the GPU. */
		GpuParticleSystem* gpuParticleSystem = nullptr;

		/** Element used for sorting and rendering the particle system. */
		mutable ParticlesRenderElement renderElement;

		/** Parameters used by the particle rendering shader. */
		SPtr<GpuParamBlockBuffer> particlesParamBuffer;
	};

	/** Default material used for rendering particles, when no other is available. */
	class DefaultParticlesMat : public RendererMaterial<DefaultParticlesMat> { RMAT_DEF("ParticlesUnlit.bsl"); };

	/** Keeps a pool of textures used for the purposes of the particle system. */
	class ParticleTexturePool final
	{
		/** A set of created textures, per size. */
		struct BuffersPerSize
		{
			Vector<ParticleTextures*> buffers;
			UINT32 nextFreeIdx = 0;
		};

	public:
		~ParticleTexturePool();

		/** 
		 * Returns a set of textures containing the pixel data from the provided @p simulationData. Returned textures
		 * will remain in-use until the next call to clear().
		 */
		const ParticleTextures* alloc(const ParticleCPUSimulationData& simulationData);

		/** Frees all allocates textures and makes them available for re-use. */
		void clear();

	private:
		/** Creates a new set of textures with @p size width and height. */
		ParticleTextures* createNewTextures(UINT32 size);

		UnorderedMap<UINT32, BuffersPerSize> mBufferList;
		PoolAlloc<sizeof(ParticleTextures), 32> mAlloc;
	};

	/** Handles internal logic for rendering of particle systems. */
	class ParticleRenderer final : public Module<ParticleRenderer>
	{
		struct Members;
	public:
		ParticleRenderer();
		~ParticleRenderer();

		/** 
		 * Returns a texture pool object that can be used for allocating textures required for holding particle system
		 * properties like position/color/etc.
		 */
		ParticleTexturePool& getTexturePool() { return mTexturePool; }

		/** Draws @p count quads used for billboard rendering, using instanced drawing. */
		void drawBillboards(UINT32 count);

	private:
		ParticleTexturePool mTexturePool;
		Members* m;
	};

	/** @} */
}}
