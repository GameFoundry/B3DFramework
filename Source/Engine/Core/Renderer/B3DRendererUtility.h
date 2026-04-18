//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Math/B3DArea2.h"
#include "Math/B3DArea2.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Mesh/B3DMeshBase.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Renderer
		 *  @{
		 */

		/** Shader that copies a source texture into a render target, and optionally resolves it. */
		class B3D_EXPORT BlitMat : public RendererMaterial<BlitMat>
		{
			RMAT_DEF("Blit.bsl");

			/** Helper method used for initializing variations of this material. */
			template <u32 MSAA, u32 MODE, u32 BLEND, u32 WRITE_ALPHA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					TInlineArray<ShaderVariationParameter, 4>({
						ShaderVariationParameter("MSAA_COUNT", MSAA),
						ShaderVariationParameter("MODE", MODE),
						ShaderVariationParameter("BLEND", BLEND),
						ShaderVariationParameter("WRITE_ALPHA", WRITE_ALPHA),
					}));

				return variation;
			}

		public:
			BlitMat() = default;
			void Initialize() override;

			/** Creates a new GpuParameterSet and assigns the provided texture. */
			SPtr<GpuParameterSet> Prepare(const SPtr<Texture>& source);

			/**
			 * Executes the blit operation using pre-configured GPU parameters.
			 *
			 * @param commandBuffer     Command buffer to record draw commands into. A render pass must already be active.
			 * @param gpuParameters     GPU parameters containing the configured source texture. Should be obtained via Prepare().
			 * @param area              Area to use for UV coordinates when drawing the fullscreen quad. For unfiltered blits,
			 *                          this controls the sampled region. For filtered blits, normalized (0,1) UVs are used.
			 * @param flipUV            If true, vertical texture coordinates are flipped.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuParameterSet>& gpuParameters, const Area2& area, bool flipUV);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	msaaCount		Number of MSAA samples in the input texture. If larger than 1 the texture will be resolved
			 *							before written to the destination.
			 * @param	isColor			If true the input is assumed to be a 4-component color texture. If false it is assumed
			 *							the input is a 1-component depth texture. This controls how is the texture resolve and is
			 *							only relevant if @p msaaCount > 1. Color texture MSAA samples will be averaged, while for
			 *							depth textures the minimum of all samples will be used.
			 * @param	isFiltered		True if to apply bilinear filtering to the sampled texture. Only relevant for color
			 *							textures with no multiple samples.
			 * @param	blend			If true blit source will be blended with the target image, rather than overwriting it, using
			 *							the alpha value from the source.
			 * @param	writeAlpha		If true, alpha value from the source will be passed to the destination. Only relevant when
			 *							@p blend in enabled.
			 */
			static BlitMat* GetVariation(u32 msaaCount, bool isColor, bool isFiltered, bool blend = false, bool writeAlpha = false);

		private:
			bool mIsFiltered = false;
		};

		B3D_UNIFORM_BUFFER_BEGIN(CompositeUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Color, gTint)
		B3D_UNIFORM_BUFFER_END

		extern CompositeUniformDefinition gCompositeUniformDefinition;

		/** Blends the contents of the provided texture with the bound render target. */
		class B3D_EXPORT CompositeMaterial : public RendererMaterial<CompositeMaterial>
		{
			RMAT_DEF("Composite.bsl");

		public:
			CompositeMaterial() = default;
			void Initialize() override;

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the provided
			 * render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	source			Texture to blend with the target.
			 * @param	target			Render target to blend with and write the results to.
			 * @param	tint			Optional value to multiply all the values from @p source before blending.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, const SPtr<RenderTarget>& target, const Color& tint = Color::kWhite);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mSourceTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(BicubicUpsampleUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Color, gTint)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gTextureSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gInvPixel)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gInvTwoPixels)
		B3D_UNIFORM_BUFFER_END

		extern BicubicUpsampleUniformDefinition gBicubicUpsampleUniformDefinition;

		/** Samples the source texture using bicubic filtering and outputs the results to the provided render target. */
		class B3D_EXPORT BicubicUpsampleMaterial : public RendererMaterial<BicubicUpsampleMaterial>
		{
			RMAT_DEF("BicubicUpsample.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool HERMITE>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					TInlineArray<ShaderVariationParameter, 4>({ ShaderVariationParameter("HERMITE", HERMITE) }));

				return variation;
			}

		public:
			BicubicUpsampleMaterial() = default;
			void Initialize() override;

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the provided
			 * render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	source			Texture to filter.
			 * @param	target			Render target to write the results to. Results will be additively blended
			 *								with the target.
			 * @param	tint			Optional value to multiply all the values from @p source before blending.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, const SPtr<RenderTarget>& target, const Color& tint = Color::kWhite);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	hermite		If true, use Hermite cubic filtering, otherwise use Lagrange cubic filtering.
			 */
			static BicubicUpsampleMaterial* GetVariation(bool hermite = false);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mSourceTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(ClearUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gClearValue)
		B3D_UNIFORM_BUFFER_END

		extern ClearUniformDefinition gClearUniformDefinition;

		/** Shader that clears the currently bound render target to an integer value. */
		class B3D_EXPORT ClearMaterial : public RendererMaterial<ClearMaterial>
		{
			RMAT_DEF("Clear.bsl");

		public:
			ClearMaterial() = default;
			void Initialize() override;

			/** Executes the material on the currently bound render target, clearing to to @p value. */
			void Execute(GpuCommandBuffer& commandBuffer, u32 value);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
		};

		/** Information describing a blit operation from a source texture to a render target. */
		struct BlitInformation
		{
			/**
			 * Source texture to blit from. This texture will be sampled and copied to the output render target.
			 * The texture can be either a color or depth texture, determined by the @p IsDepth parameter.
			 */
			SPtr<Texture> InputTexture;

			/**
			 * Area of the source texture to blit from, in pixel coordinates. If set to Area2I::kEmpty (default),
			 * the entire source texture will be used. The area is used to control which portion of the source
			 * texture is sampled when useFiltering is false. When useFiltering is true, the entire texture
			 * is always sampled with normalized UV coordinates.
			 */
			Area2I InputArea = Area2I::kEmpty;

			/**
			 * Destination render target to blit to. The blit operation will render into this target's
			 * color and/or depth surfaces depending on the isDepth parameter.
			 */
			SPtr<RenderTarget> OutputRenderTarget;

			/**
			 * Optional viewport area for the output render target, in normalized coordinates [0,1].
			 * If not specified, the current viewport remains unchanged. If specified, the viewport
			 * will be set to this area before rendering, allowing the blit to target a specific
			 * region of the output render target.
			 */
			TOptional<Area2> OutputArea;

			/**
			 * Determines which render surfaces should not be loaded from memory at the start of the render pass.
			 * This is a performance optimization - surfaces that will be completely overwritten don't need to be
			 * loaded. Use RenderSurfaceMask flags (RT_NONE, RT_COLOR0, RT_DEPTH, etc.) to specify surfaces.
			 * Default is RT_NONE (no surfaces are loaded).
			 *
			 * @see RenderSurfaceMask for available flags
			 * @see RenderPassCreateInformation::LoadMask for more details
			 */
			RenderSurfaceMask LoadMask = RT_NONE;

			/**
			 * Determines which render surfaces should be treated as read-only during the render pass.
			 * Read-only surfaces cannot be written to and may enable additional optimizations (e.g., keeping
			 * depth buffer in compressed state). Use RenderSurfaceMask flags (RT_NONE, RT_COLOR0, RT_DEPTH,
			 * RT_ALL, etc.) to specify surfaces.
			 * Default is RT_NONE (all surfaces are writable).
			 *
			 * @see RenderSurfaceMask for available flags
			 * @see RenderPassCreateInformation::ReadOnlyMask for more details
			 */
			RenderSurfaceMask ReadOnlyMask = RT_NONE;

			/** Determines which render surfaces to clear. */
			RenderSurfaceMask ClearMask = RT_NONE;

			/**
			 * If true, the vertical texture coordinates will be flipped during the blit. This is useful
			 * when blitting between render targets with different coordinate systems (e.g., OpenGL vs DirectX).
			 */
			bool FlipUV = false;

			/**
			 * If true, the source texture is treated as a depth texture and the blit will write to the
			 * output's depth surface. If false, the source is treated as a color texture and writes to
			 * the color surface. Depth blits use point sampling and write depth values directly.
			 */
			bool IsDepth = false;

			/**
			 * If true, bilinear filtering is used when sampling the source texture. This produces smoother
			 * results when scaling but may introduce blur. If false, point sampling is used which preserves
			 * sharp edges but may show aliasing when scaling.
			 * Note: Ignored when @p IsDepth is true (depth always uses point sampling).
			 */
			bool UseFiltering = false;

			/**
			 * If true, the blit will blend with the existing contents of the output render target using
			 * standard alpha blending (source alpha, inverse source alpha, add). If false, the source
			 * texture completely replaces the destination. Only relevant if @p IsDepth is false.
			 */
			bool UseBlend = false;

			/**
			 * Controls whether the alpha channel is written during blending operations. Only relevant
			 * when @p UseBlend is true. If false, only RGB channels are written and alpha is preserved.
			 * If true, all RGBA channels are written.
			 */
			bool WriteAlpha = false;

			/** Helper to create blit information with commonly used settings for copying a color texture (no blending, no filtering, no UV flip). */
			static BlitInformation BlitColor(const SPtr<Texture>& inputTexture, const SPtr<RenderTarget>& outputRenderTarget, const Area2I& inputArea = Area2I::kEmpty, RenderSurfaceMask readOnlyMask = RT_NONE, RenderSurfaceMask loadMask = RT_NONE)
			{
				BlitInformation blitInformation;
				blitInformation.InputTexture = inputTexture;
				blitInformation.OutputRenderTarget = outputRenderTarget;
				blitInformation.InputArea = inputArea;
				blitInformation.LoadMask = loadMask;
				blitInformation.ReadOnlyMask = readOnlyMask;

				return blitInformation;
			}

			/** Helper to create blit information with commonly used settings for copying a depth texture (no blending, no filtering, no UV flip). */
			static BlitInformation BlitDepth(const SPtr<Texture>& inputTexture, const SPtr<RenderTarget>& outputRenderTarget, const Area2I& inputArea = Area2I::kEmpty, RenderSurfaceMask readOnlyMask = RT_NONE, RenderSurfaceMask loadMask = RT_NONE)
			{
				BlitInformation blitInformation = BlitColor(inputTexture, outputRenderTarget, inputArea, readOnlyMask, loadMask);
				blitInformation.IsDepth = true;

				return blitInformation;
			}

			/** Helper to create blit information with commonly used settings for blending a color texture with the currently bound render target (no filtering, no UV flip). */
			static BlitInformation Blend(const SPtr<Texture>& inputTexture, const SPtr<RenderTarget>& outputRenderTarget, const Area2I& inputArea = Area2I::kEmpty, RenderSurfaceMask readOnlyMask = RT_NONE, RenderSurfaceMask loadMask = RT_NONE)
			{
				BlitInformation blitInformation = BlitColor(inputTexture, outputRenderTarget, inputArea, readOnlyMask, loadMask);
				blitInformation.UseBlend = true;

				return blitInformation;
			}
		};

		/**
		 * Contains various utility methods that make various common operations in the renderer easier.
		 *
		 * @note	Render thread only.
		 */
		class B3D_EXPORT RendererUtility : public Module<RendererUtility>
		{
		public:
			RendererUtility();
			~RendererUtility() = default;

			/**
			 * Activates the specified material pass for rendering. Any further draw calls will be executed using this pass.
			 *
			 * @param	commandBuffer	Command buffer to bind the material pass to.
			 * @param	material		Material containing the pass.
			 * @param	passIndex		Index of the pass in the material.
			 * @param	variationIndex	Index of the variation the pass belongs to, if the material has multiple variations.
			 *
			 * @note	Render thread.
			 */
			void SetPass(GpuCommandBuffer& commandBuffer, const SPtr<Material>& material, u32 passIndex = 0, u32 variationIndex = 0);

			/**
			 * Activates the specified material pass for compute. Any further dispatch calls will be executed using this pass.
			 *
			 * @param	commandBuffer	Command buffer to bind the pass to.
			 * @param	material		Material containing the pass.
			 * @param	passIndex		Index of the pass in the material.
			 *
			 * @note	Render thread.
			 */
			void SetComputePass(GpuCommandBuffer& commandBuffer, const SPtr<Material>& material, u32 passIndex = 0);

			/**
			 * Sets parameters (textures, samplers, buffers) for the currently active pass.
			 *
			 * @param	commandBuffer	Command buffer to bind the pass parameters to.
			 * @param	adapter			Object containing the parameters.
			 * @param	passIndex		Pass for which to set the parameters.
			 *
			 * @note	Render thread.
			 */
			void SetPassParams(GpuCommandBuffer& commandBuffer, const SPtr<MaterialParameterAdapter>& adapter, u32 passIndex = 0);

			/**
			 * Draws the specified mesh.
			 *
			 * @param	commandBuffer	Command buffer to encode the draw command on.
			 * @param	mesh			Mesh to draw.
			 * @param	instanceCount	Number of times to draw the mesh using instanced rendering.
			 *
			 * @note	Render thread.
			 */
			void Draw(GpuCommandBuffer& commandBuffer, const SPtr<MeshBase>& mesh, u32 instanceCount = 1);

			/**
			 * Draws the specified mesh.
			 *
			 * @param	commandBuffer	Command buffer to encode the draw command on.
			 * @param	mesh			Mesh to draw.
			 * @param	subMesh			Portion of the mesh to draw.
			 * @param	instanceCount	Number of times to draw the mesh using instanced rendering.
			 *
			 * @note	Render thread.
			 */
			void Draw(GpuCommandBuffer& commandBuffer, const SPtr<MeshBase>& mesh, const SubMesh& subMesh, u32 instanceCount = 1);

			/**
			 * Draws the specified mesh with an additional vertex buffer containing morph shape vertices.
			 *
			 * @param	commandBuffer			Command buffer to encode the draw command on.
			 * @param	mesh					Mesh to draw.
			 * @param	subMesh					Portion of the mesh to draw.
			 * @param	morphVertices			Buffer containing the morph shape vertices. Will be bound to stream 1.
			 *									Expected to contain the same number of vertices as the source mesh.
			 * @param	morphVertexDescription	Object describing vertices of the provided mesh and the vertices
			 *									provided in the morph vertex buffer.
			 *
			 * @note	Render thread.
			 */
			void DrawMorph(GpuCommandBuffer& commandBuffer, const SPtr<MeshBase>& mesh, const SubMesh& subMesh, const SPtr<GpuBuffer>& morphVertices, const SPtr<VertexDescription>& morphVertexDescription);

			/**
			 * Blits a source texture to a render target with optional filtering, blending, and coordinate transformations.
			 * This method encapsulates the complete blit operation including render pass management and viewport setup.
			 *
			 * @param commandBuffer     Command buffer to record the blit operation into.
			 * @param blitInformation   Structure containing all blit parameters (source/destination, areas, options).
			 *
			 * @note This method manages the render pass lifecycle - callers should NOT call BeginRenderPass/EndRenderPass.
			 * @note The viewport is only modified if blitInfo.OutputArea is specified; otherwise, the current viewport is preserved.
			 */
			void Blit(GpuCommandBuffer& commandBuffer, const BlitInformation& blitInformation);

			/**
			 * Draws a quad over the entire viewport in normalized device coordinates.
			 *
			 * @param	commandBuffer	Command buffer to encode the draw command on.
			 * @param	uv				UV coordinates to assign to the corners of the quad.
			 * @param	textureSize		Size of the texture the UV coordinates are specified for. If the UV coordinates are
			 *							already in normalized (0, 1) range then keep this value as is. If the UV coordinates
			 *							are in texels then set this value to the texture size so they can be normalized
			 *							internally.
			 * @param	numInstances	How many instances of the quad to draw (using instanced rendering). Useful when
			 *							drawing to 3D textures.
			 * @param	flipUV			If true, vertical UV coordinate will be flipped upside down.
			 *
			 * @note	Render thread.
			 */
			void DrawScreenQuad(GpuCommandBuffer& commandBuffer, const Area2& uv, const Vector2I& textureSize = Vector2I(1, 1), u32 numInstances = 1, bool flipUV = false);

			/**
			 * Draws a quad over the entire viewport in normalized device coordinates.
			 *
			 * @param	commandBuffer	Command buffer to encode the draw command on.
			 * @param	numInstances	How many instances of the quad to draw (using instanced rendering). Useful when
			 *							drawing to 3D textures.
			 *
			 * @note	Render thread.
			 */
			void DrawScreenQuad(GpuCommandBuffer& commandBuffer, u32 numInstances = 1)
			{
				Area2 uv(0.0f, 0.0f, 1.0f, 1.0f);
				Vector2I textureSize(1, 1);

				DrawScreenQuad(commandBuffer, uv, textureSize, numInstances);
			}

			/**
			 * Clears the currently bound render target to the provided integer value. This is similar to
			 * RenderAPI::clearRenderTarget(), except it supports integer clears.
			 */
			void Clear(GpuCommandBuffer& commandBuffer, u32 value);

			/** Returns a unit sphere stencil mesh. */
			SPtr<Mesh> GetSphereStencil() const { return mUnitSphereStencilMesh; }

			/** Returns a unit axis aligned box stencil mesh. */
			SPtr<Mesh> GetBoxStencil() const { return mUnitBoxStencilMesh; }

			/** Number of sides in the cone mesh used for spot light stencil volumes. */
			static constexpr u32 kSpotLightStencilSideCount = 20;

			/** Number of slices in the cone mesh used for spot light stencil volumes. */
			static constexpr u32 kSpotLightStencilSliceCount = 10;

			/**
			 * Returns a stencil mesh used for a spot light. Actual vertex positions need to be computed in shader as this
			 * method will return uninitialized vertex positions.
			 */
			SPtr<Mesh> GetSpotLightStencil() const { return mSpotLightStencilMesh; }

			/** Returns a mesh that can be used for rendering a skybox. */
			SPtr<Mesh> GetSkyBoxMesh() const { return mSkyBoxMesh; }

		private:
			static constexpr u32 kNumQuadVbSlots = 1024;

			SPtr<GpuBuffer> mFullScreenQuadIB;
			SPtr<GpuBuffer> mFullScreenQuadVB;
			SPtr<VertexDescription> mFullscreenQuadVertexDescription;
			u32 mNextQuadVBSlot = 0;

			SPtr<Mesh> mUnitSphereStencilMesh;
			SPtr<Mesh> mUnitBoxStencilMesh;
			SPtr<Mesh> mSpotLightStencilMesh;
			SPtr<Mesh> mSkyBoxMesh;
		};

		/** Provides easy access to RendererUtility. */
		B3D_EXPORT RendererUtility& GetRendererUtility();

		/** @} */
	} // namespace render
} // namespace b3d
