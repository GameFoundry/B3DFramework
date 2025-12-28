//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DVariation.h"

#include "B3DApplication.h"
#include "B3DShaderCompiler.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "Error/B3DException.h"
#include "Renderer/B3DRendererManager.h"
#include "Material/B3DPass.h"
#include "Renderer/B3DRenderer.h"
#include "RTTI/B3DVariationRTTI.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Utility/B3DPersistentCache.h"

using namespace b3d;

VariationBase::VariationBase(const String& language, const ShaderVariationParameters& variationParameters)
	: mLanguage(language), mVariationParameters(variationParameters)
{
}

bool VariationBase::IsSupported() const
{
	const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();

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
	: VariationBase(StringUtil::kBlank, ShaderVariationParameters())
{}

template <bool IsRenderProxy>
SPtr<typename TVariation<IsRenderProxy>::PassType> TVariation<IsRenderProxy>::GetPass(u32 passIndex) const
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
void TVariation<IsRenderProxy>::SetCompiledPassData(TInlineArray<SPtr<PassType>, 1> compiledPasses)
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
		const SPtr<ShaderType> owner = mOwner.lock();
		if(!B3D_ENSURE(owner != nullptr))
		{
			B3D_LOG(Error, LogMaterial, "Cannot compile shader variation. Parent shader is not assigned.");
			operation.CompleteOperation(false);
			return operation;
		}

		const String& variationName = mVariationParameters.CreateVariationName();

		StringStream hashStringStream;
		hashStringStream << variationName << "\n";

		const SPtr<ShaderCompilerMetaData>& shaderCompilerMetaData = owner->GetCompilerMetaData();
		if(shaderCompilerMetaData != nullptr)
		{
			hashStringStream << shaderCompilerMetaData->Source;

			for(const auto& includeHashPair : shaderCompilerMetaData->IncludeHashes)
				hashStringStream << includeHashPair.first << " = " << StringUtil::HexToLiteral(includeHashPair.second.data(), (u32)includeHashPair.second.size()) << "\n";
		}

		const String& hashString = hashStringStream.str();
		const Array<u64, 2> variationHash = Shader::ComputeHash(hashString);

		Path cacheName;
		PersistentCache& cache = GetApplication().GetApplicationCache();

		if(shaderCompilerMetaData != nullptr && !shaderCompilerMetaData->NameInCache.empty())
		{
			cacheName = Path(shaderCompilerMetaData->NameInCache)+ mLanguage + StringUtil::HexToLiteral(variationHash.data(), (u32)variationHash.size());

			const SPtr<VariationType> cachedTechnique = cache.TryGetEntry<VariationType>(cacheName);
			if(cachedTechnique != nullptr)
				GetSelf()->SetCompiledPassData(cachedTechnique->mPasses);
		}

		if(!mHasPassData)
		{
			SPtr<IShaderCompiler> shaderCompiler = ShaderCompilers::Instance().GetCompiler("bsl");
			if(shaderCompiler == nullptr)
			{
				B3D_LOG(Error, LogMaterial, "Cannot compile variation. BSL shader compiler is not available.");
				operation.CompleteOperation(false);
				return operation;
			}

			const ShadingLanguageFlag language = ShaderCompilers::ParseShadingLanguage(mLanguage);
			const ShaderCompilerResult compileResult = shaderCompiler->CompileVariation(*owner, mVariationParameters, language, *GetSelf());

			if(!compileResult.ErrorMessage.empty())
			{
				B3D_LOG(Error, LogRenderer, "Compilation error when compiling a variation for shader \"{0}\":\n{1}. Location: {2} ({3})", owner->GetShaderName(), compileResult.ErrorMessage, compileResult.ErrorLine, compileResult.ErrorColumn);
				operation.CompleteOperation(false);
				return operation;
			}

			if(!cacheName.IsEmpty())
				cache.SetEntry(cacheName, GetSelf());

			mHasPassData = true;
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

template class TVariation<false>;
template class TVariation<true>;

Variation::Variation(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData)
	: TVariation(owner, language, variationParameters, precompiledData)
{}

Variation::Variation()
	: TVariation()
{}

SPtr<render::RenderProxy> Variation::CreateRenderProxy() const
{
	const SPtr<Shader> owner = mOwner.lock();
	const WeakSPtr<render::Shader> ownerRenderProxy = B3DGetRenderProxy(owner);

	TInlineArray<SPtr<render::Pass>, 1> passRenderProxies;
	for(auto& pass : mPasses)
		passRenderProxies.Add(B3DGetRenderProxy(pass));

	TOptional<render::PrecompiledVariationData> precompiledDataRenderProxy = mHasPassData ? render::PrecompiledVariationData(passRenderProxies) : TOptional<render::PrecompiledVariationData>{};

	render::Variation* const renderProxy = new(B3DAllocate<render::Variation>()) render::Variation(ownerRenderProxy, mLanguage, mVariationParameters, precompiledDataRenderProxy);
	const SPtr<render::Variation> renderProxyShared = B3DMakeSharedFromExisting<render::Variation>(renderProxy);
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
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(SPtr<Shader>, Owner)
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

SPtr<Variation> Variation::Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData)
{
	Variation* variation = new(B3DAllocate<Variation>()) Variation(owner, language, variationParameters, precompiledData);
	SPtr<Variation> variationShared = B3DMakeSharedFromExisting<Variation>(variation);
	variationShared->SetShared(variationShared);
	variationShared->Initialize();

	return variationShared;
}

SPtr<Variation> Variation::CreateEmpty()
{
	Variation* variation = new(B3DAllocate<Variation>()) Variation();
	SPtr<Variation> variationShared = B3DMakeSharedFromExisting<Variation>(variation);
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

namespace b3d { namespace render
{
Variation::Variation()
	: TVariation(WeakSPtr<Shader>(), StringUtil::kBlank, ShaderVariationParameters(), {})
{ }

Variation::Variation(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData)
	: TVariation(owner, language, variationParameters, precompiledData)
{}

SPtr<Variation> Variation::Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData)
{
	Variation *const variation = new(B3DAllocate<Variation>()) Variation(owner, language, variationParameters, precompiledData);
	SPtr<Variation> variationShared = B3DMakeSharedFromExisting<Variation>(variation);
	variationShared->SetShared(variationShared);
	variationShared->Initialize();

	return variationShared;
}

SPtr<Variation> Variation::CreateEmpty()
{
	Variation* const variation = new(B3DAllocate<Variation>()) Variation();
	SPtr<Variation> variationShared = B3DMakeSharedFromExisting<Variation>(variation);
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
