//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include <VectorGraphics/BsVectorGraphics.h>
#include "Renderer/BsGpuDataParameterBlock.h"
#include "Renderer/BsRendererMaterial.h"
#include <ThirdParty/nanovg.h>

namespace bs
{
	class NVGVectorPathRenderableRTTI;

	/** @addtogroup VectorGraphics-Internal
	 *  @{
	 */

	struct NVGVertex
	{
		NVGVertex() = default;
		NVGVertex(Vector2 position, Vector2 uv)
			: Position(position), UV(uv)
		{ }

		Vector2 Position;
		Vector2 UV;
	};

	struct NVGRenderUniforms
	{
		Matrix4 ScissorMatrix;
		Matrix4 PaintMatrix;
		Color InnerColor;
		Color OuterColor;
		Vector2 ScissorExtents;
		Vector2 ScissorScale;
		Vector2 Extent;
		float Radius;
		float Feather;
		float StrokeMultiplier;
		float StrokeThreshold;
	};

	enum class NVGRenderCommandType
	{
		Fill,
		ConvexFill,
		Stroke
	};

	struct NVGRenderCommand
	{
		NVGRenderCommandType Type;
		u32 PathCount;
		VectorGraphicsBlendMode BlendMode;
		NVGRenderUniforms PrimaryPassUniforms;
		Optional<NVGRenderUniforms> SecondaryPassUniforms;
	};

	enum class NVGDrawMode
	{
		FillShapeStencil,
		FillAA,
		FillDraw,
		StrokeStencil,
		StrokeAA,
		ClearStencil,
		FillSimple,
	};

	namespace ct
	{
		B3D_PARAM_BLOCK_BEGIN(VectorGraphicsRenderUniformDefinition)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gScissorMatrix)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gPaintMatrix)
			B3D_PARAM_BLOCK_ENTRY(Color, gInnerColor)
			B3D_PARAM_BLOCK_ENTRY(Color, gOuterColor)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gScissorExtents)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gScissorScale)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gExtent)
			B3D_PARAM_BLOCK_ENTRY(float, gRadius)
			B3D_PARAM_BLOCK_ENTRY(float, gFeather)
			B3D_PARAM_BLOCK_ENTRY(float, gStrokeMultiplier)
			B3D_PARAM_BLOCK_ENTRY(float, gStrokeThreshold)
		B3D_PARAM_BLOCK_END

		extern VectorGraphicsRenderUniformDefinition gVectorGraphicsRenderUniforms;

		B3D_PARAM_BLOCK_BEGIN(VectorGraphicsViewUniformDefinition)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gViewportOffset)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gInverseViewportHalfSize)
			B3D_PARAM_BLOCK_ENTRY(float, gViewportYFlip)
		B3D_PARAM_BLOCK_END

		extern VectorGraphicsViewUniformDefinition gVectorGraphicsViewUniforms;

		class VectorGraphicsMaterial : public RendererMaterial<VectorGraphicsMaterial>
		{
			RMAT_DEF("VectorGraphics.bsl");

			/** Helper method used for initializing variations of this material. */
			template <NVGDrawMode DrawMode, VectorGraphicsBlendMode BlendMode, bool Antialiasing>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
				{
						ShaderVariationParameter("DRAW_MODE", (u32)DrawMode),
						ShaderVariationParameter("BLEND_MODE", (u32)BlendMode),
						ShaderVariationParameter("EDGE_AA", Antialiasing),
					});

				return variation;
			}

		public:
			VectorGraphicsMaterial() = default;

			static VectorGraphicsMaterial* GetVariation(NVGDrawMode drawMode, VectorGraphicsBlendMode blendMode, bool antialiasing);
		};

		struct NVGPathRenderData
		{
			Vector<NVGVertex> Vertices;
			Vector<u32> Indices;
			Vector<SubMesh> Submeshes;
			Vector<NVGRenderCommand> RenderCommands;
		};

		class NVGVectorPathRenderable : public VectorPathRenderable
		{
		public:
			NVGVectorPathRenderable(const VectorPath& vectorPath, const VectorGraphicsSettings& settings);

			void Render(GpuCommandBuffer& commandBuffer) override;

		private:
			struct RenderGpuBuffers
			{
				SPtr<GpuBuffer> VertexBuffer;
				SPtr<GpuBuffer> IndexBuffer;
				SPtr<GpuBuffer> ViewUniformBuffer;
				SPtr<GpuBuffer> RenderUniformBuffer;
			};

			struct NVGRenderContext
			{
				VectorGraphicsSettings Settings;
				NVGPathRenderData OutputRenderData;
			};

			RenderGpuBuffers CookRenderBuffers();
			static NVGPathRenderData PlaybackPathCommands(const VectorPath& vectorPath, const VectorGraphicsSettings& settings);

			static void NVGRenderFillCallback(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, const float* bounds, const NVGpath* paths, int npaths);
			static void NVGRenderStrokeCallback(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, float strokeWidth, const NVGpath* paths, int npaths);

			NVGPathRenderData mRawRenderData;

			RenderGpuBuffers mRenderBuffers;
			bool mRenderBuffersCooked = false;

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			NVGVectorPathRenderable() = default; // Deserialization only

			friend class NVGVectorPathRenderableRTTI;
			static RTTITypeBase* GetRttiStatic();
			RTTITypeBase* GetRtti() const override;
		};
	}

	/** @} */

} // namespace bs
