//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsTechnique.h"

#include "BsShaderCompiler.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Renderer/BsRendererManager.h"
#include "Material/BsPass.h"
#include "Renderer/BsRenderer.h"
#include "Managers/BsGpuProgramManager.h"
#include "Private/RTTI/BsTechniqueRTTI.h"

using namespace bs;

TechniqueBase::TechniqueBase(const String& language, const ShaderVariationParameters& variationParameters)
	: mLanguage(language), mVariationParameters(variationParameters)
{
}

bool TechniqueBase::IsSupported() const
{
	if(ct::GpuProgramManager::Instance().IsLanguageSupported(mLanguage) || mLanguage == "Any")
		return true;

	return false;
}

template <bool Core>
TTechnique<Core>::TTechnique(const WeakSPtr<ShaderType>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<TPrecompiledVariationData<Core>>& precompiledData)
	: TechniqueBase(language, variationParameters), mOwner(owner), mPasses(precompiledData.value_or(TPrecompiledVariationData<Core>()).PrecompiledPasses), mHasPassData(precompiledData.has_value()), mIsCompiled(precompiledData.has_value())
{ }

template <bool Core>
TTechnique<Core>::TTechnique()
	: TechniqueBase(StringUtil::kBlank, ShaderVariationParameters())
{}

template <bool Core>
SPtr<typename TTechnique<Core>::PassType> TTechnique<Core>::GetPass(u32 index) const
{
	if(!mIsCompiled)
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
	if(!mIsCompiled)
	{
		B3D_LOG(Error, Material, "Unable to retrieve shader variation pass count. The variation has not been compiled.");
		return 0;
	}
	
	return mPasses.size();
}

template <bool Core>
void TTechnique<Core>::SetCompiledPassData(SmallVector<SPtr<PassType>, 1> compiledPasses)
{
	mPasses = std::move(compiledPasses);
	mIsCompiled = true;
}

template class TTechnique<false>;
template class TTechnique<true>;

Technique::Technique(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData)
	: TTechnique(owner, language, variationParameters, precompiledData)
{}

Technique::Technique()
	: TTechnique()
{}

template<bool Core>
void TTechnique<Core>::Compile()
{
	if(!mHasPassData)
	{
		SPtr<IShaderCompiler> shaderCompiler = ShaderCompilers::Instance().GetCompiler("bsl");
		if(shaderCompiler == nullptr)
		{
			B3D_LOG(Error, Material, "Cannot compile variation. BSL shader compiler is not available.");
			return;
		}

		const SPtr<ShaderType> owner = mOwner.lock();
		B3D_ASSERT(owner != nullptr);

		const ShadingLanguageFlag language = ShaderCompilers::ParseShadingLanguage(mLanguage);
		const ShaderCompilerResult compileResult = shaderCompiler->CompileVariation(*owner, mVariationParameters, language, GetSelf());

		if(!compileResult.ErrorMessage.empty())
		{
			B3D_LOG(Error, Renderer, "Compilation error when compiling a variation for shader \"{0}\":\n{1}. Location: {2} ({3})", owner->GetShaderName(), compileResult.ErrorMessage, compileResult.ErrorLine, compileResult.ErrorColumn);
			return;
		}

		mHasPassData = true;
	}

	if(!mIsCompiled)
	{
		for(auto& pass : mPasses)
		{
			pass->Compile();
		}

		mIsCompiled = true;
	}
}

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

	ct::Technique *const coreVariation = new(B3DAllocate<ct::Technique>()) ct::Technique(coreOwner, mLanguage, mVariationParameters, TPrecompiledVariationData<true>(corePasses));
	const SPtr<ct::Technique> coreVariationShared = B3DMakeSharedFromExisting<ct::Technique>(coreVariation);
	coreVariationShared->SetShared(coreVariationShared);

	return coreVariationShared;
}

void Technique::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	for(auto& pass : mPasses)
		dependencies.push_back(pass.get());
}

SPtr<Technique> Technique::Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData)
{
	Technique* technique = new(B3DAllocate<Technique>()) Technique(owner, language, variationParameters, precompiledData);
	SPtr<Technique> techniquePtr = B3DMakeCoreFromExisting<Technique>(technique);
	techniquePtr->SetThisPtrInternal(techniquePtr);
	techniquePtr->Initialize();

	return techniquePtr;
}

SPtr<Technique> Technique::CreateEmpty()
{
	Technique* technique = new(B3DAllocate<Technique>()) Technique();
	SPtr<Technique> techniquePtr = B3DMakeCoreFromExisting<Technique>(technique);
	techniquePtr->SetThisPtrInternal(techniquePtr);

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
Technique::Technique(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData)
	: TTechnique(owner, language, variationParameters, precompiledData)
{}

SPtr<Technique> Technique::Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData)
{
	Technique *const technique = new(B3DAllocate<Technique>()) Technique(owner, language, variationParameters, precompiledData);
	const SPtr<Technique> techniqueShared = B3DMakeSharedFromExisting<Technique>(technique);
	techniqueShared->SetShared(techniqueShared);
	techniqueShared->Initialize();

	return techniqueShared;
}
}}
