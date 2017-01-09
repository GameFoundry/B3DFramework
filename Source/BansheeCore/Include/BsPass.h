//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsColor.h"
#include "BsIReflectable.h"
#include "BsCoreObject.h"

namespace bs
{
	/** @addtogroup Material
	 *  @{
	 */

	/** Descriptor structure used for initializing a shader pass. */
	struct PASS_DESC
	{
		SPtr<BlendState> blendState;
		SPtr<RasterizerState> rasterizerState;
		SPtr<DepthStencilState> depthStencilState;
		UINT32 stencilRefValue;

		SPtr<GpuProgram> vertexProgram;
		SPtr<GpuProgram> fragmentProgram;
		SPtr<GpuProgram> geometryProgram;
		SPtr<GpuProgram> hullProgram;
		SPtr<GpuProgram> domainProgram;
		SPtr<GpuProgram> computeProgram;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** Descriptor structure used for initializing a core thread variant of a shader pass. */
	struct PASS_DESC
	{
		SPtr<BlendStateCore> blendState;
		SPtr<RasterizerStateCore> rasterizerState;
		SPtr<DepthStencilStateCore> depthStencilState;
		UINT32 stencilRefValue;

		SPtr<GpuProgramCore> vertexProgram;
		SPtr<GpuProgramCore> fragmentProgram;
		SPtr<GpuProgramCore> geometryProgram;
		SPtr<GpuProgramCore> hullProgram;
		SPtr<GpuProgramCore> domainProgram;
		SPtr<GpuProgramCore> computeProgram;
	};

	/** @} */
	}

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
		typedef SPtr<BlendState> BlendStateType;
		typedef SPtr<RasterizerState> RasterizerStateType;
		typedef SPtr<DepthStencilState> DepthStencilStateType;
		typedef SPtr<GpuProgram> GpuProgramType;
		typedef SPtr<GraphicsPipelineState> GraphicsPipelineStateType;
		typedef SPtr<ComputePipelineState> ComputePipelineStateType;
		typedef PASS_DESC PassDescType;
	};

	template<>
	struct TPassTypes < true >
	{
		typedef SPtr<ct::BlendStateCore> BlendStateType;
		typedef SPtr<ct::RasterizerStateCore> RasterizerStateType;
		typedef SPtr<ct::DepthStencilStateCore> DepthStencilStateType;
		typedef SPtr<ct::GpuProgramCore> GpuProgramType;
		typedef SPtr<ct::GraphicsPipelineStateCore> GraphicsPipelineStateType;
		typedef SPtr<ct::ComputePipelineStateCore> ComputePipelineStateType;
		typedef ct::PASS_DESC PassDescType;
	};

	/**
	 * Class defining a single pass of a technique (of a material).
	 *
	 * Pass may contain multiple GPU programs (vertex, fragment, geometry, etc.), and a set of pipeline states (blend, 
	 * rasterizer, etc.).
	 *
	 * @note	Templated so it can be used for both core and non-core versions of a pass.
	 */
	template<bool Core>
	class BS_CORE_EXPORT TPass
    {
    public:
		typedef typename TPassTypes<Core>::BlendStateType BlendStateType;
		typedef typename TPassTypes<Core>::RasterizerStateType RasterizerStateType;
		typedef typename TPassTypes<Core>::DepthStencilStateType DepthStencilStateType;
		typedef typename TPassTypes<Core>::GpuProgramType GpuProgramType;
		typedef typename TPassTypes<Core>::PassDescType PassDescType;
		typedef typename TPassTypes<Core>::GraphicsPipelineStateType GraphicsPipelineStateType;
		typedef typename TPassTypes<Core>::ComputePipelineStateType ComputePipelineStateType;

		virtual ~TPass() { }

        bool hasVertexProgram() const { return mData.vertexProgram != nullptr; }
		bool hasFragmentProgram() const { return mData.fragmentProgram != nullptr; }
		bool hasGeometryProgram() const { return mData.geometryProgram != nullptr; }
		bool hasHullProgram() const { return mData.hullProgram != nullptr; }
		bool hasDomainProgram() const { return mData.domainProgram != nullptr; }
		bool hasComputeProgram() const { return mData.computeProgram != nullptr; }

		/**	Returns true if this pass has some element of transparency. */
		bool hasBlending() const;

		BlendStateType getBlendState() const { return mData.blendState; }
		RasterizerStateType getRasterizerState() const { return mData.rasterizerState; }
		DepthStencilStateType getDepthStencilState() const { return mData.depthStencilState; }

		/** Gets the stencil reference value that is used when performing operations using the stencil buffer. */
		UINT32 getStencilRefValue() const { return mData.stencilRefValue; }

		const GpuProgramType& getVertexProgram() const { return mData.vertexProgram; }
		const GpuProgramType& getFragmentProgram() const { return mData.fragmentProgram; }
		const GpuProgramType& getGeometryProgram() const { return mData.geometryProgram; }
		const GpuProgramType& getHullProgram() const { return mData.hullProgram; }
		const GpuProgramType& getDomainProgram() const { return mData.domainProgram; }
		const GpuProgramType& getComputeProgram() const { return mData.computeProgram; }

		const GraphicsPipelineStateType& getGraphicsPipelineState() const { return mGraphicsPipelineState; }
		const ComputePipelineStateType& getComputePipelineState() const { return mComputePipelineState; }
	protected:
		TPass();
		TPass(const PassDescType& desc);

		PassDescType mData;
		GraphicsPipelineStateType mGraphicsPipelineState;
		ComputePipelineStateType mComputePipelineState;
    };

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	/**
	 * @copydoc	TPass
	 *
	 * @note	Sim thread.
	 */
	class BS_CORE_EXPORT Pass : public IReflectable, public CoreObject, public TPass<false>
    {
    public:
		virtual ~Pass() { }

		/** Retrieves an implementation of a pass usable only from the core thread. */
		SPtr<ct::PassCore> getCore() const;

		/**	Creates a new empty pass. */
		static SPtr<Pass> create(const PASS_DESC& desc);

	protected:
		friend class Technique;

		Pass() { }
		Pass(const PASS_DESC& desc);

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc CoreObject::syncToCore */
		void getCoreDependencies(Vector<CoreObject*>& dependencies) override;

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
	 * @copydoc	TPass
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT PassCore : public CoreObject, public TPass<true>
    {
    public:
		virtual ~PassCore() { }

		/**	Creates a new empty pass. */
		static SPtr<PassCore> create(const PASS_DESC& desc);

	protected:
		friend class Pass;
		friend class TechniqueCore;

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		PassCore() { }
		PassCore(const PASS_DESC& desc);
		PassCore(const PASS_DESC& desc, const SPtr<GraphicsPipelineStateCore>& pipelineState);
		PassCore(const PASS_DESC& desc, const SPtr<ComputePipelineStateCore>& pipelineState);

		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;
    };

	/** @} */
	}
}