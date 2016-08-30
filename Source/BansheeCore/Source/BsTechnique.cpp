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

namespace BansheeEngine
{
	TechniqueBase::TechniqueBase(const StringID& renderAPI, const StringID& renderer, const Vector<StringID>& tags)
		:mRenderAPI(renderAPI), mRenderer(renderer), mTags(tags)
	{

	}

	bool TechniqueBase::isSupported() const
	{
		if ((RenderAPICore::instancePtr()->getName() == mRenderAPI ||
			RenderAPIAny == mRenderAPI) &&
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
	TTechnique<Core>::TTechnique(const StringID& renderAPI, const StringID& renderer, const Vector<StringID>& tags, 
		const Vector<SPtr<PassType>>& passes)
		: TechniqueBase(renderAPI, renderer, tags), mPasses(passes)
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

	TechniqueCore::TechniqueCore(const StringID& renderAPI, const StringID& renderer, const Vector<StringID>& tags, 
		const Vector<SPtr<PassCore>>& passes)
		:TTechnique(renderAPI, renderer, tags, passes)
	{ }

	SPtr<TechniqueCore> TechniqueCore::create(const StringID& renderAPI, const StringID& renderer, 
		const Vector<SPtr<PassCore>>& passes)
	{
		TechniqueCore* technique = new (bs_alloc<TechniqueCore>()) TechniqueCore(renderAPI, renderer, {}, passes);
		SPtr<TechniqueCore> techniquePtr = bs_shared_ptr<TechniqueCore>(technique);
		techniquePtr->_setThisPtr(techniquePtr);
		techniquePtr->initialize();

		return techniquePtr;
	}

	SPtr<TechniqueCore> TechniqueCore::create(const StringID& renderAPI, const StringID& renderer, 
		const Vector<StringID>& tags, const Vector<SPtr<PassCore>>& passes)
	{
		TechniqueCore* technique = new (bs_alloc<TechniqueCore>()) TechniqueCore(renderAPI, renderer, tags, passes);
		SPtr<TechniqueCore> techniquePtr = bs_shared_ptr<TechniqueCore>(technique);
		techniquePtr->_setThisPtr(techniquePtr);
		techniquePtr->initialize();

		return techniquePtr;
	}

	Technique::Technique(const StringID& renderAPI, const StringID& renderer, const Vector<StringID>& tags, 
		const Vector<SPtr<Pass>>& passes)
		:TTechnique(renderAPI, renderer, tags, passes)
	{ }

	Technique::Technique()
		: TTechnique()
	{ }

	SPtr<TechniqueCore> Technique::getCore() const
	{
		return std::static_pointer_cast<TechniqueCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> Technique::createCore() const
	{
		Vector<SPtr<PassCore>> passes;
		for (auto& pass : mPasses)
			passes.push_back(pass->getCore());

		TechniqueCore* technique = new (bs_alloc<TechniqueCore>()) TechniqueCore(mRenderAPI, mRenderer, mTags, passes);
		SPtr<TechniqueCore> techniquePtr = bs_shared_ptr<TechniqueCore>(technique);
		techniquePtr->_setThisPtr(techniquePtr);

		return techniquePtr;
	}

	void Technique::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		for (auto& pass : mPasses)
			dependencies.push_back(pass.get());
	}

	SPtr<Technique> Technique::create(const StringID& renderAPI, const StringID& renderer, const Vector<SPtr<Pass>>& passes)
	{
		Technique* technique = new (bs_alloc<Technique>()) Technique(renderAPI, renderer, {}, passes);
		SPtr<Technique> techniquePtr = bs_core_ptr<Technique>(technique);
		techniquePtr->_setThisPtr(techniquePtr);
		techniquePtr->initialize();

		return techniquePtr;
	}

	SPtr<Technique> Technique::create(const StringID& renderAPI, const StringID& renderer, const Vector<StringID>& tags, 
		const Vector<SPtr<Pass>>& passes)
	{
		Technique* technique = new (bs_alloc<Technique>()) Technique(renderAPI, renderer, tags, passes);
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
}