//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTechnique.h"
#include "BsException.h"
#include "BsRenderAPI.h"
#include "BsRendererManager.h"
#include "BsPass.h"
#include "BsCoreRenderer.h"
#include "BsFrameAlloc.h"
#include "BsTechniqueRTTI.h"

namespace bs
{
	TechniqueBase::TechniqueBase(const String& language, const StringID& renderer, const Vector<StringID>& tags)
		:mLanguage(language), mRenderer(renderer), mTags(tags)
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
		const Vector<SPtr<PassType>>& passes)
		: TechniqueBase(language, renderer, tags), mPasses(passes)
	{ }

	template<bool Core>
	TTechnique<Core>::TTechnique()
		: TechniqueBase("", "", {})
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
		const Vector<SPtr<Pass>>& passes)
		:TTechnique(language, renderer, tags, passes)
	{ }

	Technique::Technique()
		: TTechnique()
	{ }

	SPtr<ct::TechniqueCore> Technique::getCore() const
	{
		return std::static_pointer_cast<ct::TechniqueCore>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> Technique::createCore() const
	{
		Vector<SPtr<ct::PassCore>> passes;
		for (auto& pass : mPasses)
			passes.push_back(pass->getCore());

		ct::TechniqueCore* technique = new (bs_alloc<ct::TechniqueCore>()) ct::TechniqueCore(mLanguage, mRenderer, mTags, passes);
		SPtr<ct::TechniqueCore> techniquePtr = bs_shared_ptr<ct::TechniqueCore>(technique);
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
		Technique* technique = new (bs_alloc<Technique>()) Technique(language, renderer, {}, passes);
		SPtr<Technique> techniquePtr = bs_core_ptr<Technique>(technique);
		techniquePtr->_setThisPtr(techniquePtr);
		techniquePtr->initialize();

		return techniquePtr;
	}

	SPtr<Technique> Technique::create(const String& language, const StringID& renderer, const Vector<StringID>& tags,
		const Vector<SPtr<Pass>>& passes)
	{
		Technique* technique = new (bs_alloc<Technique>()) Technique(language, renderer, tags, passes);
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
	TechniqueCore::TechniqueCore(const String& language, const StringID& renderer, const Vector<StringID>& tags,
		const Vector<SPtr<PassCore>>& passes)
		:TTechnique(language, renderer, tags, passes)
	{ }

	SPtr<TechniqueCore> TechniqueCore::create(const String& language, const StringID& renderer,
		const Vector<SPtr<PassCore>>& passes)
	{
		TechniqueCore* technique = new (bs_alloc<TechniqueCore>()) TechniqueCore(language, renderer, {}, passes);
		SPtr<TechniqueCore> techniquePtr = bs_shared_ptr<TechniqueCore>(technique);
		techniquePtr->_setThisPtr(techniquePtr);
		techniquePtr->initialize();

		return techniquePtr;
	}

	SPtr<TechniqueCore> TechniqueCore::create(const String& language, const StringID& renderer,
		const Vector<StringID>& tags, const Vector<SPtr<PassCore>>& passes)
	{
		TechniqueCore* technique = new (bs_alloc<TechniqueCore>()) TechniqueCore(language, renderer, tags, passes);
		SPtr<TechniqueCore> techniquePtr = bs_shared_ptr<TechniqueCore>(technique);
		techniquePtr->_setThisPtr(techniquePtr);
		techniquePtr->initialize();

		return techniquePtr;
	}
	}
}