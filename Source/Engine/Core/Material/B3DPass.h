//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DColor.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "Reflection/B3DIReflectable.h"
#include "CoreObject/B3DCoreObject.h"
#include "GpuBackend/B3DGpuPipelineState.h"

namespace b3d
{
	/** @addtogroup Material
	 *  @{
	 */

	/** Descriptor structure used for initializing a shader pass. */
	struct PassCreateInformation
	{
		BlendStateInformation BlendStateInformation;
		RasterizerStateInformation RasterizerStateInformation;
		DepthStencilStateInformation DepthStencilStateInformation;
		u32 StencilRefValue = 0;

		GpuProgramCreateInformation VertexProgramCreateInformation;
		GpuProgramCreateInformation FragmentProgramCreateInformation;
		GpuProgramCreateInformation GeometryProgramCreateInformation;
		GpuProgramCreateInformation HullProgramCreateInformation;
		GpuProgramCreateInformation DomainProgramCreateInformation;
		GpuProgramCreateInformation ComputeProgramCreateInformation;
	};

	/** @} */

	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** Contains common functionality used by both main and render thread counterparts of Pass. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TPass
	{
	public:
		virtual ~TPass() = default;

		/**	Returns true if this pass has some element of transparency. */
		bool HasBlending() const;

		/** Returns true if the pass executes a compute program. */
		bool IsCompute() const { return !mData.ComputeProgramCreateInformation.Source.empty(); }

		/** Gets the stencil reference value that is used when performing operations using the stencil buffer. */
		u32 GetStencilRefValue() const { return mData.StencilRefValue; }

		/** Returns the GPU program descriptor for the specified GPU program type. */
		const GpuProgramCreateInformation& GetGpuProgramCreateInformation(GpuProgramType type) const;

		/**
		 * Returns the graphics pipeline state describing this pass, or null if its a compute pass.
		 * Only valid after Compile() has been called.
		 */
		const TShared<GpuGraphicsPipelineState>& GetGraphicsPipelineState() const { return mGraphicsPipelineState; }

		/**
		 * Returns the compute pipeline state describing this pass, or null if its a graphics pass.
		 * Only valid after compile has been called.
		 */
		const TShared<GpuComputePipelineState>& GetComputePipelineState() const { return mComputePipelineState; }
	protected:
		TPass();
		TPass(const PassCreateInformation& createInformation);

		/** Creates either the graphics or the compute pipeline state from the stored pass data. */
		void CreatePipelineState();

		PassCreateInformation mData;
		TShared<GpuGraphicsPipelineState> mGraphicsPipelineState;
		TShared<GpuComputePipelineState> mComputePipelineState;
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	/**
	 * Class defining a single pass of a variation (of a material). Pass may contain multiple GPU programs (vertex,
	 * fragment, geometry, etc.), and a set of pipeline states (blend, rasterizer, etc.). When initially created the pass
	 * is in its uncompiled state. It needs to be explicitly compiled by calling Compile() before use.
	 *
	 * @note	Main thread.
	 */
	class B3D_EXPORT Pass : public IReflectable, public CoreObject, public TPass<false>
	{
	public:
		virtual ~Pass() = default;

		/**
		 * Initializes the pass internals by compiling the GPU programs and creating the relevant pipeline state. This
		 * method must be called before pass pipelines can be retrieved. After initial compilation further calls do this
		 * method will perform no operation.
		 */
		void Compile();

		/**	Creates a new empty pass. */
		static TShared<Pass> Create(const PassCreateInformation& createInformation);

	protected:
		friend class Variation;
		friend class render::Pass;
		struct SyncPacket;

		Pass() = default;
		Pass(const PassCreateInformation& createInformation);

		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;
		TShared<render::RenderProxy> CreateRenderProxy() const override;

		/**	Creates a new empty pass but doesn't initialize it. */
		static TShared<Pass> CreateEmpty();

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class PassRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	class PassRenderProxyRTTI;

	namespace render
	{
		/** @addtogroup Material-Internal
		 *  @{
		 */

		/**
		 * Render thread counterpart of b3d::Pass.
		 *
		 * @note	Render thread.
		 */
		class B3D_EXPORT Pass : public IReflectable, public RenderProxy, public TPass<true>
		{
		public:
			virtual ~Pass() = default;

			/**	Creates a new pass. */
			static TShared<Pass> Create(const PassCreateInformation& createInformation);

			/**	Creates a new empty pass. */
			static TShared<Pass> CreateEmpty();

			/** @copydoc b3d::Pass::Compile */
			void Compile();

		protected:
			friend class b3d::Pass;
			friend class Variation;

			Pass() = default;
			Pass(const PassCreateInformation& desc);

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class b3d::PassRenderProxyRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};

		/** @} */
	} // namespace render
} // namespace b3d
