//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRenderElement.h"
#include "Components/B3DRenderable.h"
#include "Renderer/B3DGpuDataParameterBlock.h"
#include "Material/B3DMaterialParam.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "B3DRendererReflectionProbe.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(PerObjectParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatWorld)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatInvWorld)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatWorldNoScale)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatInvWorldNoScale)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatPrevWorld)
			B3D_UNIFORM_BUFFER_MEMBER(float, gWorldDeterminantSign)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gLayer)
		B3D_UNIFORM_BUFFER_END

		extern PerObjectParamDef gPerObjectParamDef;

		B3D_UNIFORM_BUFFER_BEGIN(PerCallParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatWorldViewProj)
		B3D_UNIFORM_BUFFER_END

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
			GpuParameterBinding PerCameraBinding;

			/** Collection of parameters used for direct lighting using the forward rendering path. */
			ForwardLightingParams ForwardLightingParams;

			/** Collection of parameters used for image based lighting. */
			ImageBasedLightingParameterBinding ImageBasedParams;

			/** Vertex buffer containing element's morph shape vertices, if it has any. */
			SPtr<GpuBuffer> MorphShapeBuffer;

			/** Vertex declaration used for rendering meshes containing morph shape information. */
			SPtr<VertexDescription> MorphVertexDefinition;

			/** Time to used for evaluating material animation. */
			float MaterialAnimationTime = 0.0f;

			/** Shader parameter binding for the bone matrix buffer. */
			TGpuParameterBuffer<true> BoneMatrixBufferParameter;

			/** Shader parameter binding for the previous frame's bone matrix buffer. */
			TGpuParameterBuffer<true> PreviousBoneMatrixBufferParameter;

			/** Version of the morph shape vertices in the buffer. */
			mutable u32 MorphShapeVersion;

			void Draw(GpuCommandBuffer& commandBuffer) const override;
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

			Renderable* Renderable = nullptr;
			Vector<RenderableElement> Elements;

			SPtr<GpuBuffer> PerObjectParamBuffer;
			SPtr<GpuBuffer> PerCallParamBuffer;
		};

		/** @} */
	} // namespace render
} // namespace b3d
