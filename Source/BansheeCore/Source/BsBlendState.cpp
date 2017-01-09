//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsBlendState.h"
#include "BsRenderStateManager.h"
#include "BsRenderAPI.h"
#include "BsBlendStateRTTI.h"
#include "BsResources.h"

namespace bs
{
	bool RENDER_TARGET_BLEND_STATE_DESC::operator == (const RENDER_TARGET_BLEND_STATE_DESC& rhs) const
	{
		return blendEnable == rhs.blendEnable &&
			srcBlend == rhs.srcBlend &&
			dstBlend == rhs.dstBlend &&
			blendOp == rhs.blendOp &&
			srcBlendAlpha == rhs.srcBlendAlpha &&
			dstBlendAlpha == rhs.dstBlendAlpha &&
			blendOpAlpha == rhs.blendOpAlpha &&
			renderTargetWriteMask == rhs.renderTargetWriteMask;
	}

	bool BLEND_STATE_DESC::operator == (const BLEND_STATE_DESC& rhs) const
	{
		bool equals = alphaToCoverageEnable == rhs.alphaToCoverageEnable &&
			independantBlendEnable == rhs.independantBlendEnable;

		if (equals)
		{
			for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
			{
				equals &= renderTargetDesc[i] == rhs.renderTargetDesc[i];
			}
		}

		return equals;
	}

	BlendProperties::BlendProperties(const BLEND_STATE_DESC& desc)
		:mData(desc), mHash(BlendState::generateHash(desc))
	{ }

	bool BlendProperties::getBlendEnabled(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].blendEnable;
	}

	BlendFactor BlendProperties::getSrcBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].srcBlend;
	}

	BlendFactor BlendProperties::getDstBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].dstBlend;
	}

	BlendOperation BlendProperties::getBlendOperation(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].blendOp;
	}

	BlendFactor BlendProperties::getAlphaSrcBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].srcBlendAlpha;
	}

	BlendFactor BlendProperties::getAlphaDstBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].dstBlendAlpha;
	}

	BlendOperation BlendProperties::getAlphaBlendOperation(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].blendOpAlpha;
	}

	UINT8 BlendProperties::getRenderTargetWriteMask(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].renderTargetWriteMask;
	}

	BlendState::BlendState(const BLEND_STATE_DESC& desc)
		:mProperties(desc), mId(0)
	{ }

	BlendState::~BlendState()
	{

	}

	SPtr<ct::BlendStateCore> BlendState::getCore() const
	{
		return std::static_pointer_cast<ct::BlendStateCore>(mCoreSpecific);
	}

	SPtr<ct::CoreObjectCore> BlendState::createCore() const
	{
		SPtr<ct::BlendStateCore> core = ct::RenderStateManager::instance()._createBlendState(mProperties.mData);
		mId = core->getId(); // Accessing core from sim thread is okay here since core ID is immutable

		return core;
	}

	const BlendProperties& BlendState::getProperties() const
	{
		return mProperties;
	}

	const SPtr<BlendState>& BlendState::getDefault()
	{
		return RenderStateManager::instance().getDefaultBlendState();
	}

	SPtr<BlendState> BlendState::create(const BLEND_STATE_DESC& desc)
	{
		return RenderStateManager::instance().createBlendState(desc);
	}

	UINT64 BlendState::generateHash(const BLEND_STATE_DESC& desc)
	{
		size_t hash = 0;
		hash_combine(hash, desc.alphaToCoverageEnable);
		hash_combine(hash, desc.independantBlendEnable);

		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			hash_combine(hash, desc.renderTargetDesc[i].blendEnable);
			hash_combine(hash, (UINT32)desc.renderTargetDesc[i].srcBlend);
			hash_combine(hash, (UINT32)desc.renderTargetDesc[i].dstBlend);
			hash_combine(hash, (UINT32)desc.renderTargetDesc[i].blendOp);
			hash_combine(hash, (UINT32)desc.renderTargetDesc[i].srcBlendAlpha);
			hash_combine(hash, (UINT32)desc.renderTargetDesc[i].dstBlendAlpha);
			hash_combine(hash, (UINT32)desc.renderTargetDesc[i].blendOpAlpha);
			hash_combine(hash, desc.renderTargetDesc[i].renderTargetWriteMask);
		}

		return (UINT64)hash;
	}

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/

	RTTITypeBase* BlendState::getRTTIStatic()
	{
		return BlendStateRTTI::instance();
	}

	RTTITypeBase* BlendState::getRTTI() const
	{
		return BlendState::getRTTIStatic();
	}

	namespace ct
	{
	BlendStateCore::BlendStateCore(const BLEND_STATE_DESC& desc, UINT32 id)
		:mProperties(desc), mId(id)
	{

	}

	BlendStateCore::~BlendStateCore()
	{

	}

	void BlendStateCore::initialize()
	{
		// Since we cache states it's possible this object was already initialized
		// (i.e. multiple sim-states can share a single core-state)
		if (isInitialized())
			return;

		createInternal();
		CoreObjectCore::initialize();
	}

	const BlendProperties& BlendStateCore::getProperties() const
	{
		return mProperties;
	}

	SPtr<BlendStateCore> BlendStateCore::create(const BLEND_STATE_DESC& desc)
	{
		return RenderStateManager::instance().createBlendState(desc);
	}

	const SPtr<BlendStateCore>& BlendStateCore::getDefault()
	{
		return RenderStateManager::instance().getDefaultBlendState();
	}
	}
}