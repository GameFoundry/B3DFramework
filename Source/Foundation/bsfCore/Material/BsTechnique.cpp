//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsTechnique.h"

#include "BsCoreApplication.h"
#include "BsShaderCompiler.h"
#include "Error/BsException.h"
#include "Renderer/BsRendererManager.h"
#include "Material/BsPass.h"
#include "Renderer/BsRenderer.h"
#include "Private/RTTI/BsTechniqueRTTI.h"
#include "RenderAPI/BsGpuDevice.h"
#include "Utility/BsPersistentCache.h"

using namespace bs;

TechniqueBase::TechniqueBase(const String& language, const ShaderVariationParameters& variationParameters)
	: mLanguage(language), mVariationParameters(variationParameters)
{
}

bool TechniqueBase::IsSupported() const
{
	const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();

	if((gpuDevice != nullptr && gpuDevice->IsGpuProgramLanguageSupported(mLanguage)) || mLanguage == "Any")
		return true;

	return false;
}

template <bool Core>
TTechnique<Core>::TTechnique(const WeakSPtr<ShaderType>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<TPrecompiledVariationData<Core>>& precompiledData)
	: TechniqueBase(language, variationParameters), mOwner(owner), mPasses(precompiledData.value_or(TPrecompiledVariationData<Core>()).PrecompiledPasses), mHasPassData(precompiledData.has_value())
{ }

template <bool Core>
TTechnique<Core>::TTechnique()
	: TechniqueBase(StringUtil::kBlank, ShaderVariationParameters())
{}

template <bool Core>
SPtr<typename TTechnique<Core>::PassType> TTechnique<Core>::GetPass(u32 index) const
{
	if(!mHasPassData)
	{
		B3D_LOG(Error, Material, "Unable to retrieve shader variation pass. The variation has not been compiled.");
		return nullptr;
	}

	if(index >= (u32)mPasses.size())
	{
		B3D_LOG(Error, Material, "Unable to retrieve shader variation pass. The provided index={0} is out of range=[0, {1}].", index, mPasses.size());
		return nullptr;
	}

	return mPasses[index];
}

template <bool Core>
u32 TTechnique<Core>::GetPassCount() const
{
	if(!mHasPassData)
	{
		B3D_LOG(Error, Material, "Unable to retrieve shader variation pass count. The variation has not been compiled.");
		return 0;
	}
	
	return (u32)mPasses.size();
}

template <bool Core>
void TTechnique<Core>::SetCompiledPassData(SmallVector<SPtr<PassType>, 1> compiledPasses)
{
	mPasses = std::move(compiledPasses);
	mHasPassData = true;

	MarkCoreDirty(ShaderVariationDirtyFlag::Passes);
	SyncToCore();
}

template <bool Core>
void TTechnique<Core>::SetOwner(const WeakSPtr<ShaderType>& owner)
{
	mOwner = owner;

	MarkCoreDirty(ShaderVariationDirtyFlag::Parent);
	SyncToCore();
}

template<bool Core>
TAsyncOp<bool> TTechnique<Core>::Compile()
{
	// TODO - This should be done async, but XShaderCompiler has issues with multiple threads. Additionally the pass compile needs to be made thread safe on the Vulkan level.
	TAsyncOp<bool> operation;

	if(!mHasPassData)
	{
		const SPtr<ShaderType> owner = mOwner.lock();
		if(!B3D_ENSURE(owner != nullptr))
		{
			B3D_LOG(Error, Material, "Cannot compile shader variation. Parent shader is not assigned.");
			operation.CompleteOperation(false);
			return operation;
		}

		const String& variationName = mVariationParameters.CreateVariationName();

		StringStream hashStringStream;
		hashStringStream << variationName << "\n";

		const SPtr<ShaderCompilerMetaData>& shaderCompilerMetaData = owner->GetCompilerMetaData();
		if(shaderCompilerMetaData != nullptr)
		{
			for(const auto& cachedIncludePair : shaderCompilerMetaData->IncludeHashes)
				hashStringStream << cachedIncludePair.first << " = " << StringUtil::HexToLiteral(cachedIncludePair.second.data(), (u32)cachedIncludePair.second.size()) << "\n";
		}

		const String& hashString = hashStringStream.str();
		const Array<u64, 2> variationHash = Shader::ComputeHash(hashString);

		Path cacheName;
		PersistentCache& cache = GetCoreApplication().GetApplicationCache();

		if(shaderCompilerMetaData != nullptr && !shaderCompilerMetaData->NameInCache.empty())
		{
			cacheName = Path(shaderCompilerMetaData->NameInCache)+ mLanguage + StringUtil::HexToLiteral(variationHash.data(), (u32)variationHash.size());

			const SPtr<TechniqueType> cachedTechnique = cache.TryGetEntry<TechniqueType>(cacheName);
			if(cachedTechnique != nullptr)
				GetSelf()->SetCompiledPassData(cachedTechnique->mPasses);
		}

		if(!mHasPassData)
		{
			SPtr<IShaderCompiler> shaderCompiler = ShaderCompilers::Instance().GetCompiler("bsl");
			if(shaderCompiler == nullptr)
			{
				B3D_LOG(Error, Material, "Cannot compile variation. BSL shader compiler is not available.");
				operation.CompleteOperation(false);
				return operation;
			}

			const ShadingLanguageFlag language = ShaderCompilers::ParseShadingLanguage(mLanguage);
			const ShaderCompilerResult compileResult = shaderCompiler->CompileVariation(*owner, mVariationParameters, language, *GetSelf());

			if(!compileResult.ErrorMessage.empty())
			{
				B3D_LOG(Error, Renderer, "Compilation error when compiling a variation for shader \"{0}\":\n{1}. Location: {2} ({3})", owner->GetShaderName(), compileResult.ErrorMessage, compileResult.ErrorLine, compileResult.ErrorColumn);
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

template class TTechnique<false>;
template class TTechnique<true>;

Technique::Technique(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData)
	: TTechnique(owner, language, variationParameters, precompiledData)
{}

Technique::Technique()
	: TTechnique()
{}

SPtr<ct::Technique> Technique::GetCore() const
{
	return std::static_pointer_cast<ct::Technique>(mCoreSpecific);
}

SPtr<ct::CoreObject> Technique::CreateCore() const
{
	const SPtr<Shader> owner = mOwner.lock();
	const WeakSPtr<ct::Shader> coreOwner = owner != nullptr ? owner->GetCore() : nullptr;

	SmallVector<SPtr<ct::Pass>, 1> corePasses;
	for(auto& pass : mPasses)
		corePasses.Add(pass->GetCore());

	Optional<ct::PrecompiledVariationData> corePrecompileData = mHasPassData ? ct::PrecompiledVariationData(corePasses) : Optional<ct::PrecompiledVariationData>{};
	ct::Technique* const coreVariation = new(B3DAllocate<ct::Technique>()) ct::Technique(coreOwner, mLanguage, mVariationParameters, corePrecompileData);
	const SPtr<ct::Technique> coreVariationShared = B3DMakeSharedFromExisting<ct::Technique>(coreVariation);
	coreVariationShared->SetShared(coreVariationShared);

	return coreVariationShared;
}

void Technique::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	for(auto& pass : mPasses)
		dependencies.push_back(pass.get());
}

void Technique::MarkCoreDirty(ShaderVariationDirtyFlags flags)
{
	CoreObject::MarkCoreDirty(flags);
}

void Technique::SyncToCore()
{
	CoreObject::SyncToCore();
}

CoreSyncData Technique::SyncToCore(FrameAlloc* allocator)
{
	const ShaderVariationDirtyFlags dirtyFlags = (ShaderVariationDirtyFlags)GetCoreDirtyFlags();

	u32 size = sizeof(u32);
	if(dirtyFlags.IsSet(ShaderVariationDirtyFlag::Parent))
		size += sizeof(SPtr<ct::Shader>);

	const u32 passCount = (u32)mPasses.size();
	if(dirtyFlags.IsSet(ShaderVariationDirtyFlag::Passes))
		size += sizeof(SPtr<ct::Pass>) * passCount + sizeof(passCount) + sizeof(mHasPassData);

	u8* buffer = allocator->Alloc(size);
	Bitstream stream(buffer, size);

	B3DRTTIWrite((u32)dirtyFlags, stream);

	if(dirtyFlags.IsSet(ShaderVariationDirtyFlag::Parent))
	{
		SPtr<ct::Shader> *const coreOwner = new(stream.Cursor()) SPtr<ct::Shader>();
		const SPtr<Shader> owner = mOwner.lock();
		if(owner != nullptr)
			*coreOwner = owner->GetCore();
		else
			*coreOwner = nullptr;

		stream.SkipBytes(sizeof(SPtr<ct::Shader>));
	}

	if(dirtyFlags.IsSet(ShaderVariationDirtyFlag::Passes))
	{
		B3DRTTIWrite(mHasPassData, stream);
		B3DRTTIWrite((u32)mPasses.size(), stream);

		for(u32 passIndex = 0; passIndex < passCount; passIndex++)
		{
			SPtr<ct::Pass> *const corePass = new(stream.Cursor()) SPtr<ct::Pass>();
			*corePass = mPasses[passIndex] != nullptr ? mPasses[passIndex]->GetCore() : nullptr;

			stream.SkipBytes(sizeof(SPtr<ct::Pass>));
		}
	}

	return CoreSyncData(buffer, size);
}


SPtr<Technique> Technique::Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData)
{
	Technique* technique = new(B3DAllocate<Technique>()) Technique(owner, language, variationParameters, precompiledData);
	SPtr<Technique> techniquePtr = B3DMakeCoreFromExisting<Technique>(technique);
	techniquePtr->SetShared(techniquePtr);
	techniquePtr->Initialize();

	return techniquePtr;
}

SPtr<Technique> Technique::CreateEmpty()
{
	Technique* technique = new(B3DAllocate<Technique>()) Technique();
	SPtr<Technique> techniquePtr = B3DMakeCoreFromExisting<Technique>(technique);
	techniquePtr->SetShared(techniquePtr);

	return techniquePtr;
}

RTTITypeBase* Technique::GetRttiStatic()
{
	return TechniqueRTTI::Instance();
}

RTTITypeBase* Technique::GetRtti() const
{
	return Technique::GetRttiStatic();
}

namespace bs { namespace ct
{
Technique::Technique()
	: TTechnique(WeakSPtr<Shader>(), StringUtil::kBlank, ShaderVariationParameters(), {})
{ }

Technique::Technique(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData)
	: TTechnique(owner, language, variationParameters, precompiledData)
{}

SPtr<Technique> Technique::Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData)
{
	Technique *const technique = new(B3DAllocate<Technique>()) Technique(owner, language, variationParameters, precompiledData);
	SPtr<Technique> techniqueShared = B3DMakeSharedFromExisting<Technique>(technique);
	techniqueShared->SetShared(techniqueShared);
	techniqueShared->Initialize();

	return techniqueShared;
}

SPtr<Technique> Technique::CreateEmpty()
{
	Technique* const technique = new(B3DAllocate<Technique>()) Technique();
	SPtr<Technique> techniqueShared = B3DMakeSharedFromExisting<Technique>(technique);
	techniqueShared->SetShared(techniqueShared);
	techniqueShared->Initialize();

	return techniqueShared;
}

void Technique::SyncToCore(const CoreSyncData& data, FrameAlloc& allocator)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());

	u32 dirtyFlagBits;
	B3DRTTIRead(dirtyFlagBits, stream);

	const ShaderVariationDirtyFlags dirtyFlags = (ShaderVariationDirtyFlags)dirtyFlagBits;

	if(dirtyFlags.IsSet(ShaderVariationDirtyFlag::Parent))
	{
		SPtr<Shader>* const owner = (SPtr<Shader>*)stream.Cursor();
		B3D_ASSERT(owner != nullptr);

		mOwner = *owner;

		owner->~SPtr<Shader>();
		stream.SkipBytes(sizeof(SPtr<Shader>));
	}

	if(dirtyFlags.IsSet(ShaderVariationDirtyFlag::Passes))
	{
		B3DRTTIRead(mHasPassData, stream);

		u32 passCount;
		B3DRTTIRead(passCount, stream);

		mPasses.resize(passCount);
		for(u32 passIndex = 0; passIndex < passCount; passIndex++)
		{
			SPtr<Pass>* const sourcePass = (SPtr<Pass>*)stream.Cursor();
			B3D_ASSERT(sourcePass != nullptr);

			mPasses[passIndex] = *sourcePass;

			sourcePass->~SPtr<Pass>();
			stream.SkipBytes(sizeof(SPtr<Pass>));
		}
	}
}

RTTITypeBase* Technique::GetRttiStatic()
{
	return CoreTechniqueRTTI::Instance();
}

RTTITypeBase* Technique::GetRtti() const
{
	return Technique::GetRttiStatic();
}
}}
