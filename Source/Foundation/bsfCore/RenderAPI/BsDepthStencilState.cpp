//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsDepthStencilState.h"
#include "Managers/BsRenderStateManager.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Private/RTTI/BsDepthStencilStateRTTI.h"
#include "Error/BsException.h"
#include "Resources/BsResources.h"

namespace bs
{
	bool DEPTH_STENCIL_STATE_DESC::operator == (const DEPTH_STENCIL_STATE_DESC& rhs) const
	{
		return depthReadEnable == rhs.depthReadEnable &&
			depthWriteEnable == rhs.depthWriteEnable &&
			depthComparisonFunc == rhs.depthComparisonFunc &&
			stencilEnable == rhs.stencilEnable &&
			stencilReadMask == rhs.stencilReadMask &&
			stencilWriteMask == rhs.stencilWriteMask &&
			frontStencilFailOp == rhs.frontStencilFailOp &&
			frontStencilZFailOp == rhs.frontStencilZFailOp &&
			frontStencilPassOp == rhs.frontStencilPassOp &&
			frontStencilComparisonFunc == rhs.frontStencilComparisonFunc &&
			backStencilFailOp == rhs.backStencilFailOp &&
			backStencilZFailOp == rhs.backStencilZFailOp &&
			backStencilPassOp == rhs.backStencilPassOp &&
			backStencilComparisonFunc == rhs.backStencilComparisonFunc;
	}

	DepthStencilProperties::DepthStencilProperties(const DEPTH_STENCIL_STATE_DESC& desc)
		:mData(desc), mHash(DepthStencilState::generateHash(desc))
	{

	}

	DepthStencilState::DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc)
		:mProperties(desc), mId(0)
	{

	}

	SPtr<ct::DepthStencilState> DepthStencilState::getCore() const
	{
		return std::static_pointer_cast<ct::DepthStencilState>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> DepthStencilState::createCore() const
	{
		SPtr<ct::DepthStencilState> core = ct::RenderStateManager::instance().CreateDepthStencilStateInternal(mProperties.mData);
		mId = core->getId(); // Accessing core from sim thread is okay here since core ID is immutable

		return core;
	}

	const SPtr<DepthStencilState>& DepthStencilState::getDefault()
	{
		return RenderStateManager::instance().getDefaultDepthStencilState();
	}

	const DepthStencilProperties& DepthStencilState::getProperties() const
	{
		return mProperties;
	}

	SPtr<DepthStencilState> DepthStencilState::create(const DEPTH_STENCIL_STATE_DESC& desc)
	{
		return RenderStateManager::instance().createDepthStencilState(desc);
	}

	UINT64 DepthStencilState::generateHash(const DEPTH_STENCIL_STATE_DESC& desc)
	{
		size_t hash = 0;
		bs_hash_combine(hash, desc.depthReadEnable);
		bs_hash_combine(hash, desc.depthWriteEnable);
		bs_hash_combine(hash, (UINT32)desc.depthComparisonFunc);
		bs_hash_combine(hash, desc.stencilEnable);
		bs_hash_combine(hash, desc.stencilReadMask);
		bs_hash_combine(hash, desc.stencilWriteMask);
		bs_hash_combine(hash, (UINT32)desc.frontStencilFailOp);
		bs_hash_combine(hash, (UINT32)desc.frontStencilZFailOp);
		bs_hash_combine(hash, (UINT32)desc.frontStencilPassOp);
		bs_hash_combine(hash, (UINT32)desc.frontStencilComparisonFunc);
		bs_hash_combine(hash, (UINT32)desc.backStencilFailOp);
		bs_hash_combine(hash, (UINT32)desc.backStencilZFailOp);
		bs_hash_combine(hash, (UINT32)desc.backStencilPassOp);
		bs_hash_combine(hash, (UINT32)desc.backStencilComparisonFunc);

		return (UINT64)hash;
	}

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/

	RTTITypeBase* DepthStencilState::getRTTIStatic()
	{
		return DepthStencilStateRTTI::instance();
	}

	RTTITypeBase* DepthStencilState::getRTTI() const
	{
		return DepthStencilState::getRTTIStatic();
	}

	namespace ct
	{
	DepthStencilState::DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id)
		: mProperties(desc), mId(id)
	{

	}

	DepthStencilState::~DepthStencilState()
	{

	}

	void DepthStencilState::initialize()
	{
		// Since we cache states it's possible this object was already initialized
		// (i.e. multiple sim-states can share a single core-state)
		if (isInitialized())
			return;

		createInternal();
		CoreObject::initialize();
	}

	const DepthStencilProperties& DepthStencilState::getProperties() const
	{
		return mProperties;
	}

	SPtr<DepthStencilState> DepthStencilState::create(const DEPTH_STENCIL_STATE_DESC& desc)
	{
		return RenderStateManager::instance().createDepthStencilState(desc);
	}

	const SPtr<DepthStencilState>& DepthStencilState::getDefault()
	{
		return RenderStateManager::instance().getDefaultDepthStencilState();
	}
	}
}
