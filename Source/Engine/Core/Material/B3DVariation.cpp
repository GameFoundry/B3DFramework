//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DVariation.h"

#include "B3DApplication.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "Renderer/B3DRendererManager.h"
#include "Material/B3DPass.h"
#include "Material/B3DShaderRegistry.h"
#include "Renderer/B3DRenderer.h"
#include "RTTI/B3DVariationRTTI.h"
#include "GpuBackend/B3DGpuDevice.h"

using namespace b3d;

VariationBase::VariationBase(const String& language, const ShaderVariationParameters& variationParameters)
	: mLanguage(language), mVariationParameters(variationParameters)
{
}

bool VariationBase::IsSupported() const
{
	const TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();

	if((gpuDevice != nullptr && gpuDevice->IsGpuProgramLanguageSupported(mLanguage)) || mLanguage == "Any")
		return true;

	return false;
}

template <bool IsRenderProxy>
TVariation<IsRenderProxy>::TVariation(const WeakSPtr<ShaderType>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<TPrecompiledVariationData<IsRenderProxy>>& precompiledData)
	: VariationBase(language, variationParameters), mOwner(owner), mPasses(precompiledData.value_or(TPrecompiledVariationData<IsRenderProxy>()).PrecompiledPasses), mHasPassData(precompiledData.has_value())
{ }

template <bool IsRenderProxy>
TVariation<IsRenderProxy>::TVariation()
	: VariationBase(StringUtility::kBlank, ShaderVariationParameters())
{}

template <bool IsRenderProxy>
TShared<typename TVariation<IsRenderProxy>::PassType> TVariation<IsRenderProxy>::GetPass(u32 passIndex) const
{
	if(!mHasPassData)
	{
		B3D_LOG(Error, LogMaterial, "Unable to retrieve shader variation pass. The variation has not been compiled.");
		return nullptr;
	}

	if(passIndex >= (u32)mPasses.size())
	{
		B3D_LOG(Error, LogMaterial, "Unable to retrieve shader variation pass. The provided index={0} is out of range=[0, {1}].", passIndex, mPasses.size());
		return nullptr;
	}

	return mPasses[passIndex];
}

template <bool IsRenderProxy>
u32 TVariation<IsRenderProxy>::GetPassCount() const
{
	if(!mHasPassData)
	{
		B3D_LOG(Error, LogMaterial, "Unable to retrieve shader variation pass count. The variation has not been compiled.");
		return 0;
	}
	
	return (u32)mPasses.size();
}

template <bool IsRenderProxy>
void TVariation<IsRenderProxy>::SetCompiledPassData(TInlineArray<TShared<PassType>, 1> compiledPasses)
{
	mPasses = std::move(compiledPasses);
	mHasPassData = true;

	MarkRenderProxyDirty(ShaderVariationDirtyFlag::Passes);
	SyncToRenderProxy();
}

template <bool IsRenderProxy>
void TVariation<IsRenderProxy>::SetOwner(const WeakSPtr<ShaderType>& owner)
{
	mOwner = owner;

	MarkRenderProxyDirty(ShaderVariationDirtyFlag::Parent);
	SyncToRenderProxy();
}

template<bool IsRenderProxy>
TAsyncOp<bool> TVariation<IsRenderProxy>::Compile()
{
	// TODO - This should be done async, but XShaderCompiler has issues with multiple threads. Additionally the pass compile needs to be made thread safe on the Vulkan level.
	TAsyncOp<bool> operation;

	if(!mHasPassData)
	{
		const TShared<ShaderType> owner = mOwner.lock();
		if(!B3D_ENSURE(owner != nullptr))
		{
			B3D_LOG(Error, LogMaterial, "Cannot compile shader variation. Parent shader is not assigned.");
			operation.CompleteOperation(false);
			return operation;
		}

		if(!ShaderRegistry::Instance().GetOrCompileVariation<IsRenderProxy>(owner, GetSelf(), mLanguage))
		{
			operation.CompleteOperation(false);
			return operation;
		}
	}

	if(!mIsCompiled)
	{
		for(auto& pass : mPasses)
			pass->Compile();

		mIsCompiled = true;
	}

	operation.CompleteOperation(true);
	return operation;
}

template <bool IsRenderProxy>
TShared<VariationPrecompiledData> TVariation<IsRenderProxy>::GetPrecompiledData() const
{
	TShared<VariationPrecompiledData> data = B3DMakeShared<VariationPrecompiledData>();
	data->Language = mLanguage;
	data->VariationParameters = mVariationParameters;

	for(const auto& pass : mPasses)
		data->Passes.Add(pass->GetInformation());

	return data;
}

// Explicit instantiations must be declared within the template's enclosing namespace
namespace b3d
{
	template class TVariation<false>;
	template class TVariation<true>;
} // namespace b3d

Variation::Variation(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData)
	: TVariation(owner, language, variationParameters, precompiledData)
{}

Variation::Variation()
	: TVariation()
{}

TShared<render::RenderProxy> Variation::CreateRenderProxy() const
{
	const TShared<Shader> owner = mOwner.lock();
	const WeakSPtr<render::Shader> ownerRenderProxy = B3DGetRenderProxy(owner);

	TInlineArray<TShared<render::Pass>, 1> passRenderProxies;
	for(auto& pass : mPasses)
		passRenderProxies.Add(B3DGetRenderProxy(pass));

	TOptional<render::PrecompiledVariationData> precompiledDataRenderProxy = mHasPassData ? render::PrecompiledVariationData(passRenderProxies) : TOptional<render::PrecompiledVariationData>{};

	render::Variation* const renderProxy = new(B3DAllocate<render::Variation>()) render::Variation(ownerRenderProxy, mLanguage, mVariationParameters, precompiledDataRenderProxy);
	const TShared<render::Variation> renderProxyShared = B3DMakeSharedFromExisting<render::Variation>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

void Variation::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	for(auto& pass : mPasses)
		dependencies.push_back(pass.get());
}

void Variation::MarkRenderProxyDirty(ShaderVariationDirtyFlags flags)
{
	CoreObject::MarkRenderProxyDataDirty(flags);
}

void Variation::SyncToRenderProxy()
{
	CoreObject::SyncToRenderProxy();
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(Variation, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mPasses)
		B3D_SYNC_BLOCK_ENTRY(mHasPassData)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(TShared<Shader>, Owner)
	B3D_SYNC_BLOCK_END
}

RenderProxySyncPacket* Variation::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	const ShaderVariationDirtyFlags dirtyFlags = (ShaderVariationDirtyFlags)flags;

	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);

	if(dirtyFlags.IsSet(ShaderVariationDirtyFlag::Parent))
		syncPacket->Owner = B3DGetRenderProxy(mOwner.lock());

	return syncPacket;
}

TShared<Variation> Variation::Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData)
{
	Variation* variation = new(B3DAllocate<Variation>()) Variation(owner, language, variationParameters, precompiledData);
	TShared<Variation> variationShared = B3DMakeSharedFromExisting<Variation>(variation);
	variationShared->SetShared(variationShared);
	variationShared->Initialize();

	return variationShared;
}

TShared<Variation> Variation::CreateEmpty()
{
	Variation* variation = new(B3DAllocate<Variation>()) Variation();
	TShared<Variation> variationShared = B3DMakeSharedFromExisting<Variation>(variation);
	variationShared->SetShared(variationShared);

	return variationShared;
}

RTTIType* Variation::GetRttiStatic()
{
	return VariationRTTI::Instance();
}

RTTIType* Variation::GetRtti() const
{
	return Variation::GetRttiStatic();
}

RTTIType* VariationPrecompiledData::GetRttiStatic()
{
	return VariationPrecompiledDataRTTI::Instance();
}

RTTIType* VariationPrecompiledData::GetRtti() const
{
	return VariationPrecompiledData::GetRttiStatic();
}

namespace b3d { namespace render
{
Variation::Variation()
	: TVariation(WeakSPtr<Shader>(), StringUtility::kBlank, ShaderVariationParameters(), {})
{ }

Variation::Variation(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData)
	: TVariation(owner, language, variationParameters, precompiledData)
{}

TShared<Variation> Variation::Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData)
{
	Variation *const variation = new(B3DAllocate<Variation>()) Variation(owner, language, variationParameters, precompiledData);
	TShared<Variation> variationShared = B3DMakeSharedFromExisting<Variation>(variation);
	variationShared->SetShared(variationShared);
	variationShared->Initialize();

	return variationShared;
}

TShared<Variation> Variation::CreateEmpty()
{
	Variation* const variation = new(B3DAllocate<Variation>()) Variation();
	TShared<Variation> variationShared = B3DMakeSharedFromExisting<Variation>(variation);
	variationShared->SetShared(variationShared);
	variationShared->Initialize();

	return variationShared;
}

void Variation::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::Variation::SyncPacket>();
	if(!syncPacket)
		return;

	const ShaderVariationDirtyFlags dirtyFlags = (ShaderVariationDirtyFlags)syncPacket->Flags;
	if(dirtyFlags.IsSet(ShaderVariationDirtyFlag::Parent))
		mOwner = syncPacket->Owner;

	if(dirtyFlags.IsSet(ShaderVariationDirtyFlag::Passes))
		syncPacket->ApplySyncData(this);
}

RTTIType* Variation::GetRttiStatic()
{
	return VariationRenderProxyRTTI::Instance();
}

RTTIType* Variation::GetRtti() const
{
	return Variation::GetRttiStatic();
}
}}
