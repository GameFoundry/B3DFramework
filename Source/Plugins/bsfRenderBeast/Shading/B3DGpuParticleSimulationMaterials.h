//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "B3DGpuParticleConstants.h"

namespace b3d { namespace render
{
	class GpuParticleSystem;
	class GpuParticleResources;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	B3D_UNIFORM_BUFFER_BEGIN(GpuParticleTileVertexUniformDefinition)
		B3D_UNIFORM_BUFFER_MEMBER(Vector4, gUVToNDC)
	B3D_UNIFORM_BUFFER_END

	inline GpuParticleTileVertexUniformDefinition gGpuParticleTileVertexUniformDefinition;

	/** Material used for clearing tiles in the texture used for particle GPU simulation. */
	class GpuParticleClearMaterial : public RendererMaterial<GpuParticleClearMaterial>
	{
		RMAT_DEF_CUSTOMIZED("GpuParticleClear.bsl")

	public:
		GpuParticleClearMaterial() = default;

		/** Populates GPU parameters for rendering using this material. */
		static void PopulateParameters(const SPtr<GpuParameterSet>& gpuParameters, const SPtr<GpuBuffer>& vertexInputBuffer, const SPtr<GpuBuffer>& tileUVs);
	};

	/** Material used for adding new particles into the particle state textures. */
	class GpuParticleInjectMaterial : public RendererMaterial<GpuParticleInjectMaterial>
	{
		RMAT_DEF("GpuParticleInject.bsl");

	public:
		GpuParticleInjectMaterial() = default;

		/** Populates GPU parameters for rendering using this material. */
		static void PopulateParameters(const SPtr<GpuParameterSet>& gpuParameters, const SPtr<GpuBuffer>& vertexInputBuffer);
	};

	/** Material used for adding new curves into the curve texture. */
	class GpuParticleCurveInjectMaterial : public RendererMaterial<GpuParticleCurveInjectMaterial>
	{
		RMAT_DEF("GpuParticleCurveInject.bsl");

	public:
		GpuParticleCurveInjectMaterial() = default;

		/** Populates GPU parameters for rendering using this material. */
		void Prepare(const SPtr<GpuBuffer>& vertexInputBuffer);
	};

	B3D_UNIFORM_BUFFER_BEGIN(VectorFieldUniformDefinition)
		B3D_UNIFORM_BUFFER_MEMBER(Vector3, gFieldBounds)
		B3D_UNIFORM_BUFFER_MEMBER(float, gFieldIntensity)
		B3D_UNIFORM_BUFFER_MEMBER(Vector3, gFieldTiling)
		B3D_UNIFORM_BUFFER_MEMBER(float, gFieldTightness)
		B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gWorldToField)
		B3D_UNIFORM_BUFFER_MEMBER(Matrix3, gFieldToWorld)
	B3D_UNIFORM_BUFFER_END

	inline VectorFieldUniformDefinition gVectorFieldUniformDefinition;

	B3D_UNIFORM_BUFFER_BEGIN(GpuParticleDepthCollisionUniformDefinition)
		B3D_UNIFORM_BUFFER_MEMBER(float, gCollisionRange)
		B3D_UNIFORM_BUFFER_MEMBER(float, gRestitution)
		B3D_UNIFORM_BUFFER_MEMBER(float, gDampening)
		B3D_UNIFORM_BUFFER_MEMBER(float, gCollisionRadiusScale)
		B3D_UNIFORM_BUFFER_MEMBER(Vector2, gSizeScaleCurveOffset)
		B3D_UNIFORM_BUFFER_MEMBER(Vector2, gSizeScaleCurveScale)
	B3D_UNIFORM_BUFFER_END

	inline GpuParticleDepthCollisionUniformDefinition gGpuParticleDepthCollisionUniformDefinition;

	B3D_UNIFORM_BUFFER_BEGIN(GpuParticleSimulateUniformDefinition)
		B3D_UNIFORM_BUFFER_MEMBER(i32, gNumVectorFields)
		B3D_UNIFORM_BUFFER_MEMBER(i32, gNumIterations)
		B3D_UNIFORM_BUFFER_MEMBER(float, gDT)
		B3D_UNIFORM_BUFFER_MEMBER(float, gDrag)
		B3D_UNIFORM_BUFFER_MEMBER(Vector3, gAcceleration)
	B3D_UNIFORM_BUFFER_END

	inline GpuParticleSimulateUniformDefinition gGpuParticleSimulateUniformDefinition;

	/**
	 * Material used for performing GPU particle simulation. State is read from the provided input textures and output
	 * into the output textures bound as render targets.
	 */
	class GpuParticleSimulateMaterial : public RendererMaterial<GpuParticleSimulateMaterial>
	{
		RMAT_DEF_CUSTOMIZED("GpuParticleSimulate.bsl");

		/** Helper method used for initializing variations of this material. */
		template <u32 DEPTH_COLLISIONS>
		static const ShaderVariationParameters& GetVariation()
		{
			static ShaderVariationParameters variation = ShaderVariationParameters(
				{ ShaderVariationParameter("DEPTH_COLLISIONS", DEPTH_COLLISIONS) });

			return variation;
		}

	public:
		GpuParticleSimulateMaterial() = default;

		/**
		 * Populates GPU parameters for rendering using this material.
		 *
		 * @param	gpuParameters				Parameters to populate
		 * @param	resources					GPU particle resources containing textures
		 * @param	particleVertexInputBuffer	Uniform buffer used in the particle vertex shader.
		 * @param	viewParams					Per-camera view parameters
		 * @param	depth						Depth texture for collision detection
		 * @param	normals						Normals texture for collision detection
		 * @param	tileUVs						Sets the UV offsets of individual tiles for a particular particle system that's being rendered.
		 * @param	perObjectParams				General purpose particle system parameters.
		 * @param	vectorFieldTexture			3D texture representing the vector field, or null if none.
		 * @param	supportsDepthCollisions		True if this material variation supports depth collisions
		 */
		static void PopulateParameters(const SPtr<GpuParameterSet>& gpuParameters, GpuParticleResources& resources, const SPtr<GpuBuffer>& particleVertexInputBuffer,
			const SPtr<GpuBuffer>& viewParams, const SPtr<Texture>& depth, const SPtr<Texture>& normals, const SPtr<GpuBuffer>& tileUVs,
			const SPtr<GpuBuffer>& perObjectParams, const SPtr<Texture>& vectorFieldTexture, bool supportsDepthCollisions);

		/** Returns the material variation matching the provided parameters. */
		static GpuParticleSimulateMaterial* GetVariation(bool depthCollisions, bool localSpace);
	};

	B3D_UNIFORM_BUFFER_BEGIN(GpuParticleBoundsUniformDefinition)
		B3D_UNIFORM_BUFFER_MEMBER(u32, gIterationsPerGroup)
		B3D_UNIFORM_BUFFER_MEMBER(u32, gNumExtraIterations)
		B3D_UNIFORM_BUFFER_MEMBER(u32, gNumParticles)
	B3D_UNIFORM_BUFFER_END

	inline GpuParticleBoundsUniformDefinition gGpuParticleBoundsUniformDefinition;

	/** Material used for calculating particle system bounds. */
	class GpuParticleBoundsMaterial : public RendererMaterial<GpuParticleBoundsMaterial>
	{
		static constexpr u32 kNumThreads = 64;

		RMAT_DEF_CUSTOMIZED("GpuParticleBounds.bsl");

	public:
		GpuParticleBoundsMaterial() = default;
		void Initialize() override;

		/** Binds the material to the pipeline along with the global input texture containing particle positions and times. */
		void Bind(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& positionAndTime);

		/**
		 * Executes the material, calculating the bounds. Note that this function reads back from the GPU and should not
		 * be called at runtime.
		 *
		 * @param		commandBuffer	Command buffer to execute on.
		 * @param		indices			Buffer containing offsets into the position texture for each particle.
		 * @param		numParticles	Number of particle in the provided indices buffer.
		 */
		AABox Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& indices, u32 numParticles);

	private:
		GpuParameterUniformBuffer mInputUniformBufferParameter;
		GpuParameterStorageBuffer mParticleIndicesParam;
		GpuParameterStorageBuffer mOutputParam;
		GpuParameterSampledTexture mPosAndTimeTexParam;
	};

	B3D_UNIFORM_BUFFER_BEGIN(GpuParticleSortPrepareUniformDefinition)
		B3D_UNIFORM_BUFFER_MEMBER(i32, gIterationsPerGroup)
		B3D_UNIFORM_BUFFER_MEMBER(i32, gNumExtraIterations)
		B3D_UNIFORM_BUFFER_MEMBER(i32, gNumParticles)
		B3D_UNIFORM_BUFFER_MEMBER(i32, gOutputOffset)
		B3D_UNIFORM_BUFFER_MEMBER(i32, gSystemKey)
		B3D_UNIFORM_BUFFER_MEMBER(Vector3, gLocalViewOrigin)
	B3D_UNIFORM_BUFFER_END

	inline GpuParticleSortPrepareUniformDefinition gGpuParticleSortPrepareUniformDefinition;

	/** Material used for preparing key/values buffers used for particle sorting. */
	class GpuParticleSortPrepareMaterial : public RendererMaterial<GpuParticleSortPrepareMaterial>
	{
		static constexpr u32 kNumThreads = 64;

		RMAT_DEF_CUSTOMIZED("GpuParticleSortPrepare.bsl");

	public:
		GpuParticleSortPrepareMaterial() = default;
		void Initialize() override;

		/** Binds the material to the pipeline along with the global input texture containing particle positions and times. */
		void Bind(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& positionAndTime);

		/**
		 * Executes the material, generating sort data for a particular particle system and injecting it into the specified
		 * location in the key and index buffers.
		 *
		 * @param	commandBuffer	Command buffer to execute on.
		 * @param	system			System whose particles to insert into the sort key/index buffers.
		 * @param	systemIdx		Sequential index of the system to insert into the sort buffers.
		 * @param	offset			Offset into the key/index buffer at which to insert the sort data.
		 * @param	viewOrigin		View origin to use for determining sorting keys, in world space.
		 * @param	outKeys			Pre-allocated buffer that will receive the keys used for sorting. The buffer must
		 *								be GPU writable and use a 1x 32-bit integer format.
		 * @param	outIndices		Pre-allocated buffer that will receive the indices to be sorted. The buffer must
		 *								be GPU writable and use a 2x 16-bit integer format. Must have the same capacity
		 *								as @p outKeys.
		 * @return						Number of particle that were written to the buffers.
		 */
		u32 Execute(GpuCommandBuffer& commandBuffer, const GpuParticleSystem& system, u32 systemIdx, const Vector3& viewOrigin, u32 offset, const SPtr<GpuBuffer>& outKeys, const SPtr<GpuBuffer>& outIndices);

	private:
		GpuParameterUniformBuffer mInputUniformBufferParameter;
		GpuParameterStorageBuffer mInputIndicesParam;
		GpuParameterStorageBuffer mOutputKeysParam;
		GpuParameterStorageBuffer mOutputIndicesParam;
		GpuParameterSampledTexture mPosAndTimeTexParam;
	};

	/** @} */
}}
