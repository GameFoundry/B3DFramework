//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Material/BsTechnique.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Renderer/BsRendererManager.h"
#include "Material/BsPass.h"
#include "Renderer/BsRenderer.h"
#include "RTTI/BsTechniqueRTTI.h"

namespace bs
{
	TechniqueBase::TechniqueBase(const String& language, const StringID& renderer, const Vector<StringID>& tags, 
		const ShaderVariation& variation)
		:mLanguage(language), mRenderer(renderer), mTags(tags), mVariation(variation)
	{

	}

	bool TechniqueBase::isSupported() const
	{
		if ((ct::RenderAPI::instancePtr()->getShadingLanguageName() == mLanguage || mLanguage == "Any") &&
			(RendererManager::instance().getActive()->getName() == mRenderer ||
			RendererAny == mRenderer))
		{
			return true;
		}

		return false;
	}

	bool TechniqueBase::hasTag(const StringID& tag)
	{
		for(auto& entry : mTags)
		{
			if (entry == tag)
				return true;
		}

		return false;
	}

	template<bool Core>
	TTechnique<Core>::TTechnique(const String& language, const StringID& renderer, const Vector<StringID>& tags,
		const ShaderVariation& variation, const Vector<SPtr<PassType>>& passes)
		: TechniqueBase(language, renderer, tags, variation), mPasses(passes)
	{ }

	template<bool Core>
	TTechnique<Core>::TTechnique()
		: TechniqueBase("", "", {}, ShaderVariation())
	{ }

	template<bool Core>
	SPtr<typename TTechnique<Core>::PassType> TTechnique<Core>::getPass(UINT32 idx) const
	{
		if (idx < 0 || idx >= (UINT32)mPasses.size())
			BS_EXCEPT(InvalidParametersException, "Index out of range: " + toString(idx));

		return mPasses[idx];
	}

	template class TTechnique < false > ;
	template class TTechnique < true >;

	Technique::Technique(const String& language, const StringID& renderer, const Vector<StringID>& tags,
		const ShaderVariation& variation, const Vector<SPtr<Pass>>& passes)
		:TTechnique(language, renderer, tags, variation, passes)
	{ }

	Technique::Technique()
		: TTechnique()
	{ }

	SPtr<ct::Technique> Technique::getCore() const
	{
		return std::static_pointer_cast<ct::Technique>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> Technique::createCore() const
	{
		Vector<SPtr<ct::Pass>> passes;
		for (auto& pass : mPasses)
			passes.push_back(pass->getCore());

		ct::Technique* technique = new(bs_alloc<ct::Technique>()) ct::Technique(
			mLanguage,
			mRenderer,
			mTags,
			mVariation,
			passes);

		SPtr<ct::Technique> techniquePtr = bs_shared_ptr<ct::Technique>(technique);
		techniquePtr->_setThisPtr(techniquePtr);

		return techniquePtr;
	}

	void Technique::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		for (auto& pass : mPasses)
			dependencies.push_back(pass.get());
	}

	SPtr<Technique> Technique::create(const String& language, const StringID& renderer, const Vector<SPtr<Pass>>& passes)
	{
		Technique* technique = new (bs_alloc<Technique>()) Technique(language, renderer, {}, ShaderVariation(), passes);
		SPtr<Technique> techniquePtr = bs_core_ptr<Technique>(technique);
		techniquePtr->_setThisPtr(techniquePtr);
		techniquePtr->initialize();

		return techniquePtr;
	}

	SPtr<Technique> Technique::create(const String& language, const StringID& renderer, const Vector<StringID>& tags,
		const ShaderVariation& variation, const Vector<SPtr<Pass>>& passes)
	{
		Technique* technique = new (bs_alloc<Technique>()) Technique(language, renderer, tags, variation, passes);
		SPtr<Technique> techniquePtr = bs_core_ptr<Technique>(technique);
		techniquePtr->_setThisPtr(techniquePtr);
		techniquePtr->initialize();

		return techniquePtr;
	}

	SPtr<Technique> Technique::createEmpty()
	{
		Technique* technique = new (bs_alloc<Technique>()) Technique();
		SPtr<Technique> techniquePtr = bs_core_ptr<Technique>(technique);
		techniquePtr->_setThisPtr(techniquePtr);

		return techniquePtr;
	}

	RTTITypeBase* Technique::getRTTIStatic()
	{
		return TechniqueRTTI::instance();
	}

	RTTITypeBase* Technique::getRTTI() const
	{
		return Technique::getRTTIStatic();
	}

	namespace ct
	{
	Technique::Technique(const String& language, const StringID& renderer, const Vector<StringID>& tags,
		const ShaderVariation& variation, const Vector<SPtr<Pass>>& passes)
		:TTechnique(language, renderer, tags, variation, passes)
	{ }

	SPtr<Technique> Technique::create(const String& language, const StringID& renderer,
		const Vector<SPtr<Pass>>& passes)
	{
		Technique* technique = new (bs_alloc<Technique>()) Technique(language, renderer, {}, ShaderVariation(), passes);
		SPtr<Technique> techniquePtr = bs_shared_ptr<Technique>(technique);
		techniquePtr->_setThisPtr(techniquePtr);
		techniquePtr->initialize();

		return techniquePtr;
	}

	SPtr<Technique> Technique::create(const String& language, const StringID& renderer,
		const Vector<StringID>& tags, const ShaderVariation& variation, const Vector<SPtr<Pass>>& passes)
	{
		Technique* technique = new (bs_alloc<Technique>()) Technique(language, renderer, tags, variation, passes);
		SPtr<Technique> techniquePtr = bs_shared_ptr<Technique>(technique);
		techniquePtr->_setThisPtr(techniquePtr);
		techniquePtr->initialize();

		return techniquePtr;
	}
	}
}