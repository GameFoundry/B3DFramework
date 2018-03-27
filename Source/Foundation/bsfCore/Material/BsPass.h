//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsColor.h"
#include "RenderAPI/BsBlendState.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "RenderAPI/BsGpuProgram.h"
#include "Reflection/BsIReflectable.h"
#include "CoreThread/BsCoreObject.h"
#include "RenderAPI/BsGpuPipelineState.h"

namespace bs
{
	/** @addtogroup Material
	 *  @{
	 */

	/** Descriptor structure used for initializing a shader pass. */
	struct PASS_DESC
	{
		BLEND_STATE_DESC blendStateDesc;
		RASTERIZER_STATE_DESC rasterizerStateDesc;
		DEPTH_STENCIL_STATE_DESC depthStencilStateDesc;
		UINT32 stencilRefValue;

		GPU_PROGRAM_DESC vertexProgramDesc;
		GPU_PROGRAM_DESC fragmentProgramDesc;
		GPU_PROGRAM_DESC geometryProgramDesc;
		GPU_PROGRAM_DESC hullProgramDesc;
		GPU_PROGRAM_DESC domainProgramDesc;
		GPU_PROGRAM_DESC computeProgramDesc;
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Contains all data used by a pass, templated so it may contain both core and sim thread data. */
	template<bool Core>
	struct TPassTypes
	{ };

	template<>
	struct TPassTypes < false >
	{
		typedef BlendState BlendStateType;
		typedef RasterizerState RasterizerStateType;
		typedef DepthStencilState DepthStencilStateType;
		typedef GpuProgram GpuProgramType;
		typedef GraphicsPipelineState GraphicsPipelineStateType;
		typedef ComputePipelineState ComputePipelineStateType;
	};

	template<>
	struct TPassTypes < true >
	{
		typedef ct::BlendState BlendStateType;
		typedef ct::RasterizerState RasterizerStateType;
		typedef ct::DepthStencilState DepthStencilStateType;
		typedef ct::GpuProgram GpuProgramType;
		typedef ct::GraphicsPipelineState GraphicsPipelineStateType;
		typedef ct::ComputePipelineState ComputePipelineStateType;
	};

	/** Contains common functionality used by both sim and core thread versions of Pass. */
	template<bool Core>
	class BS_CORE_EXPORT TPass
	{
	public:
		typedef typename TPassTypes<Core>::BlendStateType BlendStateType;
		typedef typename TPassTypes<Core>::RasterizerStateType RasterizerStateType;
		typedef typename TPassTypes<Core>::DepthStencilStateType DepthStencilStateType;
		typedef typename TPassTypes<Core>::GpuProgramType GpuProgramType;
		typedef typename TPassTypes<Core>::GraphicsPipelineStateType GraphicsPipelineStateType;
		typedef typename TPassTypes<Core>::ComputePipelineStateType ComputePipelineStateType;
		typedef typename TGpuPipelineStateTypes<Core>::StateDescType PipelineStateDescType;

		virtual ~TPass() { }

		/**	Returns true if this pass has some element of transparency. */
		bool hasBlending() const;

		/** Returns true if the pass executes a compute program. */
		bool isCompute() const { return !mData.computeProgramDesc.source.empty(); }

		/** Gets the stencil reference value that is used when performing operations using the stencil buffer. */
		UINT32 getStencilRefValue() const { return mData.stencilRefValue; }

		/** Returns the GPU program descriptor for the specified GPU program type. */
		const GPU_PROGRAM_DESC& getProgramDesc(bs::GpuProgramType type) const;

		/** 
		 * Returns the graphics pipeline state describing this pass, or null if its a compute pass.
		 * Only valid after compile() has been called.
		 */
		const SPtr<GraphicsPipelineStateType>& getGraphicsPipelineState() const { return mGraphicsPipelineState; }

		/** 
		 * Returns the compute pipeline state describing this pass, or null if its a graphics pass. 
		 * Only valid after compile has been called.
		 */
		const SPtr<ComputePipelineStateType>& getComputePipelineState() const { return mComputePipelineState; }
	protected:
		TPass();
		TPass(const PASS_DESC& desc);

		/** Creates either the graphics or the compute pipeline state from the stored pass data. */
		void createPipelineState();

		PASS_DESC mData;
		SPtr<GraphicsPipelineStateType> mGraphicsPipelineState;
		SPtr<ComputePipelineStateType> mComputePipelineState;
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	/**
	 * Class defining a single pass of a technique (of a material). Pass may contain multiple GPU programs (vertex, 
	 * fragment, geometry, etc.), and a set of pipeline states (blend, rasterizer, etc.). When initially created the pass
	 * is in its uncompiled state. It needs to be explicitly compiled by calling compile() before use.
	 *
	 * @note	Sim thread.
	 */
	class BS_CORE_EXPORT Pass : public IReflectable, public CoreObject, public TPass<false>
	{
	public:
		virtual ~Pass() { }

		/** Retrieves an implementation of a pass usable only from the core thread. */
		SPtr<ct::Pass> getCore() const;

		/** 
		 * Initializes the pass internals by compiling the GPU programs and creating the relevant pipeline state. This 
		 * method must be called before pass pipelines can be retrieved. After initial compilation further calls do this
		 * method will perform no operation.
		 */
		void compile();

		/**	Creates a new empty pass. */
		static SPtr<Pass> create(const PASS_DESC& desc);

	protected:
		friend class Technique;

		Pass() { }
		Pass(const PASS_DESC& desc);

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/**	Creates a new empty pass but doesn't initialize it. */
		static SPtr<Pass> createEmpty();

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class PassRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/**
	 * Core thread counterpart of bs::Pass.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT Pass : public CoreObject, public TPass<true>
	{
	public:
		virtual ~Pass() { }

		/**	Creates a new empty pass. */
		static SPtr<Pass> create(const PASS_DESC& desc);

		/** @copydoc bs::Pass::compile */
		void compile();

	protected:
		friend class bs::Pass;
		friend class Technique;

		Pass() { }
		Pass(const PASS_DESC& desc);

		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;
	};

	/** @} */
	}
}
