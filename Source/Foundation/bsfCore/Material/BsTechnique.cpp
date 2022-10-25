//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsTechnique.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Renderer/BsRendererManager.h"
#include "Material/BsPass.h"
#include "Renderer/BsRenderer.h"
#include "Managers/BsGpuProgramManager.h"
#include "Private/RTTI/BsTechniqueRTTI.h"

using namespace bs;

TechniqueBase::TechniqueBase(const String& language, const Vector<StringID>& tags, const ShaderVariation& variation)
	: mLanguage(language), mTags(tags), mVariation(variation)
{
}

bool TechniqueBase::IsSupported() const
{
	if(ct::GpuProgramManager::Instance().IsLanguageSupported(mLanguage) || mLanguage == "Any")
		return true;

	return false;
}

bool TechniqueBase::HasTag(const StringID& tag)
{
	for(auto& entry : mTags)
	{
		if(entry == tag)
			return true;
	}

	return false;
}

template <bool Core>
TTechnique<Core>::TTechnique(const String& language, const Vector<StringID>& tags, const ShaderVariation& variation, const Vector<SPtr<PassType>>& passes)
	: TechniqueBase(language, tags, variation), mPasses(passes)
{}

template <bool Core>
TTechnique<Core>::TTechnique()
	: TechniqueBase("", {}, ShaderVariation())
{}

template <bool Core>
SPtr<typename TTechnique<Core>::PassType> TTechnique<Core>::GetPass(u32 idx) const
{
	if(idx < 0 || idx >= (u32)mPasses.size())
		BS_EXCEPT(InvalidParametersException, "Index out of range: " + toString(idx));

	return mPasses[idx];
}

template <bool Core>
void TTechnique<Core>::Compile()
{
	for(auto& pass : mPasses)
		pass->Compile();
}

template class TTechnique<false>;
template class TTechnique<true>;

Technique::Technique(const String& language, const Vector<StringID>& tags, const ShaderVariation& variation, const Vector<SPtr<Pass>>& passes)
	: TTechnique(language, tags, variation, passes)
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
	Vector<SPtr<ct::Pass>> passes;
	for(auto& pass : mPasses)
		passes.push_back(pass->GetCore());

	ct::Technique* technique = new(bs_alloc<ct::Technique>()) ct::Technique(
		mLanguage,
		mTags,
		mVariation,
		passes);

	SPtr<ct::Technique> techniquePtr = bs_shared_ptr<ct::Technique>(technique);
	techniquePtr->SetThisPtrInternal(techniquePtr);

	return techniquePtr;
}

void Technique::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	for(auto& pass : mPasses)
		dependencies.push_back(pass.get());
}

SPtr<Technique> Technique::Create(const String& language, const Vector<SPtr<Pass>>& passes)
{
	Technique* technique = new(bs_alloc<Technique>()) Technique(language, {}, ShaderVariation(), passes);
	SPtr<Technique> techniquePtr = bs_core_ptr<Technique>(technique);
	techniquePtr->SetThisPtrInternal(techniquePtr);
	techniquePtr->Initialize();

	return techniquePtr;
}

SPtr<Technique> Technique::Create(const String& language, const Vector<StringID>& tags, const ShaderVariation& variation, const Vector<SPtr<Pass>>& passes)
{
	Technique* technique = new(bs_alloc<Technique>()) Technique(language, tags, variation, passes);
	SPtr<Technique> techniquePtr = bs_core_ptr<Technique>(technique);
	techniquePtr->SetThisPtrInternal(techniquePtr);
	techniquePtr->Initialize();

	return techniquePtr;
}

SPtr<Technique> Technique::CreateEmpty()
{
	Technique* technique = new(bs_alloc<Technique>()) Technique();
	SPtr<Technique> techniquePtr = bs_core_ptr<Technique>(technique);
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
Technique::Technique(const String& language, const Vector<StringID>& tags, const ShaderVariation& variation, const Vector<SPtr<Pass>>& passes)
	: TTechnique(language, tags, variation, passes)
{}

SPtr<Technique> Technique::Create(const String& language, const Vector<SPtr<Pass>>& passes)
{
	Technique* technique = new(bs_alloc<Technique>()) Technique(language, {}, ShaderVariation(), passes);
	SPtr<Technique> techniquePtr = bs_shared_ptr<Technique>(technique);
	techniquePtr->SetThisPtrInternal(techniquePtr);
	techniquePtr->Initialize();

	return techniquePtr;
}

SPtr<Technique> Technique::Create(const String& language, const Vector<StringID>& tags, const ShaderVariation& variation, const Vector<SPtr<Pass>>& passes)
{
	Technique* technique = new(bs_alloc<Technique>()) Technique(language, tags, variation, passes);
	SPtr<Technique> techniquePtr = bs_shared_ptr<Technique>(technique);
	techniquePtr->SetThisPtrInternal(techniquePtr);
	techniquePtr->Initialize();

	return techniquePtr;
}
}}
