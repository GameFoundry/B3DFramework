//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRenderElement.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "RenderAPI/B3DGpuBufferPool.h"
#include "Material/B3DShaderVariation.h"
#include "Allocators/B3DPoolAlloc.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Utility/B3DTextureRowAllocator.h"
#include "B3DRendererObject.h"
#include "B3DRendererLight.h"
#include "B3DRendererReflectionProbe.h"
#include "Shading/B3DGpuParticleSimulation.h"

namespace b3d
{
	struct ParticleMeshRenderData;
	struct ParticleBillboardRenderData;
	struct ParticleRenderData;
} // namespace b3d

namespace b3d
{
	namespace render
	{
		class GpuParticleSystem;
		class GpuParticleResources;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(ParticlesUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gSubImageSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gUVOffset)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gUVScale)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gAxisUp)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gTexSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gAxisRight)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gBufferOffset)
		B3D_UNIFORM_BUFFER_END

		extern ParticlesUniformDefinition gParticlesUniformDefinition;

		B3D_UNIFORM_BUFFER_BEGIN(GpuParticlesUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gColorCurveOffset)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gColorCurveScale)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gSizeScaleFrameIdxCurveOffset)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gSizeScaleFrameIdxCurveScale)
		B3D_UNIFORM_BUFFER_END

		extern GpuParticlesUniformDefinition gGpuParticlesUniformDefinition;

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
		template <ParticleOrientation ORIENT, bool LOCK_Y, bool GPU, bool IS_3D, ParticleForwardLightingType FWD>
		static const ShaderVariationParameters& GetParticleShaderVariation()
		{
			static bool initialized = false;
			static TInlineArray<ShaderVariationParameter, 4> params({
				ShaderVariationParameter("ORIENT", (u32)ORIENT),
				ShaderVariationParameter("LOCK_Y", LOCK_Y),
				ShaderVariationParameter("GPU", GPU),
				ShaderVariationParameter("IS_3D", IS_3D),
			});

			if(!initialized)
			{
				switch(FWD)
				{
				case ParticleForwardLightingType::Clustered:
					params.Add(ShaderVariationParameter("CLUSTERED", true));
					break;
				case ParticleForwardLightingType::Standard:
					params.Add(ShaderVariationParameter("CLUSTERED", false));
					break;
				case ParticleForwardLightingType::None:
					break;
				}

				initialized = true;
			}

			static ShaderVariationParameters variation = ShaderVariationParameters(params);
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
		const ShaderVariationParameters& GetParticleShaderVariation(ParticleOrientation orient, bool lockY, bool gpu, bool is3d, ParticleForwardLightingType forwardLighting);

		/** Contains information required for rendering a single particle system. */
		class ParticlesRenderElement : public RenderElement
		{
		public:
			/** Parameters relevant for billboard rendering of the outputs of the particle CPU simulation. */
			struct CpuBillboardSimulationParams
			{
				/** Binding spot for the texture containing position and rotation information. */
				GpuParameterSampledTexture PositionAndRotTexture;

				/** Binding spot for the texture containing color information. */
				GpuParameterSampledTexture ColorTexture;

				/** Binding spot for the texture containing size and sub-image index information. */
				GpuParameterSampledTexture SizeAndFrameIdxTexture;
			};

			/** Parameters relevant for mesh rendering of the outputs of the particle CPU simulation. */
			struct CpuMeshSimulationParams
			{
				/** Binding spot for the texture containing position. */
				GpuParameterSampledTexture PositionTexture;

				/** Binding spot for the texture containing color information. */
				GpuParameterSampledTexture ColorTexture;

				/** Binding spot for the texture containing rotation. */
				GpuParameterSampledTexture RotationTexture;

				/** Binding spot for the texture containing size. */
				GpuParameterSampledTexture SizeTexture;
			};

			/** Parameters relevant for rendering the outputs of the particle GPU simulation. */
			struct GpuSimulationParams
			{
				/** Binding spot for the texture containing position (.xyz) and time (.w) information. */
				GpuParameterSampledTexture PositionTimeTexture;

				/** Binding spot for the texture containing 2D size (.xy) and rotation (.z) information. */
				GpuParameterSampledTexture SizeRotationTexture;

				/** Binding spot for the texture containing quantized curves used for evaluating various particle properties. */
				GpuParameterSampledTexture CurvesTexture;
			};

			/** Parameter for binding the per-camera uniform buffer. */
			GpuParameterUniformBuffer PerCameraUniformBufferParameter;

			/** Parameter for binding the particle params uniform buffer. */
			GpuParameterUniformBuffer ParticlesUniformBufferParameter;

			/** Binding spot for the buffer containing instance id -> particle index mapping. */
			GpuParameterStorageBuffer IndicesBuffer;

			/** Optional texture input for the depth buffer. */
			GpuParameterSampledTexture DepthInputTexture;

			/** Parameters relevant for billboard rendering of the outputs of the particle CPU simulation. */
			CpuBillboardSimulationParams ParamsCpuBillboard;

			/** Parameters relevant for mesh rendering of the outputs of the particle CPU simulation. */
			CpuMeshSimulationParams ParamsCpuMesh;

			/** Parameters relevant for rendering the outputs of the particle GPU simulation. */
			GpuSimulationParams ParamsGpu;

			/** Collection of parameters used for direct lighting using the forward rendering path. */
			ForwardLightingParams ForwardLightingParams;

			/** Collection of parameters used for image based lighting. */
			ImageBasedLightingParameterBinding ImageBasedParams;

			/** Number of particles to render. */
			u32 NumParticles = 0;

			/** Dynamic buffer offset for GPU particle parameters (GPU-simulated only). */
			u32 GpuParticlesParamBufferOffset = 0;

			/** True if the particle system uses GPU simulation. */
			bool IsGpuSimulated = false;

			/** True if the particle should be drawn as a 3D mesh instead of a billboard. */
			bool Is3D = false;

			/** Checks if the element has all the properties required for rendering. */
			bool IsValid() const { return !Is3D || Mesh != nullptr; }

			void Draw(GpuCommandBuffer& commandBuffer) const override;
		};

		/** Contains information about a ParticleSystem, used by the Renderer. */
		struct RendererParticles : RendererObject
		{
			/** Owner particle system. */
			ParticleSystem* ParticleSystem = nullptr;

			/** Variant of the particle system used for simulating the particles on the GPU. */
			GpuParticleSystem* GpuParticleSystem = nullptr;

			/** Element used for sorting and rendering the particle system. */
			mutable ParticlesRenderElement RenderElement;

			/** Suballocation for GPU particle parameters (GPU-simulated only). */
			GpuBufferSuballocation GpuParticlesParamSuballocation;

			/** Information about the color over lifetime curve stored in the global curve texture. */
			TextureRowAllocation ColorCurveAlloc;

			/** Information about the size over lifetime / frame index curve stored in the global curve texture. */
			TextureRowAllocation SizeScaleFrameIdxCurveAlloc;

			/** Updates the per-object data from the current ParticleSystem state. */
			void UpdatePerObjectData();

			/**
			 * Binds all the GPU program inputs required for rendering a particle system that is being simulated by the CPU.
			 *
			 * @param renderData	Render data representing the state of a CPU simulated particle system.
			 * @param view			View the particle system is being rendered from.
			 */
			void BindCpuSimulatedInputs(const ParticleRenderData* renderData, const RendererView& view) const;

			/**
			 * Binds all the GPU program inputs required for rendering a particle system that is being simulated by the GPU.
			 *
			 * @param gpuSimResources	Resources containing global data for all GPU simulated particle systems.
			 * @param view				View the particle system is being rendered from.
			 */
			void BindGpuSimulatedInputs(const GpuParticleResources& gpuSimResources, const RendererView& view) const;

		private:
			/**
			 * Allocates a transient uniform buffer and populates it with particle parameters.
			 *
			 * @param texSize		Size of the particle data texture.
			 * @param bufferOffset	Offset into the particle index buffer.
			 */
			void PopulateAndBindParticlesUniformBuffer(i32 texSize, i32 bufferOffset) const;
		};

		/** Default material used for rendering particles, when no other is available. */
		class DefaultParticlesMat : public RendererMaterial<DefaultParticlesMat>
		{
			RMAT_DEF("ParticlesUnlit.bsl");
		};

		/**
		 * Contains a set of textures used for rendering a particle system using billboards. Each pixel in a texture represent
		 * properties of a single particle.
		 */
		struct ParticleBillboardTextures
		{
			SPtr<Texture> PositionAndRotation;
			SPtr<Texture> Color;
			SPtr<Texture> SizeAndFrameIdx;
			SPtr<GpuBuffer> Indices;
		};

		/**
		 * Contains a set of textures used for rendering a particle system using 3D meshes. Each pixel in a texture represent
		 * properties of a single particle.
		 */
		struct ParticleMeshTextures
		{
			SPtr<Texture> Position;
			SPtr<Texture> Color;
			SPtr<Texture> Size;
			SPtr<Texture> Rotation;
			SPtr<GpuBuffer> Indices;
		};

		/** Keeps a pool of textures used for the purposes of the particle system. */
		class ParticleTexturePool final
		{
			/** A set of created textures for billboard rendering, per size. */
			struct BillboardBuffersPerSize
			{
				Vector<ParticleBillboardTextures*> Buffers;
				u32 NextFreeIdx = 0;
			};

			/** A set of created textures for mesh rendering, per size. */
			struct MeshBuffersPerSize
			{
				Vector<ParticleMeshTextures*> Buffers;
				u32 NextFreeIdx = 0;
			};

		public:
			~ParticleTexturePool();

			/**
			 * Returns a set of textures used for particle billboard rendering. The textures will contain the pixel data from
			 * the provided @p simulationData. Returned textures will remain in-use until the next call to clear().
			 */
			const ParticleBillboardTextures* Alloc(const ParticleBillboardRenderData& simulationData);

			/**
			 * Returns a set of textures used for particle mesh rendering. The textures will contain the pixel data from
			 * the provided @p simulationData. Returned textures will remain in-use until the next call to clear().
			 */
			const ParticleMeshTextures* Alloc(const ParticleMeshRenderData& simulationData);

			/** Frees all allocates textures and makes them available for re-use. */
			void Clear();

		private:
			/** Creates a new set of textures for billboard rendering, with @p size width and height. */
			ParticleBillboardTextures* CreateNewBillboardTextures(u32 size);

			/** Creates a new set of textures for mesh rendering, with @p size width and height. */
			ParticleMeshTextures* CreateNewMeshTextures(u32 size);

			UnorderedMap<u32, BillboardBuffersPerSize> mBillboardBufferList;
			PoolAlloc<sizeof(ParticleBillboardTextures), 32> mBillboardAlloc;

			UnorderedMap<u32, MeshBuffersPerSize> mMeshBufferList;
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
			ParticleTexturePool& GetTexturePool() { return mTexturePool; }

			/** Draws @p count quads used for billboard rendering, using instanced drawing. */
			void DrawBillboards(GpuCommandBuffer& commandBuffer, u32 count);

			/**
			 * Updates the provided indices buffer so they particles are sorted from further to nearest with respect to
			 * some reference point.
			 *
			 * @param[in]	refPoint		Reference point respect to which to determine the distance of individual particles.
			 *								Should be in the simulation space of the particle system.
			 * @param[in]	positions		Buffer containing positions of individual particles.
			 * @param[in]	numParticles	Number of particles in the provided position and indices buffers.
			 * @param[in]	stride			Offset between positions in the @p positions buffer, in number of floats.
			 * @param[out]	indices			Index buffer that will be sorted according to the particle distance, in descending
			 *								order.
			 */
			static void SortByDistance(const Vector3& refPoint, const PixelData& positions, u32 numParticles, u32 stride, Vector<u32>& indices);

		private:
			ParticleTexturePool mTexturePool;
			Members* m;
		};

		/** @} */
	} // namespace render
} // namespace b3d
