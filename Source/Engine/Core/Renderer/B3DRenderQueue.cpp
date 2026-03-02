//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRenderQueue.h"
#include "RenderAPI/B3DSubMesh.h"
#include "Material/B3DShader.h"
#include "Mesh/B3DMesh.h"
#include "Material/B3DMaterial.h"
#include "Renderer/B3DRenderElement.h"

using namespace b3d;

namespace b3d { namespace render
{
RenderQueue::RenderQueue(StateReduction mode)
	: mStateReductionMode(mode)
{
}

void RenderQueue::Clear()
{
	mSortableElements.clear();
	mSortableElementIdx.clear();
	mElements.clear();

	mSortedRenderElements.clear();
}

void RenderQueue::Add(const RenderElement* element, float distFromCamera, u32 variationIndex)
{
	SPtr<Material> material = element->Material;
	SPtr<Shader> shader = material->GetShader();

	u32 queuePriority = shader->GetQueuePriority();
	QueueSortType sortType = shader->GetQueueSortType();
	u32 shaderId = shader->GetShaderId();
	bool separablePasses = shader->GetAllowSeparablePasses();

	switch(sortType)
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

	u32 passCount = material->GetPassCount(variationIndex);
	if(!separablePasses)
		passCount = std::min(1U, passCount);

	for(u32 i = 0; i < passCount; i++)
	{
		u32 idx = (u32)mSortableElementIdx.size();
		mSortableElementIdx.push_back(idx);

		mSortableElements.push_back(SortableElement());
		SortableElement& sortableElem = mSortableElements.back();

		sortableElem.SeqIdx = idx;
		sortableElem.Priority = queuePriority;
		sortableElem.ShaderId = shaderId;
		sortableElem.VariationIndex = variationIndex;
		sortableElem.PassIndex = i;
		sortableElem.DistFromCamera = distFromCamera;

		mElements.push_back(element);
	}
}

void RenderQueue::Sort()
{
	std::function<bool(u32, u32, const Vector<SortableElement>&)> sortMethod;

	switch(mStateReductionMode)
	{
	case StateReduction::None:
		sortMethod = &ElementSorterNoGroup;
		break;
	case StateReduction::Material:
		sortMethod = &ElementSorterPreferGroup;
		break;
	case StateReduction::Distance:
		sortMethod = &ElementSorterPreferDistance;
		break;
	}

	// Sort only indices since we generate an entirely new data set anyway, it doesn't make sense to move sortable elements
	std::sort(mSortableElementIdx.begin(), mSortableElementIdx.end(), [&sortMethod, this](u32 a, u32 b) { return sortMethod(a, b, mSortableElements); });

	u32 previousShaderId = ~0u;
	u32 previousVariationIndex = ~0u;
	u32 previousPassIndex = ~0u;
	for(u32 i = 0; i < (u32)mSortableElementIdx.size(); i++)
	{
		const u32 idx = mSortableElementIdx[i];
		const SortableElement& elem = mSortableElements[idx];
		const RenderElement* renderElem = mElements[idx];

		const bool separablePasses = renderElem->Material->GetShader()->GetAllowSeparablePasses();

		if(separablePasses)
		{
			mSortedRenderElements.push_back(RenderQueueElement());

			RenderQueueElement& sortedElem = mSortedRenderElements.back();
			sortedElem.RenderElem = renderElem;
			sortedElem.VariationIndex = elem.VariationIndex;
			sortedElem.PassIndex = elem.PassIndex;

			if(previousShaderId != elem.ShaderId || previousVariationIndex != elem.VariationIndex || previousPassIndex != elem.PassIndex)
			{
				sortedElem.ApplyPass = true;
				previousShaderId = elem.ShaderId;
				previousVariationIndex = elem.VariationIndex;
				previousPassIndex = elem.PassIndex;
			}
			else
				sortedElem.ApplyPass = false;
		}
		else
		{
			const u32 numPasses = renderElem->Material->GetPassCount(elem.VariationIndex);
			for(u32 j = 0; j < numPasses; j++)
			{
				mSortedRenderElements.push_back(RenderQueueElement());

				RenderQueueElement& sortedElem = mSortedRenderElements.back();
				sortedElem.RenderElem = renderElem;
				sortedElem.VariationIndex = elem.VariationIndex;
				sortedElem.PassIndex = j;

				if(previousShaderId != elem.ShaderId || previousVariationIndex != elem.VariationIndex || previousPassIndex != j)
				{
					sortedElem.ApplyPass = true;
					previousShaderId = elem.ShaderId;
					previousVariationIndex = elem.VariationIndex;
					previousPassIndex = j;
				}
				else
					sortedElem.ApplyPass = false;
			}
		}
	}
}

bool RenderQueue::ElementSorterNoGroup(u32 aIdx, u32 bIdx, const Vector<SortableElement>& lookup)
{
	const SortableElement& a = lookup[aIdx];
	const SortableElement& b = lookup[bIdx];

	u8 isHigher = (a.Priority > b.Priority) << 2 |
		(a.DistFromCamera < b.DistFromCamera) << 1 |
		(a.SeqIdx < b.SeqIdx);

	u8 isLower = (a.Priority < b.Priority) << 2 |
		(a.DistFromCamera > b.DistFromCamera) << 1 |
		(a.SeqIdx > b.SeqIdx);

	return isHigher > isLower;
}

bool RenderQueue::ElementSorterPreferGroup(u32 aIdx, u32 bIdx, const Vector<SortableElement>& lookup)
{
	const SortableElement& a = lookup[aIdx];
	const SortableElement& b = lookup[bIdx];

	u8 isHigher = (a.Priority > b.Priority) << 5 |
		(a.ShaderId < b.ShaderId) << 4 |
		(a.VariationIndex < b.VariationIndex) << 3 |
		(a.PassIndex < b.PassIndex) << 2 |
		(a.DistFromCamera < b.DistFromCamera) << 1 |
		(a.SeqIdx < b.SeqIdx);

	u8 isLower = (a.Priority < b.Priority) << 5 |
		(a.ShaderId > b.ShaderId) << 4 |
		(a.VariationIndex > b.VariationIndex) << 3 |
		(a.PassIndex > b.PassIndex) << 2 |
		(a.DistFromCamera > b.DistFromCamera) << 1 |
		(a.SeqIdx > b.SeqIdx);

	return isHigher > isLower;
}

bool RenderQueue::ElementSorterPreferDistance(u32 aIdx, u32 bIdx, const Vector<SortableElement>& lookup)
{
	const SortableElement& a = lookup[aIdx];
	const SortableElement& b = lookup[bIdx];

	u8 isHigher = (a.Priority > b.Priority) << 5 |
		(a.DistFromCamera < b.DistFromCamera) << 4 |
		(a.ShaderId < b.ShaderId) << 3 |
		(a.VariationIndex < b.VariationIndex) << 2 |
		(a.PassIndex < b.PassIndex) << 1 |
		(a.SeqIdx < b.SeqIdx);

	u8 isLower = (a.Priority < b.Priority) << 5 |
		(a.DistFromCamera > b.DistFromCamera) << 4 |
		(a.ShaderId > b.ShaderId) << 3 |
		(a.VariationIndex > b.VariationIndex) << 2 |
		(a.PassIndex > b.PassIndex) << 1 |
		(a.SeqIdx > b.SeqIdx);

	return isHigher > isLower;
}

const Vector<RenderQueueElement>& RenderQueue::GetSortedElements() const
{
	return mSortedRenderElements;
}
}}
