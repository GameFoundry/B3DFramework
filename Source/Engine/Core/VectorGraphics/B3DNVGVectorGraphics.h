//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "VectorGraphics/B3DVectorGraphics.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Renderer/B3DRendererMaterial.h"
#include <ThirdParty/nanovg.h>

namespace b3d
{
	class NVGVectorPathRenderableRTTI;

	/** @addtogroup VectorGraphics-Internal
	 *  @{
	 */

	/** Represents a single vertex used for drawing NanoVG path elements. */
	struct NVGVertex
	{
		NVGVertex() = default;
		NVGVertex(Vector2 position, Vector2 uv)
			: Position(position), UV(uv)
		{ }

		Vector2 Position;
		Vector2 UV;
	};

	/** Represents uniform parameters required for drawing a NanoVG path element. */
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
		Vector2 Padding; // Making the struct a multiple of 16 bytes
	};

	/** Type of command that determines how is a path element rendered. */
	enum class NVGRenderCommandType
	{
		Fill, /**< Drawing a non-convex fill shape. */
		ConvexFill, /**< Drawing a convex fill shape. */
		Stroke /**< Drawing a stroke. */
	};

	/** Contains information about drawing a single NanoVG path element. */
	struct NVGRenderCommand
	{
		NVGRenderCommandType Type; /**< Determines how to render the element. */
		VectorGraphicsBlendMode BlendMode; /**< Blend mode to use when rendering. */
		NVGRenderUniforms PrimaryPassUniforms; /**< Uniform buffer parameters to use when rendering. */
		TOptional<NVGRenderUniforms> SecondaryPassUniforms; /**< In case the element is drawn using multiple passes, uniform parameters for the second pass. */
	};

	/** Determines the shader variation to use for rendering a single pass of a path element. */
	enum class NVGDrawMode
	{
		FillShapeStencil, /**< Marks the area in which non-convex fill will be drawn. */
		FillAA, /**< Draws the anti-aliased fill border (if antialiasing is enabled). */
		FillDraw, /**< Draws the fill in area marked by FillShapeStencil, and clears the stencil buffer. */
		StrokeStencil, /**< Marks the stencil area in which to draw antialised stroke. */
		StrokeAA, /**< Draws antialiased stroke in area marked by StrokeAA. */
		ClearStencil, /**< Clears stencil to zero. */
		FillSimple, /**< Simple fill shader used for convex shapes and non-antialiased strokes. */
	};

	namespace render
	{
		B3D_UNIFORM_BUFFER_BEGIN(VectorGraphicsRenderUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gScissorMatrix)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gPaintMatrix)
			B3D_UNIFORM_BUFFER_MEMBER(Color, gInnerColor)
			B3D_UNIFORM_BUFFER_MEMBER(Color, gOuterColor)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gScissorExtents)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gScissorScale)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gExtent)
			B3D_UNIFORM_BUFFER_MEMBER(float, gRadius)
			B3D_UNIFORM_BUFFER_MEMBER(float, gFeather)
			B3D_UNIFORM_BUFFER_MEMBER(float, gStrokeMultiplier)
			B3D_UNIFORM_BUFFER_MEMBER(float, gStrokeThreshold)
		B3D_UNIFORM_BUFFER_END

		extern VectorGraphicsRenderUniformDefinition gVectorGraphicsRenderUniforms;

		B3D_UNIFORM_BUFFER_BEGIN(VectorGraphicsViewUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gViewportOffset)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gInverseViewportHalfSize)
			B3D_UNIFORM_BUFFER_MEMBER(float, gViewportYFlip)
		B3D_UNIFORM_BUFFER_END

		extern VectorGraphicsViewUniformDefinition gVectorGraphicsViewUniforms;

		/** Material used for drawing NanoVG path elements. */
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

		/** Mesh data and render commands required for rendering a NanoVG path. */
		struct NVGPathRenderData
		{
			Vector<NVGVertex> Vertices;
			Vector<u32> Indices;
			Vector<SubMesh> Submeshes;
			Vector<NVGRenderCommand> RenderCommands;
		};

		/** VectorPathRenderable implementation that uses NanoVG for rasterizing a VectorPath. */
		class NVGVectorPathRenderable : public VectorPathRenderable
		{
		public:
			NVGVectorPathRenderable(const b3d::VectorPath& vectorPath, const VectorGraphicsSettings& settings);

			SPtr<GpuParameterSet> Prepare() override;
			void Render(GpuCommandBuffer& commandBuffer) override;

		private:
			/** GPU buffers holding the vertex, index and uniform parameter information. */
			struct RenderGpuBuffers
			{
				SPtr<VertexDescription> VertexDescription;
				SPtr<GpuBuffer> VertexBuffer;
				SPtr<GpuBuffer> IndexBuffer;
				SPtr<GpuBuffer> ViewUniformBuffer;
				SPtr<GpuBuffer> RenderUniformBuffer;
				SPtr<GpuParameterSet> GpuParameterSet;
			};

			/** Context that will be filled by NanoVG callbacks when executing NanoVG path commands. */
			struct NVGRenderContext
			{
				VectorGraphicsSettings Settings;
				NVGPathRenderData OutputRenderData;
			};

			/** Converts the raw render data from mRawRenderData into GPU buffers that can be fed to the vector graphics rendering material. */
			RenderGpuBuffers CookRenderBuffers();

			/** Plays back the commands in @p vectorPath and returns the mesh data and render commands that can be executed for rendering the path. */
			static NVGPathRenderData PlaybackPathCommands(const b3d::VectorPath& vectorPath, const VectorGraphicsSettings& settings);

			/** Callback called by NanoVG when a fill command is executed. This will record the provided vertices and indices into the render context, as well as render commands for rendering that mesh data. */
			static void NVGRenderFillCallback(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, const float* bounds, const NVGpath* paths, int npaths);

			/** Callback called by NanoVG when a stroke command is executed. This will record the provided vertices and indices into the render context, as well as render commands for rendering that mesh data. */
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
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};
	}

	/** @} */

} // namespace b3d
