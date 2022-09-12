//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsBlendState.h"
#include "Managers/BsRenderStateManager.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Private/RTTI/BsBlendStateRTTI.h"
#include "Resources/BsResources.h"

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

	bool BlendProperties::GetBlendEnabled(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].blendEnable;
	}

	BlendFactor BlendProperties::GetSrcBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].srcBlend;
	}

	BlendFactor BlendProperties::GetDstBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].dstBlend;
	}

	BlendOperation BlendProperties::GetBlendOperation(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].blendOp;
	}

	BlendFactor BlendProperties::GetAlphaSrcBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].srcBlendAlpha;
	}

	BlendFactor BlendProperties::GetAlphaDstBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].dstBlendAlpha;
	}

	BlendOperation BlendProperties::GetAlphaBlendOperation(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].blendOpAlpha;
	}

	UINT8 BlendProperties::GetRenderTargetWriteMask(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].renderTargetWriteMask;
	}

	BlendState::BlendState(const BLEND_STATE_DESC& desc)
		:mProperties(desc), mId(0)
	{ }

	BlendState::~BlendState()
	{

	}

	SPtr<ct::BlendState> BlendState::GetCore() const
	{
		return std::static_pointer_cast<ct::BlendState>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> BlendState::CreateCore() const
	{
		SPtr<ct::BlendState> core = ct::RenderStateManager::instance()._createBlendState(mProperties.mData);
		mId = core->GetId(); // Accessing core from sim thread is okay here since core ID is immutable

		return core;
	}

	const BlendProperties& BlendState::GetProperties() const
	{
		return mProperties;
	}

	const SPtr<BlendState>& BlendState::GetDefault()
	{
		return RenderStateManager::Instance().GetDefaultBlendState();
	}

	SPtr<BlendState> BlendState::Create(const BLEND_STATE_DESC& desc)
	{
		return RenderStateManager::Instance().CreateBlendState(desc);
	}

	UINT64 BlendState::GenerateHash(const BLEND_STATE_DESC& desc)
	{
		size_t hash = 0;
		bs_hash_combine(hash, desc.alphaToCoverageEnable);
		bs_hash_combine(hash, desc.independantBlendEnable);

		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			bs_hash_combine(hash, desc.renderTargetDesc[i].blendEnable);
			bs_hash_combine(hash, (UINT32)desc.renderTargetDesc[i].srcBlend);
			bs_hash_combine(hash, (UINT32)desc.renderTargetDesc[i].dstBlend);
			bs_hash_combine(hash, (UINT32)desc.renderTargetDesc[i].blendOp);
			bs_hash_combine(hash, (UINT32)desc.renderTargetDesc[i].srcBlendAlpha);
			bs_hash_combine(hash, (UINT32)desc.renderTargetDesc[i].dstBlendAlpha);
			bs_hash_combine(hash, (UINT32)desc.renderTargetDesc[i].blendOpAlpha);
			bs_hash_combine(hash, desc.renderTargetDesc[i].renderTargetWriteMask);
		}

		return (UINT64)hash;
	}

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/

	RTTITypeBase* BlendState::getRTTIStatic()
	{
		return BlendStateRTTI::Instance();
	}

	RTTITypeBase* BlendState::getRTTI() const
	{
		return BlendState::GetRTTIStatic();
	}

	namespace ct
	{
	BlendState::BlendState(const BLEND_STATE_DESC& desc, UINT32 id)
		:mProperties(desc), mId(id)
	{

	}

	BlendState::~BlendState()
	{

	}

	void BlendState::Initialize()
	{
		// Since we cache states it's possible this object was already initialized
		// (i.e. multiple sim-states can share a single core-state)
		if (isInitialized())
			return;

		createInternal();
		CoreObject::initialize();
	}

	const BlendProperties& BlendState::GetProperties() const
	{
		return mProperties;
	}

	SPtr<BlendState> BlendState::Create(const BLEND_STATE_DESC& desc)
	{
		return RenderStateManager::Instance().CreateBlendState(desc);
	}

	const SPtr<BlendState>& BlendState::GetDefault()
	{
		return RenderStateManager::Instance().GetDefaultBlendState();
	}
	}
}
