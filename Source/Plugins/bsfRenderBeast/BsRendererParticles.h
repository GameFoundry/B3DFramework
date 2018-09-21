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
#include "Utility/BsTextureRowAllocator.h"
#include "BsRendererLight.h"
#include "BsRendererReflectionProbe.h"

namespace bs 
{
	struct ParticleMeshRenderData;
	struct ParticleBillboardRenderData;
	struct ParticleRenderData; 
}

namespace bs { namespace ct
{
	class GpuParticleSystem;
	class GpuParticleResources;

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
		BS_PARAM_BLOCK_ENTRY(INT32, gBufferOffset)
	BS_PARAM_BLOCK_END

	extern ParticlesParamDef gParticlesParamDef;

	BS_PARAM_BLOCK_BEGIN(GpuParticlesParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gColorCurveOffset)
		BS_PARAM_BLOCK_ENTRY(Vector2, gColorCurveScale)
		BS_PARAM_BLOCK_ENTRY(Vector2, gSizeScaleFrameIdxCurveOffset)
		BS_PARAM_BLOCK_ENTRY(Vector2, gSizeScaleFrameIdxCurveScale)
	BS_PARAM_BLOCK_END

	extern GpuParticlesParamDef gGpuParticlesParamDef;

	/** Types of forward lighting supported on particle shaders. */
	enum class ParticleForwardLightingType
	{
		/** No forward lighting. */
		None,

		/** Using the modern clustered forward lighting approach (requires compute). */
		Clustered,

		/** Using the old-school standard forward lighting approach. */
		Standard
	};

	/** 
	 * Returns a specific particle rendering shader variation.
	 *
	 * @tparam ORIENT	Particle orientiation mode.
	 * @tparam LOCK_Y	If true, billboard rotation will be locked around the Y axis, otherwise the rotation is free.
	 * @tparam GPU		If true, the particle shader expects input from the GPU simulation instead of the CPU simulation.
	 * @tparam IS_3D	If true, the particle shader will render meshes instead of billboards. 
	 * @tparam FWD		Determines what form of forward lighting should the shader support. 
	 */
	template<ParticleOrientation ORIENT, bool LOCK_Y, bool GPU, bool IS_3D, ParticleForwardLightingType FWD>
	static const ShaderVariation& getParticleShaderVariation()
	{
		static bool initialized = false;
		static Vector<ShaderVariation::Param> params{
			ShaderVariation::Param("ORIENT", (UINT32)ORIENT),
			ShaderVariation::Param("LOCK_Y", LOCK_Y),
			ShaderVariation::Param("GPU", GPU),
			ShaderVariation::Param("IS_3D", IS_3D),
		};

		if(!initialized)
		{
			switch(FWD)
			{
			case ParticleForwardLightingType::Clustered:
				params.push_back(ShaderVariation::Param("CLUSTERED", true));
				break;
			case ParticleForwardLightingType::Standard:
				params.push_back(ShaderVariation::Param("CLUSTERED", false));
				break;
			}

			initialized = true;
		}

		static ShaderVariation variation = ShaderVariation(params);
		return variation;
	}

	/**
	 * Returns a particle material variation matching the provided parameters.
	 * 
	 * @param[in]	orient				Determines in which direction are billboard particles oriented.
	 * @param[in]	lockY				If true, billboard rotation will be locked around the Y axis, otherwise the 
	 *									rotation is free.
	 * @param[in]	gpu					If true, the particle shader expects input from the GPU simulation instead of the
	 *									CPU simulation.
	 * @param[in]	is3d				If true, the particle shader will render meshes instead of billboards.
	 * @param[in]	forwardLighting		Form of forward lighting the shader should support.
	 * @return							Object that can be used for looking up the variation technique in the material. 
	 */
	const ShaderVariation& getParticleShaderVariation(ParticleOrientation orient, bool lockY, bool gpu, bool is3d, 
		ParticleForwardLightingType forwardLighting);

	/** Contains information required for rendering a single particle system. */
	class ParticlesRenderElement : public RenderElement
	{
	public:
		/** Parameters relevant for billboard rendering of the outputs of the particle CPU simulation. */
		struct CpuBillboardSimulationParams
		{
			/** Binding spot for the texture containing position and rotation information. */
			GpuParamTexture positionAndRotTexture;

			/** Binding spot for the texture containing color information. */
			GpuParamTexture colorTexture;

			/** Binding spot for the texture containing size and sub-image index information. */
			GpuParamTexture sizeAndFrameIdxTexture;
		};

		/** Parameters relevant for mesh rendering of the outputs of the particle CPU simulation. */
		struct CpuMeshSimulationParams
		{
			/** Binding spot for the texture containing position. */
			GpuParamTexture positionTexture;

			/** Binding spot for the texture containing color information. */
			GpuParamTexture colorTexture;

			/** Binding spot for the texture containing rotation. */
			GpuParamTexture rotationTexture;

			/** Binding spot for the texture containing size. */
			GpuParamTexture sizeTexture;
		};

		/** Parameters relevant for rendering the outputs of the particle GPU simulation. */
		struct GpuSimulationParams
		{
			/** Binding spot for the texture containing position (.xyz) and time (.w) information. */
			GpuParamTexture positionTimeTexture;

			/** Binding spot for the texture containing 2D size (.xy) and rotation (.z) information. */
			GpuParamTexture sizeRotationTexture;

			/** Binding spot for the texture containing quantized curves used for evaluating various particle properties. */
			GpuParamTexture curvesTexture;
		};

		/** Binding locations for the per-camera param block buffer. */
		GpuParamBinding perCameraBindings[GPT_COUNT];

		/** Binding spot for the buffer containing instance id -> particle index mapping. */
		GpuParamBuffer indicesBuffer;

		/** Parameters relevant for billboard rendering of the outputs of the particle CPU simulation. */
		CpuBillboardSimulationParams paramsCPUBillboard;

		/** Parameters relevant for mesh rendering of the outputs of the particle CPU simulation. */
		CpuMeshSimulationParams paramsCPUMesh;

		/** Parameters relevant for rendering the outputs of the particle GPU simulation. */
		GpuSimulationParams paramsGPU;

		/** Collection of parameters used for direct lighting using the forward rendering path. */
		ForwardLightingParams forwardLightingParams;

		/** Collection of parameters used for image based lighting. */
		ImageBasedLightingParams imageBasedParams;

		/** Number of particles to render. */
		UINT32 numParticles = 0;

		/** True if the particle should be drawn as a 3D mesh instead of a billboard. */
		bool is3D = false;

		/** Checks if the element has all the properties required for rendering. */
		bool isValid() const { return !is3D || mesh != nullptr; }
	};

	/** Contains information about a ParticleSystem, used by the Renderer. */
	struct RendererParticles
	{
		/** Owner particle system. */
		ParticleSystem* particleSystem = nullptr;

		/** Variant of the particle system used for simulating the particles on the GPU. */
		GpuParticleSystem* gpuParticleSystem = nullptr;

		/** Matrix that transforms the particle system to world space. */
		Matrix4 localToWorld = Matrix4::IDENTITY;

		/** Element used for sorting and rendering the particle system. */
		mutable ParticlesRenderElement renderElement;

		/** Parameters used by the particle rendering shader. */
		SPtr<GpuParamBlockBuffer> particlesParamBuffer;

		/** Extra parameters required by the particle rendering shader if the particle system is GPU simulated. */
		SPtr<GpuParamBlockBuffer> gpuParticlesParamBuffer;

		/** Information about the color over lifetime curve stored in the global curve texture. */
		TextureRowAllocation colorCurveAlloc;

		/** Information about the size over lifetime / frame index curve stored in the global curve texture. */
		TextureRowAllocation sizeScaleFrameIdxCurveAlloc;

		/** 
		 * Binds all the GPU program inputs required for rendering a particle system that is being simulated by the CPU. 
		 * 
		 * @param[in]	renderData		Render data representing the state of a CPU simulated particle system. 
		 * @param[in]	view			View the particle system is being rendered from.
		 */
		void bindCPUSimulatedInputs(const ParticleRenderData* renderData, const RendererView& view) const;

		/** 
		 * Binds all the GPU program inputs required for rendering a particle system that is being simulated by the GPU. 
		 * 
		 * @param[in]	gpuSimResources	Resources containing global data for all GPU simulated particle systems.
		 * @param[in]	view			View the particle system is being rendered from.
		 */
		void bindGPUSimulatedInputs(const GpuParticleResources& gpuSimResources, const RendererView& view) const;
	};

	/** Default material used for rendering particles, when no other is available. */
	class DefaultParticlesMat : public RendererMaterial<DefaultParticlesMat> { RMAT_DEF("ParticlesUnlit.bsl"); };

	/**
	 * Contains a set of textures used for rendering a particle system using billboards. Each pixel in a texture represent 
	 * properties of a single particle.
	 */
	struct ParticleBillboardTextures
	{
		SPtr<Texture> positionAndRotation;
		SPtr<Texture> color;
		SPtr<Texture> sizeAndFrameIdx;
		SPtr<GpuBuffer> indices;
	};

	/**
	 * Contains a set of textures used for rendering a particle system using 3D meshes. Each pixel in a texture represent 
	 * properties of a single particle.
	 */
	struct ParticleMeshTextures
	{
		SPtr<Texture> position;
		SPtr<Texture> color;
		SPtr<Texture> size;
		SPtr<Texture> rotation;
		SPtr<GpuBuffer> indices;
	};

	/** Keeps a pool of textures used for the purposes of the particle system. */
	class ParticleTexturePool final
	{
		/** A set of created textures for billboard rendering, per size. */
		struct BillboardBuffersPerSize
		{
			Vector<ParticleBillboardTextures*> buffers;
			UINT32 nextFreeIdx = 0;
		};

		/** A set of created textures for mesh rendering, per size. */
		struct MeshBuffersPerSize
		{
			Vector<ParticleMeshTextures*> buffers;
			UINT32 nextFreeIdx = 0;
		};

	public:
		~ParticleTexturePool();

		/** 
		 * Returns a set of textures used for particle billboard rendering. The textures will contain the pixel data from 
		 * the provided @p simulationData. Returned textures will remain in-use until the next call to clear().
		 */
		const ParticleBillboardTextures* alloc(const ParticleBillboardRenderData& simulationData);

		/** 
		 * Returns a set of textures used for particle mesh rendering. The textures will contain the pixel data from 
		 * the provided @p simulationData. Returned textures will remain in-use until the next call to clear().
		 */
		const ParticleMeshTextures* alloc(const ParticleMeshRenderData& simulationData);

		/** Frees all allocates textures and makes them available for re-use. */
		void clear();

	private:
		/** Creates a new set of textures for billboard rendering, with @p size width and height. */
		ParticleBillboardTextures* createNewBillboardTextures(UINT32 size);

		/** Creates a new set of textures for mesh rendering, with @p size width and height. */
		ParticleMeshTextures* createNewMeshTextures(UINT32 size);

		UnorderedMap<UINT32, BillboardBuffersPerSize> mBillboardBufferList;
		PoolAlloc<sizeof(ParticleBillboardTextures), 32> mBillboardAlloc;

		UnorderedMap<UINT32, MeshBuffersPerSize> mMeshBufferList;
		PoolAlloc<sizeof(ParticleMeshTextures), 32> mMeshAlloc;
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
		 * properties used for billboard particle rendering (position/color/etc).
		 */
		ParticleTexturePool& getTexturePool() { return mTexturePool; }

		/** Draws @p count quads used for billboard rendering, using instanced drawing. */
		void drawBillboards(UINT32 count);

		/** 
		 * Updates the provided indices buffer so they particles are sorted from further to nearest with respect to
		 * some reference point. 
		 * 
		 * @param[in]	refPoint		Reference point respect to which to determine the distance of individual particles.
		 *								Should be in the simulation space of the particle system.
		 * @param[in]	positions		Buffer containing positions of individual particles.
		 * @param[in]	numParticles	Number of particles in the provided position and indices buffers.
		 * @param[in]	stride			Offset between positions in the @positions buffer, in number of floats.
		 * @param[out]	indices			Index buffer that will be sorted according to the particle distance, in descending
		 *								order.
		 */
		static void sortByDistance(const Vector3& refPoint, const PixelData& positions, UINT32 numParticles, 
			UINT32 stride, Vector<UINT32>& indices);
	private:
		ParticleTexturePool mTexturePool;
		Members* m;
	};

	/** @} */
}}
