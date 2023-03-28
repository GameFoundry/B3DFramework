//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRenderElement.h"
#include "Renderer/BsRenderable.h"
#include "Renderer/BsGpuDataParameterBlock.h"
#include "Material/BsMaterialParam.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "BsRendererReflectionProbe.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_PARAM_BLOCK_BEGIN(PerObjectParamDef)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gMatWorld)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gMatInvWorld)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gMatWorldNoScale)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gMatInvWorldNoScale)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gMatPrevWorld)
			B3D_PARAM_BLOCK_ENTRY(float, gWorldDeterminantSign)
			B3D_PARAM_BLOCK_ENTRY(i32, gLayer)
		B3D_PARAM_BLOCK_END

		extern PerObjectParamDef gPerObjectParamDef;

		B3D_PARAM_BLOCK_BEGIN(PerCallParamDef)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gMatWorldViewProj)
		B3D_PARAM_BLOCK_END

		extern PerCallParamDef gPerCallParamDef;

		/** Helper class used for manipulating the PerObject parameter buffer. */
		class PerObjectBuffer
		{
		public:
			/** Updates the provided buffer with the data from the provided matrices. */
			static void Update(SPtr<GpuBuffer>& buffer, const Matrix4& tfrm, const Matrix4& tfrmNoScale, const Matrix4& prevTfrm, u32 layer);
		};

		struct MaterialSamplerOverrides;

		/**
		 * Contains information required for rendering a single Renderable sub-mesh, representing a generic static or animated
		 * 3D model.
		 */
		class RenderableElement final : public RenderElement
		{
		public:
			/**
			 * Optional overrides for material sampler states. Used when renderer wants to override certain sampling properties
			 * on a global scale (for example filtering most commonly).
			 */
			MaterialSamplerOverrides* SamplerOverrides;

			/** Identifier of the animation running on the renderable's mesh. -1 if no animation. */
			u64 AnimationId;

			/** Type of animation applied to this element, if any. */
			RenderableAnimType AnimType;

			/** Binding indices representing where should the per-camera param block buffer be bound to. */
			GpuParamBinding PerCameraBindings[GPT_COUNT];

			/** Collection of parameters used for direct lighting using the forward rendering path. */
			ForwardLightingParams ForwardLightingParams;

			/** Collection of parameters used for image based lighting. */
			ImageBasedLightingParams ImageBasedParams;

			/** GPU buffer containing element's bone matrices, if it requires any. */
			SPtr<GpuBuffer> BoneMatrixBuffer;

			/** GPU buffer containing element's bone matrices for the previous frame, if it requires any. */
			SPtr<GpuBuffer> BonePrevMatrixBuffer;

			/** Vertex buffer containing element's morph shape vertices, if it has any. */
			SPtr<GpuBuffer> MorphShapeBuffer;

			/** Vertex declaration used for rendering meshes containing morph shape information. */
			SPtr<VertexDescription> MorphVertexDefinition;

			/** Time to used for evaluating material animation. */
			float MaterialAnimationTime = 0.0f;

			/** Version of the morph shape vertices in the buffer. */
			mutable u32 MorphShapeVersion;

			void Draw() const override;
		};

		/** Contains information about a Renderable, used by the Renderer. */
		struct RendererRenderable
		{
			RendererRenderable();

			/** Updates the per-object GPU buffer according to the currently set properties. */
			void UpdatePerObjectBuffer();

			/**
			 * Updates the per-call GPU buffer according to the provided parameters.
			 *
			 * @param[in]	viewProj	Combined view-projection matrix of the current camera.
			 * @param[in]	flush		True if the buffer contents should be immediately flushed to the GPU.
			 */
			void UpdatePerCallBuffer(const Matrix4& viewProj, bool flush = true);

			Matrix4 WorldTfrm = Matrix4::kIdentity;
			Matrix4 PrevWorldTfrm = Matrix4::kIdentity;
			PrevFrameDirtyState PrevFrameDirtyState = PrevFrameDirtyState::Clean;

			Renderable* Renderable;
			Vector<RenderableElement> Elements;

			SPtr<GpuBuffer> PerObjectParamBuffer;
			SPtr<GpuBuffer> PerCallParamBuffer;
		};

		/** @} */
	} // namespace ct
} // namespace bs
