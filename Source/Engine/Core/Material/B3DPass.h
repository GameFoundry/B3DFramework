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

	/** Descriptor structure used for describing a shader pass. */
	struct B3D_EXPORT PassInformation : public IReflectable
	{
		BlendStateInformation BlendStateInformation;
		RasterizerStateInformation RasterizerStateInformation;
		DepthStencilStateInformation DepthStencilStateInformation;
		u32 StencilRefValue = 0;

		/** Programs of the pass, one per entry point: graphics programs, a compute program, or ray tracing programs. */
		Vector<GpuProgramCreateInformation> Programs;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PassInformationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Descriptor structure used for creating a Pass. */
	struct PassCreateInformation : PassInformation
	{
		PassCreateInformation() = default;
		PassCreateInformation(const PassInformation& other)
			: PassInformation(other)
		{ }
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
		bool IsCompute() const;

		/** Returns true if the pass holds ray tracing programs, rather than graphics or compute programs. */
		bool IsRayTracing() const;

		/** Gets the stencil reference value that is used when performing operations using the stencil buffer. */
		u32 GetStencilRefValue() const { return mData.StencilRefValue; }

		/**
		 * Returns the GPU program descriptor of the first program of the specified type. A ray tracing pass can have several
		 * programs of one type; GetInformation() lists them all.
		 *
		 * @param	type	Type of the program to look up.
		 * @return			The program descriptor, or an empty one if the pass has no program of @p type.
		 */
		const GpuProgramCreateInformation& GetGpuProgramCreateInformation(GpuProgramType type) const;

		/** Returns the pass description, including the compiled GPU program bytecode. */
		const PassInformation& GetInformation() const { return mData; }

		/**
		 * Returns the graphics pipeline state describing this pass, or null if its a compute or ray tracing pass.
		 * Only valid after Compile() has been called.
		 */
		const TShared<GpuGraphicsPipelineState>& GetGraphicsPipelineState() const { return mGraphicsPipelineState; }

		/**
		 * Returns the compute pipeline state describing this pass, or null if its a graphics or ray tracing pass.
		 * Only valid after compile has been called.
		 */
		const TShared<GpuComputePipelineState>& GetComputePipelineState() const { return mComputePipelineState; }

		/**
		 * Returns the program of an entry point of the pass. Only valid after Compile() has been called.
		 *
		 * @param	entryPoint	Name of the entry point function.
		 * @return				The program, or null if the pass has no such entry point or the device can't run the program.
		 */
		TShared<GpuProgram> GetProgram(const StringView& entryPoint) const;
	protected:
		TPass();
		TPass(const PassCreateInformation& createInformation);

		/**
		 * Creates the programs and, unless this is a ray tracing pass, the graphics or compute pipeline state from the stored
		 * pass data.
		 */
		void CreatePrograms();

		PassInformation mData;

		/** Programs created from PassInformation::Programs, at the same indices. Null where no program could be created. */
		Vector<TShared<GpuProgram>> mPrograms;
		bool mAreProgramsCreated = false;

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
		 * method will perform no operation. Does nothing for a ray tracing pass, whose programs only the render thread
		 * creates (see render::Pass::Compile()).
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

			/**
			 * @copydoc b3d::Pass::Compile
			 *
			 * A ray tracing pass gets its programs, on devices with RSC_RAY_TRACING_PIPELINE.
			 */
			void Compile();

		protected:
			friend class b3d::Pass;
			friend class Variation;

			Pass() = default;
			Pass(const PassCreateInformation& createInformation);

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
