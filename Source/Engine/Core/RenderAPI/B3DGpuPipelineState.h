//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Describes blend states for a single render target. */
	struct B3D_EXPORT RenderTargetBlendStateInformation
	{
		bool operator==(const RenderTargetBlendStateInformation& rhs) const;

		/**
		 * Queries is blending enabled for the specified render target. Blending allows you to combine the color from
		 * current and previous pixel based on some value.
		 */
		bool BlendEnable = false;

		/**
		 * Determines what should the source blend factor be. This value determines what will the color being generated
		 * currently be multiplied by.
		 */
		BlendFactor ColorSourceFactor = BF_ONE;

		/**
		 * Determines what should the destination blend factor be. This value determines what will the color already in
		 * render target be multiplied by.
		 */
		BlendFactor ColorDestinationFactor = BF_ZERO;

		/**
		 * Determines how are source and destination colors combined (after they are multiplied by their respective blend
		 * factors).
		 */
		BlendOperation ColorBlendOperation = BO_ADD;

		/**
		 * Determines what should the alpha source blend factor be. This value determines what will the alpha value being
		 * generated currently be multiplied by.
		 */
		BlendFactor AlphaSourceFactor = BF_ONE;

		/**
		 * Determines what should the alpha destination blend factor be. This value determines what will the alpha value
		 * already in render target be multiplied by.
		 */
		BlendFactor AlphaDestinationFactor = BF_ZERO;

		/**
		 * Determines how are source and destination alpha values combined (after they are multiplied by their respective
		 * blend factors).
		 */
		BlendOperation AlphaBlendOperation = BO_ADD;

		/**
		 * Render target write mask allows to choose which pixel components should the pixel shader output.
		 *
		 * Only the first four bits are used. First bit representing red, second green, third blue and fourth alpha value.
		 * Set bits means pixel shader will output those channels.
		 */
		u8 RenderTargetWriteMask = 0xFF;
	};

	/** Describes a graphics pipeline blend state. */
	struct B3D_EXPORT BlendStateInformation
	{
		/**
		 * Alpha to coverage allows you to perform blending without needing to worry about order of rendering like regular
		 * blending does. It requires multi-sampling to be active in order to work, and you need to supply an alpha texture
		 * that determines object transparency.
		 *
		 * Blending is then performed by only using sub-samples covered by the alpha texture for the current pixel and
		 * combining them with sub-samples previously stored.
		 *
		 * Be aware this is a limited technique only useful for certain situations. Unless you are having performance
		 * problems use regular blending.
		 */
		bool EnableAlphaToCoverage = false;

		/**
		 * When not set, only the first render target blend descriptor will be used for all render targets. If set each
		 * render target will use its own blend descriptor.
		 */
		bool EnableIndependantBlend = false;

		RenderTargetBlendStateInformation RenderTargets[B3D_MAXIMUM_RENDER_TARGET_COUNT];

		bool operator==(const BlendStateInformation& rhs) const;

		/**	Generates a hash value from a blend state descriptor. */
		static u64 GenerateHash(const BlendStateInformation& value);
	};

	/** Describes a graphics pipeline rasterizer state. */
	struct B3D_EXPORT RasterizerStateInformation
	{
		/** Polygon mode allows you to draw polygons as solid objects or as wireframe by just drawing their edges. */
		PolygonMode PolygonMode = PM_SOLID;

		/**
		 * Sets vertex winding order. Faces that contain vertices with this order will be culled and not rasterized. Used
		 * primarily for saving cycles by not rendering backfacing faces.
		 */
		CullingMode CullMode = CULL_COUNTERCLOCKWISE;

		/**
		 * Represents a constant depth bias that will offset the depth values of new pixels by the specified amount.
		 *
		 * @note		This is useful if you want to avoid z fighting for objects at the same or similar depth.
		 */
		float DepthBias = 0;

		/**	Maximum depth bias value. */
		float DepthBiasClamp = 0.0f;

		/**
		 * Represents a dynamic depth bias that increases as the slope of the rendered polygons surface increases.
		 * Resulting value offsets depth values of new pixels. This offset will be added on top of the constant depth bias.
		 *
		 * @note	This is useful if you want to avoid z fighting for objects at the same or similar depth.
		 */
		float SlopeScaledDepthBias = 0.0f;

		/**
		 * If true, clipping of polygons past the far Z plane is enabled. This ensures proper Z ordering for polygons
		 * outside of valid depth range (otherwise they all have the same depth). It can be useful to disable if you are
		 * performing stencil operations that count on objects having a front and a back (like stencil shadow) and don't
		 * want to clip the back.
		 */
		bool DepthClipEnable = true;

		/**
		 * Scissor rectangle allows you to cull all pixels outside of the scissor rectangle.
		 *
		 * @see		render::RenderAPI::setScissorRect
		 */
		bool ScissorEnable = false;

		/**
		 * Determines how are samples in multi-sample render targets handled. If disabled all samples in the render target
		 * will be written the same value, and if enabled each sample will be generated separately.
		 *
		 * @note	In order to get an antialiased image you need to both enable this option and use a MSAA render target.
		 */
		bool MultisampleEnable = true;

		/**
		 * Determines should the lines be antialiased. This is separate from multi-sample antialiasing setting as lines can
		 * be antialiased without multi-sampling.
		 *
		 * @note	This setting is usually ignored if MSAA is used, as that provides sufficient antialiasing.
		 */
		bool AntialiasedLineEnable = false;

		bool operator==(const RasterizerStateInformation& rhs) const;

		/**	Generates a hash value from a rasterizer state descriptor. */
		static u64 GenerateHash(const RasterizerStateInformation& value);
	};

	/** Describes a depth stencil state on a graphics pipeline. */
	struct B3D_EXPORT DepthStencilStateInformation
	{
		/**
		 * If enabled, any pixel about to be written will be tested against the depth value currently in the buffer. If the
		 * depth test passes (depending on the set valueand chosen depth comparison function), that pixel is written and
		 * depth is updated (if depth write is enabled).
		 */
		bool DepthReadEnable = true;

		/** If enabled rendering pixels will update the depth buffer value. */
		bool DepthWriteEnable = true;

		/**
		 * Determines what operation should the renderer use when comparing previous and current depth value. If the
		 * operation passes, pixel with the current depth value will be considered visible.
		 */
		CompareFunction DepthComparisonFunc = CMPF_LESS;

		/**
		 * If true then stencil buffer will also be updated when a pixel is written, and pixels will be tested against
		 * the stencil buffer before rendering.
		 */
		bool StencilEnable = false;

		/** Mask to apply to any value read from the stencil buffer, before applying the stencil comparison function. */
		u8 StencilReadMask = 0xFF;

		/**	Mask to apply to any value about to be written in the stencil buffer. */
		u8 StencilWriteMask = 0xFF;

		/**	Operation that happens when stencil comparison function fails on a front facing polygon. */
		StencilOperation FrontStencilFailOp = SOP_KEEP;

		/** Operation that happens when stencil comparison function passes but depth test fails on a front facing polygon. */
		StencilOperation FrontStencilZFailOp = SOP_KEEP;

		/**	Operation that happens when stencil comparison function passes on a front facing polygon. */
		StencilOperation FrontStencilPassOp = SOP_KEEP;

		/**
		 * Stencil comparison function used for front facing polygons. Stencil buffer will be modified according to
		 * previously set stencil operations depending whether this comparison passes or fails.
		 */
		CompareFunction FrontStencilComparisonFunc = CMPF_ALWAYS_PASS;

		/** Operation that happens when stencil comparison function fails on a back facing polygon. */
		StencilOperation BackStencilFailOp = SOP_KEEP;

		/** Operation that happens when stencil comparison function passes but depth test fails on a back facing polygon. */
		StencilOperation BackStencilZFailOp = SOP_KEEP;

		/**	Operation that happens when stencil comparison function passes on a back facing polygon. */
		StencilOperation BackStencilPassOp = SOP_KEEP;

		/**
		 * Stencil comparison function used for back facing polygons. Stencil buffer will be modified according	to
		 * previously set stencil operations depending whether this comparison passes or fails.
		 */
		CompareFunction BackStencilComparisonFunc = CMPF_ALWAYS_PASS;

		bool operator==(const DepthStencilStateInformation& rhs) const;

		/**	Generates a hash value from a depth-stencil state descriptor. */
		static u64 GenerateHash(const DepthStencilStateInformation& value);
	};

	/** Descriptor structure describing a GPU graphics pipeline state. */
	struct GpuGraphicsPipelineStateInformation
	{
		BlendStateInformation BlendState;
		RasterizerStateInformation RasterizerState;
		DepthStencilStateInformation DepthStencilState;

		SPtr<GpuProgram> VertexProgram;
		SPtr<GpuProgram> FragmentProgram;
		SPtr<GpuProgram> GeometryProgram;
		SPtr<GpuProgram> HullProgram;
		SPtr<GpuProgram> DomainProgram;
	};

	/** Descriptor structure used for initializing a GPU graphics pipeline state. */
	struct GpuGraphicsPipelineStateCreateInformation : GpuGraphicsPipelineStateInformation
	{
		GpuGraphicsPipelineStateCreateInformation() = default;

		GpuGraphicsPipelineStateCreateInformation(const GpuGraphicsPipelineStateInformation& other)
			: GpuGraphicsPipelineStateInformation(other)
		{}
	};

	/** Descriptor structure describing a GPU compute pipeline state. */
	struct GpuComputePipelineStateInformation
	{
		SPtr<GpuProgram> Program;
	};

	/** Descriptor structure used for initializing a GPU compute pipeline state. */
	struct GpuComputePipelineStateCreateInformation : GpuComputePipelineStateInformation 
	{
		GpuComputePipelineStateCreateInformation() = default;

		GpuComputePipelineStateCreateInformation(const GpuComputePipelineStateInformation& other)
			: GpuComputePipelineStateInformation(other)
		{}
	};

	/**
	 * Describes the state of the GPU pipeline that determines how are primitives rendered. It consists of programmable
	 * states (vertex, fragment, geometry, etc. GPU programs), as well as a set of fixed states (blend, rasterizer,
	 * depth-stencil).
	 *
	 * @note	Thread safe (Immutable).
	 */
	class B3D_EXPORT GpuGraphicsPipelineState
	{
	public:
		GpuGraphicsPipelineState(GpuDevice& gpuDevice, const GpuGraphicsPipelineStateCreateInformation& createInformation);
		virtual ~GpuGraphicsPipelineState() = default;

		/** Initializes the object. The object should not be used before this is called. */
		virtual void Initialize();

		bool HasVertexProgram() const { return mData.VertexProgram != nullptr; }
		bool HasFragmentProgram() const { return mData.FragmentProgram != nullptr; }
		bool HasGeometryProgram() const { return mData.GeometryProgram != nullptr; }
		bool HasHullProgram() const { return mData.HullProgram != nullptr; }
		bool HasDomainProgram() const { return mData.DomainProgram != nullptr; }

		const BlendStateInformation& GetBlendState() const { return mData.BlendState; }
		const RasterizerStateInformation& GetRasterizerState() const { return mData.RasterizerState; }
		const DepthStencilStateInformation& GetDepthStencilState() const { return mData.DepthStencilState; }

		const SPtr<GpuProgram>& GetVertexProgram() const { return mData.VertexProgram; }
		const SPtr<GpuProgram>& GetFragmentProgram() const { return mData.FragmentProgram; }
		const SPtr<GpuProgram>& GetGeometryProgram() const { return mData.GeometryProgram; }
		const SPtr<GpuProgram>& GetHullProgram() const { return mData.HullProgram; }
		const SPtr<GpuProgram>& GetDomainProgram() const { return mData.DomainProgram; }

		/** Returns an object containing the layout of all parameters in all the GPU programs used in this pipeline state. */
		const SPtr<GpuPipelineParameterLayout>& GetParameterLayout() const { return mParameterLayout; }

	protected:
		GpuDevice& mGpuDevice;
		GpuGraphicsPipelineStateInformation mData;
		SPtr<GpuPipelineParameterLayout> mParameterLayout;
	};

	/**
	 * Describes the state of the GPU pipeline that determines how are compute programs executed. It consists of
	 * of a single programmable state (GPU program). 
	 *
	 * @note	Thread safe (Immutable).
	 */
	class B3D_EXPORT GpuComputePipelineState
	{
	public:
		GpuComputePipelineState(GpuDevice& gpuDevice, const GpuComputePipelineStateCreateInformation& createInformation);
		virtual ~GpuComputePipelineState() = default;

		/** Initializes the object. The object should not be used before this is called. */
		virtual void Initialize();

		const SPtr<GpuProgram>& GetProgram() const { return mData.Program; }

		/** Returns an object containing the layout of all parameters in the GPU program used in this pipeline state. */
		const SPtr<GpuPipelineParameterLayout>& GetParameterLayout() const { return mParameterLayout; }

	protected:
		GpuDevice& mGpuDevice;

		GpuComputePipelineStateInformation mData;
		SPtr<GpuPipelineParameterLayout> mParameterLayout;
	};

	/** @} */
} // namespace b3d
