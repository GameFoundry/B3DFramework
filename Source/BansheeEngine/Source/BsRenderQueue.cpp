//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderQueue.h"
#include "BsSubMesh.h"
#include "BsShader.h"
#include "BsMesh.h"
#include "BsMaterial.h"
#include "BsRenderableElement.h"

using namespace std::placeholders;

namespace bs
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

	void RenderQueue::add(RenderableElement* element, float distFromCamera)
	{
		SPtr<MaterialCore> material = element->material;
		SPtr<ShaderCore> shader = material->getShader();

		mElements.push_back(element);
		
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

		UINT32 numPasses = material->getNumPasses();
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
			sortableElem.passIdx = i;
			sortableElem.distFromCamera = distFromCamera;
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
			sortMethod = &elementSorterPreferSort;
			break;
		}

		// Sort only indices since we generate an entirely new data set anyway, it doesn't make sense to move sortable elements
		std::sort(mSortableElementIdx.begin(), mSortableElementIdx.end(), std::bind(sortMethod, _1, _2, mSortableElements));

		UINT32 prevShaderId = (UINT32)-1;
		UINT32 prevPassIdx = (UINT32)-1;
		RenderableElement* renderElem = nullptr;
		INT32 currentElementIdx = -1;
		UINT32 numPassesInCurrentElement = 0;
		bool separablePasses = true;
		for (UINT32 i = 0; i < (UINT32)mSortableElementIdx.size(); i++)
		{
			UINT32 idx = mSortableElementIdx[i];

			while (numPassesInCurrentElement == 0)
			{
				currentElementIdx++;
				renderElem = mElements[currentElementIdx];
				numPassesInCurrentElement = renderElem->material->getNumPasses();
				separablePasses = renderElem->material->getShader()->getAllowSeparablePasses();
			}

			const SortableElement& elem = mSortableElements[idx];
			if (separablePasses)
			{
				mSortedRenderElements.push_back(RenderQueueElement());

				RenderQueueElement& sortedElem = mSortedRenderElements.back();
				sortedElem.renderElem = renderElem;
				sortedElem.passIdx = elem.passIdx;

				if (prevShaderId != elem.shaderId || prevPassIdx != elem.passIdx)
				{
					sortedElem.applyPass = true;
					prevShaderId = elem.shaderId;
					prevPassIdx = elem.passIdx;
				}
				else
					sortedElem.applyPass = false;

				numPassesInCurrentElement--;
			}
			else
			{
				for (UINT32 j = 0; j < numPassesInCurrentElement; j++)
				{
					mSortedRenderElements.push_back(RenderQueueElement());

					RenderQueueElement& sortedElem = mSortedRenderElements.back();
					sortedElem.renderElem = renderElem;
					sortedElem.passIdx = j;
					sortedElem.applyPass = true;

					prevShaderId = elem.shaderId;
					prevPassIdx = j;
				}

				numPassesInCurrentElement = 0;
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
		
		UINT8 isHigher = (a.priority > b.priority) << 4 |
			(a.shaderId < b.shaderId) << 3 |
			(a.passIdx < b.passIdx) << 2 |
			(a.distFromCamera < b.distFromCamera) << 1 |
			(a.seqIdx < b.seqIdx);

		UINT8 isLower = (a.priority < b.priority) << 4 |
			(a.shaderId > b.shaderId) << 3 |
			(a.passIdx > b.passIdx) << 2 |
			(a.distFromCamera > b.distFromCamera) << 1 |
			(a.seqIdx > b.seqIdx);

		return isHigher > isLower;
	}

	bool RenderQueue::elementSorterPreferSort(UINT32 aIdx, UINT32 bIdx, const Vector<SortableElement>& lookup)
	{
		const SortableElement& a = lookup[aIdx];
		const SortableElement& b = lookup[bIdx];

		UINT8 isHigher = (a.priority > b.priority) << 4 | 
			(a.distFromCamera < b.distFromCamera) << 3 | 
			(a.shaderId < b.shaderId) << 2 | 
			(a.passIdx < b.passIdx) << 1 | 
			(a.seqIdx < b.seqIdx);

		UINT8 isLower = (a.priority < b.priority) << 4 |
			(a.distFromCamera > b.distFromCamera) << 3 |
			(a.shaderId > b.shaderId) << 2 |
			(a.passIdx > b.passIdx) << 1 |
			(a.seqIdx > b.seqIdx);

		return isHigher > isLower;
	}

	const Vector<RenderQueueElement>& RenderQueue::getSortedElements() const
	{
		return mSortedRenderElements;
	}
}