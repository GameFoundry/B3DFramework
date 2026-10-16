//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"

namespace b3d
{
	namespace render
	{
		class VisibleReflectionProbeData;
		class VisibleLightData;
		class FrameGraph;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(LightGridUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4I, gLightCounts)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gLightStrides)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gNumReflProbes)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gNumCells)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3I, gGridSize)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gMaxNumLightsPerCell)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gGridPixelSize)
		B3D_UNIFORM_BUFFER_END

		extern LightGridUniformDefinition gLightGridUniformDefinition;

		/** Buffers of the light grid of a view, which AddLightGridPasses() creates in a frame graph. */
		struct LightGridOutputs
		{
			/** Uniform buffer using LightGridUniformDefinition. */
			GpuBufferSuballocation UniformBuffer;

			/**
			 * Flattened array of grid cells, where each entry contains the number of lights affecting that cell, and a index
			 * into the @p gridLightIndices buffer.
			 */
			TShared<GpuBuffer> GridLightOffsetsAndSize;

			/**
			 * A list of light indices. Each cell's indices start at specific position and are placed sequentially one after
			 * another. Lookup into this array is done through offset & size provided by @p gridLightOffsetsAndSize.
			 */
			TShared<GpuBuffer> GridLightIndices;

			/**
			 * Flattened array of grid cells, where each entry contains the number of reflection probes affecting that cell,
			 * and a index into the @p gridProbeIndices buffer.
			 */
			TShared<GpuBuffer> GridProbeOffsetsAndSize;

			/**
			 * A list of reflection probe indices. Each cell's indices start at specific position and are placed sequentially
			 * one after another. Lookup into this array is done through offset & size provided by @p gridProbeOffsetsAndSize.
			 */
			TShared<GpuBuffer> GridProbeIndices;
		};

		/** Linked lists that LightGridLLCreationMaterial builds and LightGridLLReductionMaterial reduces. */
		struct LightGridLinkedLists
		{
			TShared<GpuBuffer> LightsCounter; /**< Number of light list entries allocated. */
			TShared<GpuBuffer> LightsHeads; /**< Index of the first light list entry of each cell. */
			TShared<GpuBuffer> Lights; /**< Light list entries. */

			TShared<GpuBuffer> ProbesCounter; /**< Number of reflection probe list entries allocated. */
			TShared<GpuBuffer> ProbesHeads; /**< Index of the first reflection probe list entry of each cell. */
			TShared<GpuBuffer> Probes; /**< Reflection probe list entries. */
		};

		/**
		 * Shader that creates a linked list for each light grid cell, containing which lights and reflection probes affects
		 * each cell.
		 */
		class LightGridLLCreationMaterial : public RendererMaterial<LightGridLLCreationMaterial>
		{
			RMAT_DEF_CUSTOMIZED("LightGridLLCreation.bsl");

		public:
			LightGridLLCreationMaterial() = default;
			void Initialize() override;

			/**
			 * Creates the parameter set of @p pass and assigns the inputs and outputs to it.
			 *
			 * @param	pass				Pass that dispatches the material.
			 * @param	view				View whose grid to build.
			 * @param	gridUniformBuffer	Uniform buffer using LightGridUniformDefinition.
			 * @param	lightsBuffer		Lights visible in the view.
			 * @param	probesBuffer		Reflection probes visible in the view.
			 * @param	linkedLists			Linked lists to write, with cleared counters and heads.
			 * @return						Parameter set to pass to Execute().
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const RendererView& view, const GpuBufferSuballocation& gridUniformBuffer,
				const TShared<GpuBuffer>& lightsBuffer, const TShared<GpuBuffer>& probesBuffer, const LightGridLinkedLists& linkedLists);

			/** Binds the material and @p set, a set from Prepare(), and dispatches over a grid of @p gridSize cells. */
			void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const Vector3I& gridSize) const;

		private:
			GpuParameterUniformBuffer mGridParamsParameter;
			GpuParameterUniformBuffer mPerCameraParameter;

			GpuParameterStorageBuffer mLightBufferParam;
			GpuParameterStorageBuffer mLightsCounterParam;
			GpuParameterStorageBuffer mLightsLLHeadsParam;
			GpuParameterStorageBuffer mLightsLLParam;

			GpuParameterStorageBuffer mProbesBufferParam;
			GpuParameterStorageBuffer mProbesCounterParam;
			GpuParameterStorageBuffer mProbesLLHeadsParam;
			GpuParameterStorageBuffer mProbesLLParam;
		};

		/** Shader that reduces the linked list created by LightGridLLCreationMat into a sequential array. */
		class LightGridLLReductionMaterial : public RendererMaterial<LightGridLLReductionMaterial>
		{
			RMAT_DEF_CUSTOMIZED("LightGridLLReduction.bsl");

		public:
			LightGridLLReductionMaterial() = default;
			void Initialize() override;

			/**
			 * Creates the parameter set of @p pass and assigns the inputs and outputs to it.
			 *
			 * @param	pass				Pass that dispatches the material.
			 * @param	view				View whose grid to build.
			 * @param	linkedLists			Linked lists that LightGridLLCreationMaterial wrote.
			 * @param	gridDataCounter		Counter of the entries of the output index lists, cleared.
			 * @param	outputs				Buffers to write. Their uniform buffer is the one of the grid.
			 * @return						Parameter set to pass to Execute().
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const RendererView& view, const LightGridLinkedLists& linkedLists,
				const TShared<GpuBuffer>& gridDataCounter, const LightGridOutputs& outputs);

			/** Binds the material and @p set, a set from Prepare(), and dispatches over a grid of @p gridSize cells. */
			void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const Vector3I& gridSize) const;

		private:
			GpuParameterUniformBuffer mGridParamsParameter;
			GpuParameterUniformBuffer mPerCameraParameter;

			GpuParameterStorageBuffer mLightsLLHeadsParam;
			GpuParameterStorageBuffer mLightsLLParam;

			GpuParameterStorageBuffer mProbesLLHeadsParam;
			GpuParameterStorageBuffer mProbesLLParam;

			GpuParameterStorageBuffer mGridDataCounterParam;

			GpuParameterStorageBuffer mGridLightOffsetAndSizeParam;
			GpuParameterStorageBuffer mGridLightIndicesParam;

			GpuParameterStorageBuffer mGridProbeOffsetAndSizeParam;
			GpuParameterStorageBuffer mGridProbeIndicesParam;
		};

		/**
		 * Adds the passes that build the light grid of @p view to @p graph: a grid in view space whose cells list the lights and
		 * reflection probes affecting them, for forward rendering.
		 *
		 * @param	graph		Graph to add the passes to.
		 * @param	view		View whose grid to build.
		 * @param	lightData	Lights visible in the view.
		 * @param	probeData	Reflection probes visible in the view.
		 * @param	noLighting	If true, the grid lists no lights.
		 * @return				Transient buffers of the grid, and its uniform buffer.
		 */
		LightGridOutputs AddLightGridPasses(FrameGraph& graph, const RendererView& view, const VisibleLightData& lightData, const VisibleReflectionProbeData& probeData,
			bool noLighting);

		/** @} */
	} // namespace render
} // namespace b3d
