//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DDrawCommand.h"
#include "Components/B3DRenderable.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Material/B3DMaterialParam.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "B3DReflectionProbeRenderState.h"
#include "B3DRenderState.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(PerObjectUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatWorld)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatInvWorld)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatWorldNoScale)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatInvWorldNoScale)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatPrevWorld)
			B3D_UNIFORM_BUFFER_MEMBER(float, gWorldDeterminantSign)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gLayer)
		B3D_UNIFORM_BUFFER_END

		extern PerObjectUniformDefinition gPerObjectUniformDefinition;

		struct MaterialSamplerOverrides;

		/** Contains information required for drawing a single Renderable sub-mesh, representing a generic static or animated 3D model. */
		class RenderableDrawCommand final : public DrawCommand
		{
		public:
			/**
			 * Optional overrides for material sampler states. Used when renderer wants to override certain sampling properties
			 * on a global scale (for example filtering most commonly).
			 */
			MaterialSamplerOverrides* SamplerOverrides;

			/**
			 * Skin cache entry whose deformed vertices the element draws, instead of the mesh's, or null. Owned by the renderable's proxy,
			 * which outlives the element.
			 */
			SkinCacheEntry* SkinCacheEntry = nullptr;

			/** Time to used for evaluating material animation. */
			float MaterialAnimationTime = 0.0f;

			void Draw(GpuCommandBuffer& commandBuffer) const override;
		};

		/** Renderer-specific state for a renderable. */
		struct RenderableRenderState : RenderState
		{
			/** Updates the per-object data from the provided renderable proxy. */
			void UpdatePerObjectData(const RenderableProxy& proxy);

			Vector<RenderableDrawCommand> DrawCommands;
		};

		/** @} */
	} // namespace render
} // namespace b3d
