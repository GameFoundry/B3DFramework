//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsDepthStencilState.h"
#include "BsRenderStateManager.h"
#include "BsRenderAPI.h"
#include "BsDepthStencilStateRTTI.h"
#include "BsException.h"
#include "BsResources.h"

namespace BansheeEngine
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

	DepthStencilStateCore::DepthStencilStateCore(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id)
		: mProperties(desc), mId(id)
	{

	}

	DepthStencilStateCore::~DepthStencilStateCore()
	{

	}

	void DepthStencilStateCore::initialize()
	{
		// Since we cache states it's possible this object was already initialized
		// (i.e. multiple sim-states can share a single core-state)
		if (isInitialized())
			return;

		createInternal();
		CoreObjectCore::initialize();
	}

	const DepthStencilProperties& DepthStencilStateCore::getProperties() const
	{
		return mProperties;
	}

	SPtr<DepthStencilStateCore> DepthStencilStateCore::create(const DEPTH_STENCIL_STATE_DESC& desc)
	{
		return RenderStateCoreManager::instance().createDepthStencilState(desc);
	}

	const SPtr<DepthStencilStateCore>& DepthStencilStateCore::getDefault()
	{
		return RenderStateCoreManager::instance().getDefaultDepthStencilState();
	}

	DepthStencilState::DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc)
		:mProperties(desc), mId(0)
	{

	}

	DepthStencilState::~DepthStencilState()
	{

	}

	SPtr<DepthStencilStateCore> DepthStencilState::getCore() const
	{
		return std::static_pointer_cast<DepthStencilStateCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> DepthStencilState::createCore() const
	{
		SPtr<DepthStencilStateCore> core = RenderStateCoreManager::instance()._createDepthStencilState(mProperties.mData);
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
		hash_combine(hash, desc.depthReadEnable);
		hash_combine(hash, desc.depthWriteEnable);
		hash_combine(hash, (UINT32)desc.depthComparisonFunc);
		hash_combine(hash, desc.stencilEnable);
		hash_combine(hash, desc.stencilReadMask);
		hash_combine(hash, desc.stencilWriteMask);
		hash_combine(hash, (UINT32)desc.frontStencilFailOp);
		hash_combine(hash, (UINT32)desc.frontStencilZFailOp);
		hash_combine(hash, (UINT32)desc.frontStencilPassOp);
		hash_combine(hash, (UINT32)desc.frontStencilComparisonFunc);
		hash_combine(hash, (UINT32)desc.backStencilFailOp);
		hash_combine(hash, (UINT32)desc.backStencilZFailOp);
		hash_combine(hash, (UINT32)desc.backStencilPassOp);
		hash_combine(hash, (UINT32)desc.backStencilComparisonFunc);

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
}