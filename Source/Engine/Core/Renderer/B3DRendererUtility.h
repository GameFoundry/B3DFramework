//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Math/B3DArea2.h"
#include "Math/B3DArea2.h"
#include "GpuBackend/B3DGpuDevice.h"
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
			template <u32 MSAA, u32 MODE, u32 BLEND, u32 WRITE_ALPHA, u32 SRGB_ENCODE>
			static const ShaderVariationParameters& GetVariation(bool output32Bit)
			{
				static const ShaderVariationParameters variations[2] = { BuildVariation<MSAA, MODE, BLEND, WRITE_ALPHA, SRGB_ENCODE>(false), BuildVariation<MSAA, MODE, BLEND, WRITE_ALPHA, SRGB_ENCODE>(true) };
				return variations[output32Bit ? 1 : 0];
			}

			/** Assembles the variation parameters for GetVariation(). */
			template <u32 MSAA, u32 MODE, u32 BLEND, u32 WRITE_ALPHA, u32 SRGB_ENCODE>
			static ShaderVariationParameters BuildVariation(bool output32Bit)
			{
				return ShaderVariationParameters(
					TInlineArray<ShaderVariationParameter, 4>({
						ShaderVariationParameter("MSAA_COUNT", MSAA),
						ShaderVariationParameter("MODE", MODE),
						ShaderVariationParameter("BLEND", BLEND),
						ShaderVariationParameter("WRITE_ALPHA", WRITE_ALPHA),
						ShaderVariationParameter("SRGB_ENCODE", SRGB_ENCODE),
						ShaderVariationParameter("OUTPUT_32BIT", output32Bit),
					}));
			}

		public:
			BlitMat() = default;
			void Initialize() override;

			/** Creates a new GpuParameterSet and assigns the provided texture. */
			TShared<GpuParameterSet> Prepare(const TShared<Texture>& source);

			/** Creates the parameter set of @p pass and assigns @p source to it. Returns the set to pass to Execute(). */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const TShared<Texture>& source);

			/**
			 * Binds the material and @p set, a set from Prepare(FrameGraphPass&, const TShared<Texture>&), and draws the blit into the
			 * current viewport.
			 *
			 * @param context	Context of the pass the set was prepared for.
			 * @param set		Parameter set holding the source texture.
			 * @param area		Area to use for UV coordinates when drawing the fullscreen quad. For unfiltered blits, this controls the
			 *					sampled region. For filtered blits, normalized (0,1) UVs are used.
			 * @param flipUV	If true, vertical texture coordinates are flipped.
			 */
			void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const Area2& area, bool flipUV) const;

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	msaaCount			Number of MSAA samples in the input texture. If larger than 1 the texture will be resolved
			 *								before written to the destination.
			 * @param	isColor				If true the input is assumed to be a 4-component color texture. If false it is assumed
			 *								the input is a 1-component depth texture. This controls how is the texture resolve and is
			 *								only relevant if @p msaaCount > 1. Color texture MSAA samples will be averaged, while for
			 *								depth textures the minimum of all samples will be used.
			 * @param	isFiltered			True if to apply bilinear filtering to the sampled texture. Only relevant for color
			 *								textures with no multiple samples.
			 * @param	blend				If true blit source will be blended with the target image, rather than overwriting it, using
			 *								the alpha value from the source.
			 * @param	premultipliedAlpha	If true the source color is assumed to be premultiplied by its alpha. Only relevant when
			 *								@p blend is enabled.
			 * @param	writeAlpha			If true, alpha value from the source will be passed to the destination. Only relevant when
			 *								@p blend in enabled.
			 * @param	srgbEncode			If true, the sampled color is encoded from linear into sRGB (gamma) space before being
			 *								written. Used when compositing a hardware-sRGB source (which decodes to linear on sample)
			 *								onto a non-sRGB target that expects sRGB-encoded values. Only relevant for color blits,
			 *								and not supported for multisampled sources.
			 * @param	output32Bit			True if the destination color surface uses a 32-bit float format. Backends that bake
			 *								the render target format into the fragment program would otherwise truncate the
			 *								output to 16 bits. Only relevant for color blits.
			 */
			static BlitMat* GetVariation(u32 msaaCount, bool isColor, bool isFiltered, bool blend = false, bool premultipliedAlpha = false, bool writeAlpha = false, bool srgbEncode = false, bool output32Bit = false);

		private:
			/** Returns a single-sample color variation that blends using the provided BLEND shader mode. */
			template <u32 BLEND>
			static BlitMat* GetBlendVariation(bool isFiltered, bool writeAlpha, bool srgbEncode, bool output32Bit);

			bool mIsFiltered = false;
			GpuParameterSampledTexture mSourceParameter;
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
			 * Creates the parameter set of @p pass, which blends @p source with the color attachment of the pass.
			 *
			 * @param	pass	Pass that draws the material.
			 * @param	source	Texture to blend with the attachment.
			 * @param	tint	Optional value to multiply all the values from @p source before blending.
			 * @return			Parameter set to pass to Execute().
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const TShared<Texture>& source, const Color& tint = Color::kWhite);

			/** Binds the material and @p set, a set from Prepare(), and draws a full screen quad. */
			void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const;

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
			 * Creates the parameter set of @p pass, which filters @p source and additively blends the result with the color attachment
			 * of the pass.
			 *
			 * @param	pass	Pass that draws the material.
			 * @param	source	Texture to filter.
			 * @param	tint	Optional value to multiply all the values from @p source before blending.
			 * @return			Parameter set to pass to Execute().
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const TShared<Texture>& source, const Color& tint = Color::kWhite);

			/** Binds the material and @p set, a set from Prepare(), and draws a full screen quad. */
			void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const;

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
			B3D_UNIFORM_BUFFER_MEMBER(Vector4UI, gIntegerClearValue)
			B3D_UNIFORM_BUFFER_MEMBER(Color, gColorClearValue)
		B3D_UNIFORM_BUFFER_END

		extern ClearUniformDefinition gClearUniformDefinition;

		/** Shader that clears the currently bound render target to an integer value. */
		class B3D_EXPORT ClearMaterial : public RendererMaterial<ClearMaterial>
		{
			RMAT_DEF("Clear.bsl");

			/** Helper method used for initializing variations of this material. */
			template <u32 MODE>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					TInlineArray<ShaderVariationParameter, 4>({ ShaderVariationParameter("MODE", MODE) }));

				return variation;
			}

		public:
			ClearMaterial() = default;
			void Initialize() override;

			/**
			 * Creates the parameter set of @p pass and assigns the integer value the target will be cleared to. Only valid on the integer
			 * variation. Returns the set to pass to Execute().
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, u32 value);

			/**
			 * Creates the parameter set of @p pass and assigns the color the target will be cleared to. Only valid on the color
			 * variation. Returns the set to pass to Execute().
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const Color& color);

			/** Binds the material and @p set, a set from Prepare(), and clears the current viewport of the pass's target. */
			void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const;

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	isColor		If true the target is cleared to a floating point color, otherwise to an integer value.
			 */
			static ClearMaterial* GetVariation(bool isColor);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
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
			void SetPass(GpuCommandBuffer& commandBuffer, const TShared<Material>& material, u32 passIndex = 0, u32 variationIndex = 0);

			/**
			 * Activates the specified material pass for compute. Any further dispatch calls will be executed using this pass.
			 *
			 * @param	commandBuffer	Command buffer to bind the pass to.
			 * @param	material		Material containing the pass.
			 * @param	passIndex		Index of the pass in the material.
			 *
			 * @note	Render thread.
			 */
			void SetComputePass(GpuCommandBuffer& commandBuffer, const TShared<Material>& material, u32 passIndex = 0);

			/**
			 * Sets parameters (textures, samplers, buffers) for the currently active pass.
			 *
			 * @param	commandBuffer	Command buffer to bind the pass parameters to.
			 * @param	adapter			Object containing the parameters.
			 * @param	passIndex		Pass for which to set the parameters.
			 *
			 * @note	Render thread.
			 */
			void SetPassParams(GpuCommandBuffer& commandBuffer, const TShared<MaterialParameterAdapter>& adapter, u32 passIndex = 0);

#if B3D_BUILD_TYPE_DEVELOPMENT
			/**
			 * Checks that every pass of a material variation declares the resources of @p expected in set @p setIndex, with the
			 * same names, slots, types and array sizes, and no others. Logs an error naming the material, pass and resource for
			 * the first difference.
			 *
			 * @return	True if every pass matches.
			 */
			bool ValidateRendererSetLayout(const Material& material, u32 variationIndex, u32 setIndex, const GpuPipelineParameterSetLayout& expected) const;
#endif

			/**
			 * Draws the specified mesh.
			 *
			 * @param	commandBuffer	Command buffer to encode the draw command on.
			 * @param	mesh			Mesh to draw.
			 * @param	instanceCount	Number of times to draw the mesh using instanced rendering.
			 *
			 * @note	Render thread.
			 */
			void Draw(GpuCommandBuffer& commandBuffer, const TShared<MeshBase>& mesh, u32 instanceCount = 1);

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
			void Draw(GpuCommandBuffer& commandBuffer, const TShared<MeshBase>& mesh, const SubMesh& subMesh, u32 instanceCount = 1);

			/**
			 * Draws @p subMesh with the vertices of @p vertexData and the indices of @p indexBuffer.
			 *
			 * @param	commandBuffer	Command buffer to encode the draw command on.
			 * @param	vertexData		Vertex buffers and their description. Buffers are bound to the streams their description assigns them.
			 * @param	indexBuffer		Indices of the vertices of @p vertexData that form the primitives of @p subMesh.
			 * @param	subMesh			Range of indices to draw, and the primitives they form.
			 * @param	instanceCount	Number of times to draw the sub-mesh using instanced rendering.
			 *
			 * @note	Render thread.
			 */
			void Draw(GpuCommandBuffer& commandBuffer, const VertexData& vertexData, const TShared<GpuBuffer>& indexBuffer, const SubMesh& subMesh, u32 instanceCount = 1);

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
			void DrawMorph(GpuCommandBuffer& commandBuffer, const TShared<MeshBase>& mesh, const SubMesh& subMesh, const TShared<GpuBuffer>& morphVertices, const TShared<VertexDescription>& morphVertexDescription);

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

			/** Returns a unit sphere stencil mesh. */
			TShared<Mesh> GetSphereStencil() const { return mUnitSphereStencilMesh; }

			/** Returns a unit axis aligned box stencil mesh. */
			TShared<Mesh> GetBoxStencil() const { return mUnitBoxStencilMesh; }

			/**
			 * Returns a stencil mesh used for a spot light. Actual vertex positions need to be computed in shader as this
			 * method will return uninitialized vertex positions.
			 */
			TShared<Mesh> GetSpotLightStencil() const { return mSpotLightStencilMesh; }

			/** Returns a mesh that can be used for rendering a skybox. */
			TShared<Mesh> GetSkyBoxMesh() const { return mSkyBoxMesh; }

			/** Number of sides in the cone mesh used for spot light stencil volumes. */
			static constexpr u32 kSpotLightStencilSideCount = 20;

			/** Number of slices in the cone mesh used for spot light stencil volumes. */
			static constexpr u32 kSpotLightStencilSliceCount = 10;

		private:
			/** Binds the vertex description and the vertex buffers of @p vertexData. */
			void BindVertexData(GpuCommandBuffer& commandBuffer, const VertexData& vertexData);

			static constexpr u32 kNumQuadVbSlots = 1024;

			TShared<GpuBuffer> mFullScreenQuadIB;
			TShared<GpuBuffer> mFullScreenQuadVB;
			TShared<VertexDescription> mFullscreenQuadVertexDescription;
			u32 mNextQuadVBSlot = 0;

			TShared<Mesh> mUnitSphereStencilMesh;
			TShared<Mesh> mUnitBoxStencilMesh;
			TShared<Mesh> mSpotLightStencilMesh;
			TShared<Mesh> mSkyBoxMesh;
		};

		/** Provides easy access to RendererUtility. */
		B3D_EXPORT RendererUtility& GetRendererUtility();

		/** @} */
	} // namespace render
} // namespace b3d
