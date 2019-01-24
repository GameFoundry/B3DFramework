//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRenderQueue.h"
#include "RenderAPI/BsSubMesh.h"
#include "Material/BsShader.h"
#include "Mesh/BsMesh.h"
#include "Material/BsMaterial.h"
#include "Renderer/BsRenderElement.h"

using namespace std::placeholders;

namespace bs { namespace ct
{
	RenderQueue::RenderQueue(StateReduction mode)
		:mStateReductionMode(mode)
	{

	}

	void RenderQueue::clear()
	{
		mSortableElements.clear();
		mSortableElementIdx.clear();
		mElements.clear();

		mSortedRenderElements.clear();
	}

	void RenderQueue::add(const RenderElement* element, float distFromCamera, UINT32 techniqueIdx)
	{
		SPtr<Material> material = element->material;
		SPtr<Shader> shader = material->getShader();
		
		UINT32 queuePriority = shader->getQueuePriority();
		QueueSortType sortType = shader->getQueueSortType();
		UINT32 shaderId = shader->getId();
		bool separablePasses = shader->getAllowSeparablePasses();

		switch (sortType)
		{
		case QueueSortType::None:
			distFromCamera = 0;
			break;
		case QueueSortType::BackToFront:
			distFromCamera = -distFromCamera;
			break;
		case QueueSortType::FrontToBack:
			break;
		}

		UINT32 numPasses = material->getNumPasses(techniqueIdx);
		if (!separablePasses)
			numPasses = std::min(1U, numPasses);

		for (UINT32 i = 0; i < numPasses; i++)
		{
			UINT32 idx = (UINT32)mSortableElementIdx.size();
			mSortableElementIdx.push_back(idx);

			mSortableElements.push_back(SortableElement());
			SortableElement& sortableElem = mSortableElements.back();

			sortableElem.seqIdx = idx;
			sortableElem.priority = queuePriority;
			sortableElem.shaderId = shaderId;
			sortableElem.techniqueIdx = techniqueIdx;
			sortableElem.passIdx = i;
			sortableElem.distFromCamera = distFromCamera;

			mElements.push_back(element);
		}
	}

	void RenderQueue::sort()
	{
		std::function<bool(UINT32, UINT32, const Vector<SortableElement>&)> sortMethod;

		switch (mStateReductionMode)
		{
		case StateReduction::None:
			sortMethod = &elementSorterNoGroup;
			break;
		case StateReduction::Material:
			sortMethod = &elementSorterPreferGroup;
			break;
		case StateReduction::Distance:
			sortMethod = &elementSorterPreferDistance;
			break;
		}

		// Sort only indices since we generate an entirely new data set anyway, it doesn't make sense to move sortable elements
		std::sort(mSortableElementIdx.begin(), mSortableElementIdx.end(), std::bind(sortMethod, _1, _2, mSortableElements));

		UINT32 prevShaderId = (UINT32)-1;
		UINT32 prevTechniqueIdx = (UINT32)-1;
		UINT32 prevPassIdx = (UINT32)-1;
		for (UINT32 i = 0; i < (UINT32)mSortableElementIdx.size(); i++)
		{
			const UINT32 idx = mSortableElementIdx[i];
			const SortableElement& elem = mSortableElements[idx];
			const RenderElement* renderElem = mElements[idx];

			const bool separablePasses = renderElem->material->getShader()->getAllowSeparablePasses();

			if (separablePasses)
			{
				mSortedRenderElements.push_back(RenderQueueElement());

				RenderQueueElement& sortedElem = mSortedRenderElements.back();
				sortedElem.renderElem = renderElem;
				sortedElem.techniqueIdx = elem.techniqueIdx;
				sortedElem.passIdx = elem.passIdx;

				if (prevShaderId != elem.shaderId || prevTechniqueIdx != elem.techniqueIdx ||  prevPassIdx != elem.passIdx)
				{
					sortedElem.applyPass = true;
					prevShaderId = elem.shaderId;
					prevTechniqueIdx = elem.techniqueIdx;
					prevPassIdx = elem.passIdx;
				}
				else
					sortedElem.applyPass = false;
			}
			else
			{
				const UINT32 numPasses = renderElem->material->getNumPasses(elem.techniqueIdx);
				for (UINT32 j = 0; j < numPasses; j++)
				{
					mSortedRenderElements.push_back(RenderQueueElement());

					RenderQueueElement& sortedElem = mSortedRenderElements.back();
					sortedElem.renderElem = renderElem;
					sortedElem.techniqueIdx = elem.techniqueIdx;
					sortedElem.passIdx = j;

					if (prevShaderId != elem.shaderId || prevTechniqueIdx != elem.techniqueIdx || prevPassIdx != j)
					{
						sortedElem.applyPass = true;
						prevShaderId = elem.shaderId;
						prevTechniqueIdx = elem.techniqueIdx;
						prevPassIdx = j;
					}
					else
						sortedElem.applyPass = false;
				}
			}
		}
	}

	bool RenderQueue::elementSorterNoGroup(UINT32 aIdx, UINT32 bIdx, const Vector<SortableElement>& lookup)
	{
		const SortableElement& a = lookup[aIdx];
		const SortableElement& b = lookup[bIdx];

		UINT8 isHigher = (a.priority > b.priority) << 2 |
			(a.distFromCamera < b.distFromCamera) << 1 |
			(a.seqIdx < b.seqIdx);

		UINT8 isLower = (a.priority < b.priority) << 2 |
			(a.distFromCamera > b.distFromCamera) << 1 |
			(a.seqIdx > b.seqIdx);

		return isHigher > isLower;
	}

	bool RenderQueue::elementSorterPreferGroup(UINT32 aIdx, UINT32 bIdx, const Vector<SortableElement>& lookup)
	{
		const SortableElement& a = lookup[aIdx];
		const SortableElement& b = lookup[bIdx];
		
		UINT8 isHigher = (a.priority > b.priority) << 5 |
			(a.shaderId < b.shaderId) << 4 |
			(a.techniqueIdx < b.techniqueIdx) << 3 |
			(a.passIdx < b.passIdx) << 2 |
			(a.distFromCamera < b.distFromCamera) << 1 |
			(a.seqIdx < b.seqIdx);

		UINT8 isLower = (a.priority < b.priority) << 5 |
			(a.shaderId > b.shaderId) << 4 |
			(a.techniqueIdx > b.techniqueIdx) << 3 |
			(a.passIdx > b.passIdx) << 2 |
			(a.distFromCamera > b.distFromCamera) << 1 |
			(a.seqIdx > b.seqIdx);

		return isHigher > isLower;
	}

	bool RenderQueue::elementSorterPreferDistance(UINT32 aIdx, UINT32 bIdx, const Vector<SortableElement>& lookup)
	{
		const SortableElement& a = lookup[aIdx];
		const SortableElement& b = lookup[bIdx];

		UINT8 isHigher = (a.priority > b.priority) << 5 | 
			(a.distFromCamera < b.distFromCamera) << 4 | 
			(a.shaderId < b.shaderId) << 3 | 
			(a.techniqueIdx < b.techniqueIdx) << 2 |
			(a.passIdx < b.passIdx) << 1 | 
			(a.seqIdx < b.seqIdx);

		UINT8 isLower = (a.priority < b.priority) << 5 |
			(a.distFromCamera > b.distFromCamera) << 4 |
			(a.shaderId > b.shaderId) << 3 |
			(a.techniqueIdx > b.techniqueIdx) << 2 |
			(a.passIdx > b.passIdx) << 1 |
			(a.seqIdx > b.seqIdx);

		return isHigher > isLower;
	}

	const Vector<RenderQueueElement>& RenderQueue::getSortedElements() const
	{
		return mSortedRenderElements;
	}
}}