//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsTextureAtlasLayout.h"
#include "Debug/BsDebug.h"
#include "Utility/BsBitwise.h"

using namespace bs;

bool StaticTextureAtlasLayout::AddElement(u32 width, u32 height, u32& x, u32& y)
{
	if(width == 0 || height == 0)
	{
		x = 0;
		y = 0;
		return true;
	}

	// Try adding without expanding, if that fails try to expand
	if(!AddToNode(0, width, height, x, y, false))
	{
		if(!AddToNode(0, width, height, x, y, true))
			return false;
	}

	// Update size to cover all nodes
	if(mPow2)
	{
		mWidth = std::max(mWidth, Bitwise::NextPow2(x + width));
		mHeight = std::max(mHeight, Bitwise::NextPow2(y + height));
	}
	else
	{
		mWidth = std::max(mWidth, x + width);
		mHeight = std::max(mHeight, y + height);
	}

	return true;
}

void StaticTextureAtlasLayout::Clear()
{
	mNodes.clear();
	mNodes.push_back(TexAtlasNode(0, 0, mWidth, mHeight));

	mWidth = mInitialWidth;
	mHeight = mInitialHeight;
}

bool StaticTextureAtlasLayout::AddToNode(u32 nodeIdx, u32 width, u32 height, u32& x, u32& y, bool allowGrowth)
{
	TexAtlasNode* node = &mNodes[nodeIdx];
	float aspect = node->Width / (float)node->Height;

	if(node->Children[0] != (u32)-1)
	{
		if(AddToNode(node->Children[0], width, height, x, y, allowGrowth))
			return true;

		return AddToNode(node->Children[1], width, height, x, y, allowGrowth);
	}
	else
	{
		if(node->NodeFull)
			return false;

		if(width > node->Width || height > node->Height)
			return false;

		if(!allowGrowth)
		{
			if(node->X + width > mWidth || node->Y + height > mHeight)
				return false;
		}

		if(width == node->Width && height == node->Height)
		{
			x = node->X;
			y = node->Y;
			node->NodeFull = true;

			return true;
		}

		float dw = (float)(node->Width - width);
		float dh = (node->Height - height) * aspect;

		u32 nextChildIdx = (u32)mNodes.size();
		node->Children[0] = nextChildIdx;
		node->Children[1] = nextChildIdx + 1;

		TexAtlasNode nodeCopy = *node;
		node = nullptr; // Undefined past this point
		if(dw > dh)
		{
			mNodes.emplace_back(nodeCopy.X, nodeCopy.Y, width, nodeCopy.Height);
			mNodes.emplace_back(nodeCopy.X + width, nodeCopy.Y, nodeCopy.Width - width, nodeCopy.Height);
		}
		else
		{
			mNodes.emplace_back(nodeCopy.X, nodeCopy.Y, nodeCopy.Width, height);
			mNodes.emplace_back(nodeCopy.X, nodeCopy.Y + height, nodeCopy.Width, nodeCopy.Height - height);
		}

		return AddToNode(nodeCopy.Children[0], width, height, x, y, allowGrowth);
	}
}

Vector<TextureAtlasUtility::Page> TextureAtlasUtility::CreateAtlasLayout(Vector<Element>& elements, u32 width, u32 height, u32 maxWidth, u32 maxHeight, bool pow2)
{
	for(size_t i = 0; i < elements.size(); i++)
	{
		elements[i].Output.Idx = (u32)i; // Preserve original index before sorting
		elements[i].Output.Page = -1;
	}

	std::sort(elements.begin(), elements.end(), [](const Element& a, const Element& b)
			  { return a.Input.Width * a.Input.Height > b.Input.Width * b.Input.Height; });

	Vector<StaticTextureAtlasLayout> layouts;
	u32 remainingCount = (u32)elements.size();
	while(remainingCount > 0)
	{
		layouts.push_back(StaticTextureAtlasLayout(width, height, maxWidth, maxHeight, pow2));
		StaticTextureAtlasLayout& curLayout = layouts.back();

		// Find largest unassigned element that fits
		u32 sizeLimit = std::numeric_limits<u32>::max();
		while(true)
		{
			u32 largestId = -1;

			// Assumes elements are sorted from largest to smallest
			for(u32 i = 0; i < (u32)elements.size(); i++)
			{
				if(elements[i].Output.Page == -1)
				{
					u32 size = elements[i].Input.Width * elements[i].Input.Height;
					if(size < sizeLimit)
					{
						largestId = i;
						break;
					}
				}
			}

			if(largestId == (u32)-1)
				break; // Nothing fits, start a new page

			Element& element = elements[largestId];

			// Check if an element is too large to ever fit
			if(element.Input.Width > maxWidth || element.Input.Height > maxHeight)
			{
				B3D_LOG(Warning, Generic, "Some of the provided elements don't fit in an atlas of provided size. "
										 "Returning empty array of pages.");
				return Vector<Page>();
			}

			if(curLayout.AddElement(element.Input.Width, element.Input.Height, element.Output.X, element.Output.Y))
			{
				element.Output.Page = (u32)layouts.size() - 1;
				remainingCount--;
			}
			else
				sizeLimit = element.Input.Width * element.Input.Height;
		}
	}

	Vector<Page> pages;
	for(auto& layout : layouts)
		pages.push_back({ layout.GetWidth(), layout.GetHeight() });

	return pages;
}

// Based on: https://github.com/nical/guillotiere
TreeTextureAtlasLayout::TreeTextureAtlasLayout(const TreeTextureAtlasLayoutSettings& settings)
	: mSettings(settings)
{
	B3D_ENSURE(settings.SmallSizeLimit < settings.LargeSizeLimit);
	B3D_ENSURE(settings.Size.Width > 0 && settings.Size.Height > 0);
	B3D_ENSURE(settings.Alignment.Width > 0 && settings.Alignment.Height > 0);
	B3D_ENSURE(settings.Alignment.Width <= settings.Size.Width);
	B3D_ENSURE(settings.Alignment.Height <= settings.Size.Height);
}

Optional<TreeTextureAtlasLayout::Allocation> TreeTextureAtlasLayout::AddElement(const Size2UI& size)
{
	if(size.Width == 0 || size.Height == 0)
		return {};

	const Size2UI alignedSize = AlignSize(size);
	if(alignedSize.Width > mSettings.Size.Width || alignedSize.Height > mSettings.Size.Height)
		return {};

	u32 bestFreeNodeId = ~0u;
	Page* freePage = nullptr;

	for(auto& page : mPages)
	{
		bestFreeNodeId = FindBestFreeNode(page, alignedSize);
		if(bestFreeNodeId != ~0u)
		{
			freePage = &page;
			break;
		}
	}

	if(freePage == nullptr)
	{
		const u32 pageCount = (u32)mPages.size();
		if(pageCount == mSettings.MaximumPageCount)
			return {}; // No more room

		mPages.Add(AllocatePage());
		freePage = &mPages.back();
		bestFreeNodeId = FindBestFreeNode(*freePage, alignedSize);
	}

	if(bestFreeNodeId == ~0u)
		return {};

	Node& bestFreeNode = mNodes[bestFreeNodeId]; 

	const Rect2I allocatedArea(bestFreeNode.Area.X, bestFreeNode.Area.Y, alignedSize.Width, alignedSize.Height);
	const NodeSplitResult splitResult = Split(bestFreeNode, alignedSize);
	const NodeOrientation bestFreeNodeFlippedOrientation = bestFreeNode.Orientation == NodeOrientation::Horizontal ? NodeOrientation::Vertical : NodeOrientation::Horizontal;

	u32 allocatedNodeId = ~0u;
	u32 smallerLeftoverNodeId = ~0u;
	u32 largerLeftoverNodeId = ~0u;

	if(splitResult.LargerAreaOrientation == bestFreeNode.Orientation)
	{
		// Add larger node as sibling to the best fit node
		if(splitResult.LargerLeftoverArea != Rect2I::kEmpty)
		{
			const u32 nextSiblingId = bestFreeNode.NextSiblingId;
			largerLeftoverNodeId = AllocateNode();

			Node& largerLeftoverNode = mNodes[largerLeftoverNodeId];
			largerLeftoverNode.ParentNodeId = bestFreeNode.ParentNodeId;
			largerLeftoverNode.NextSiblingId = nextSiblingId;
			largerLeftoverNode.PreviousSiblingId = bestFreeNodeId;
			largerLeftoverNode.Area = splitResult.LargerLeftoverArea;
			largerLeftoverNode.State = NodeState::Free;
			largerLeftoverNode.Orientation = bestFreeNode.Orientation;

			bestFreeNode.NextSiblingId = largerLeftoverNodeId;
			if(nextSiblingId != ~0u)
				mNodes[nextSiblingId].PreviousSiblingId = largerLeftoverNodeId;
		}

		// Split best fit node
		if(splitResult.SmallerLeftoverArea != Rect2I::kEmpty)
		{
			bestFreeNode.State = NodeState::Container;

			allocatedNodeId = AllocateNode();
			smallerLeftoverNodeId = AllocateNode();

			Node& allocatedNode = mNodes[allocatedNodeId];
			allocatedNode.ParentNodeId = bestFreeNodeId;
			allocatedNode.NextSiblingId = smallerLeftoverNodeId;
			allocatedNode.Area = allocatedArea;
			allocatedNode.State = NodeState::Allocated;
			allocatedNode.Orientation = bestFreeNodeFlippedOrientation;

			Node& smallerLeftoverNode = mNodes[smallerLeftoverNodeId];
			smallerLeftoverNode.ParentNodeId = bestFreeNodeId;
			smallerLeftoverNode.PreviousSiblingId = allocatedNodeId;
			smallerLeftoverNode.Area = splitResult.SmallerLeftoverArea;
			smallerLeftoverNode.State = NodeState::Free;
			smallerLeftoverNode.Orientation = bestFreeNodeFlippedOrientation;
			
		}
		// Allocated area fits in the best fit node with no leftover
		else
		{
			allocatedNodeId = bestFreeNodeId;
			bestFreeNode.State = NodeState::Allocated;
			bestFreeNode.Area = allocatedArea;
		}
	}
	else
	{
		bestFreeNode.State = NodeState::Container;

		if(splitResult.LargerLeftoverArea != Rect2I::kEmpty)
		{
			largerLeftoverNodeId = AllocateNode();

			Node& largerLeftoverNode = mNodes[largerLeftoverNodeId];
			largerLeftoverNode.ParentNodeId = bestFreeNodeId;
			largerLeftoverNode.Area = splitResult.LargerLeftoverArea;
			largerLeftoverNode.State = NodeState::Free;
			largerLeftoverNode.Orientation = bestFreeNodeFlippedOrientation;
		}

		if(splitResult.SmallerLeftoverArea != Rect2I::kEmpty)
		{
			const u32 containerNodeId = AllocateNode();
			allocatedNodeId = AllocateNode();
			smallerLeftoverNodeId = AllocateNode();

			Node& containerNode = mNodes[containerNodeId];
			containerNode.ParentNodeId = bestFreeNodeId;
			containerNode.PreviousSiblingId = largerLeftoverNodeId;
			containerNode.State = NodeState::Container;
			containerNode.Orientation = bestFreeNodeFlippedOrientation;

			if(largerLeftoverNodeId != ~0u)
				mNodes[largerLeftoverNodeId].NextSiblingId = containerNodeId;

			Node& allocatedNode = mNodes[allocatedNodeId];
			allocatedNode.ParentNodeId = containerNodeId;
			allocatedNode.NextSiblingId = smallerLeftoverNodeId;
			allocatedNode.Area = allocatedArea;
			allocatedNode.State = NodeState::Allocated;
			allocatedNode.Orientation = bestFreeNode.Orientation;

			Node& smallerLeftoverNode = mNodes[smallerLeftoverNodeId];
			smallerLeftoverNode.ParentNodeId = containerNodeId;
			smallerLeftoverNode.PreviousSiblingId = allocatedNodeId;
			smallerLeftoverNode.Area = splitResult.SmallerLeftoverArea;
			smallerLeftoverNode.State = NodeState::Free;
			smallerLeftoverNode.Orientation = bestFreeNode.Orientation;
		}
		else
		{
			allocatedNodeId = AllocateNode();
			
			Node& allocatedNode = mNodes[allocatedNodeId];
			allocatedNode.ParentNodeId = bestFreeNodeId;
			allocatedNode.NextSiblingId = largerLeftoverNodeId;
			allocatedNode.Area = allocatedArea;
			allocatedNode.State = NodeState::Allocated;
			allocatedNode.Orientation = bestFreeNodeFlippedOrientation;

			if(largerLeftoverNodeId != ~0u)
				mNodes[largerLeftoverNodeId].PreviousSiblingId = allocatedNodeId;
		}
	}

	if(smallerLeftoverNodeId != ~0u)
		RegisterFreeNode(*freePage, smallerLeftoverNodeId, Size2UI(splitResult.SmallerLeftoverArea.Width, splitResult.SmallerLeftoverArea.Height));

	if(largerLeftoverNodeId != ~0u)
		RegisterFreeNode(*freePage, largerLeftoverNodeId, Size2UI(splitResult.LargerLeftoverArea.Width, splitResult.LargerLeftoverArea.Height));

	Allocation output;
	output.NodeId = allocatedNodeId;
	output.Position = Vector2I(bestFreeNode.Area.X, bestFreeNode.Area.Y);

	return output;
}

void TreeTextureAtlasLayout::RemoveElement(u32 pageId, u32 nodeId)
{
	if(!B3D_ENSURE(pageId < (u32)mPages.size()))
		return;

	if(!B3D_ENSURE(nodeId < (u32)mNodes.size()))
		return;

	Page& page = mPages[pageId];
	Node& nodeToFree = mNodes[nodeId];
	B3D_ENSURE(nodeToFree.State == NodeState::Allocated);

	nodeToFree.State = NodeState::Free;

	u32 currentNodeId = nodeId;
	while(true)
	{
		{
			Node& currentNode = mNodes[currentNodeId];

			if(currentNode.NextSiblingId != ~0u)
				MergeWithNextSibling(currentNodeId);

			const u32 previousSiblingId = currentNode.PreviousSiblingId;
			if(previousSiblingId != ~0u)
			{
				MergeWithNextSibling(previousSiblingId);
				currentNodeId = previousSiblingId;
			}
		}

		{
			Node& currentNode = mNodes[currentNodeId];
			const u32 parentNodeId = currentNode.ParentNodeId;
			if(currentNode.PreviousSiblingId == ~0u && currentNode.NextSiblingId == ~0u && parentNodeId != ~0u)
			{
				Node& parentNode = mNodes[parentNodeId];
				B3D_ENSURE(parentNode.State == NodeState::Container);

				parentNode.Area = currentNode.Area;
				parentNode.State = NodeState::Free;

				FreeNode(currentNodeId);
				currentNodeId = parentNodeId;
			}
			else
			{
				const Size2UI freedArea(currentNode.Area.Width, currentNode.Area.Height);
				RegisterFreeNode(page, currentNodeId, freedArea);
				break;
			}
		}
	}

	// Free page if empty
	u32 currentPageId = pageId;
	while(currentPageId == (u32)(mPages.size() - 1)) // Due to page IDs being indices, we can only free pages from the back
	{
		Page& currentPage = mPages[currentPageId];
		Node& rootNode = mNodes[currentPage.RootNodeId];
		if(rootNode.State != NodeState::Free || rootNode.Area.Width != mSettings.Size.Width || rootNode.Area.Height != mSettings.Size.Height)
			break;

		FreePage(currentPageId);
		mPages.Pop();

		if(currentPageId == 0)
			break;

		--currentPageId;
	}
}

void TreeTextureAtlasLayout::Grow(const Size2UI& newSize)
{
	if(!B3D_ENSURE(newSize.Width >= mSettings.Size.Width && newSize.Height >= mSettings.Size.Height))
		return;

	const Size2UI& oldSize = mSettings.Size;
	mSettings.Size = newSize;

	const u32 deltaX = newSize.Width - oldSize.Width;
	const u32 deltaY = newSize.Height - oldSize.Height;

	for(auto& page : mPages)
	{
		Node& rootNode = mNodes[page.RootNodeId];

		// Just the root node, we can resize it directly
		if(rootNode.State == NodeState::Free && rootNode.Area.Width == oldSize.Width && rootNode.Area.Height == oldSize.Height)
		{
			B3D_ENSURE(rootNode.Area.Width == oldSize.Width && rootNode.Area.Height == oldSize.Height);
			rootNode.Area.Width = newSize.Width;
			rootNode.Area.Height = newSize.Height;

			continue;
		}

		const bool isGrowingInRootDirection = rootNode.Orientation == NodeOrientation::Horizontal ? deltaX > 0 : deltaY > 0;
		const bool isGrowingInFlippedRootDirection = rootNode.Orientation == NodeOrientation::Horizontal ? deltaY > 0 : deltaX > 0;

		// Find last sibling and either expend it (if free), or add a new free sibling
		if(isGrowingInRootDirection)
		{
			u32 currentNodeId = page.RootNodeId;
			while(mNodes[currentNodeId].NextSiblingId != ~0u)
				currentNodeId = mNodes[currentNodeId].NextSiblingId;

			Node& currentNode = mNodes[currentNodeId];
			if(currentNode.State == NodeState::Free)
			{
				if(rootNode.Orientation == NodeOrientation::Horizontal)
					currentNode.Area.Width += deltaX;
				else
					currentNode.Area.Height += deltaY;
			}
			else
			{
				Rect2I newArea;

				if(rootNode.Orientation == NodeOrientation::Horizontal)
				{
					newArea.X = currentNode.Area.X + (i32)currentNode.Area.Width;
					newArea.Y = currentNode.Area.Y;
					newArea.Width = deltaX;
					newArea.Height = currentNode.Area.Height;
				}
				else
				{
					newArea.X = currentNode.Area.X;
					newArea.Y = currentNode.Area.Y + (i32)currentNode.Area.Height;
					newArea.Width = currentNode.Area.Width;
					newArea.Height = deltaY;
				}

				const u32 newSiblingNodeId = AllocateNode();
				currentNode.NextSiblingId = newSiblingNodeId;

				Node& newSiblingNode = mNodes[newSiblingNodeId];
				newSiblingNode.PreviousSiblingId = currentNodeId;
				newSiblingNode.Area = newArea;
				newSiblingNode.State = NodeState::Free;
				newSiblingNode.Orientation = currentNode.Orientation;

				RegisterFreeNode(page, newSiblingNodeId, Size2UI(newArea.Width, newArea.Height));
			}
		}

		// Create a new root node that as children has old root node and the free area
		if(isGrowingInFlippedRootDirection)
		{
			const u32 freeNodeId = AllocateNode();
			const u32 newRootNodeId = AllocateNode();

			const u32 oldRootId = page.RootNodeId;
			page.RootNodeId = newRootNodeId;

			const NodeOrientation newRootOrientation = rootNode.Orientation == NodeOrientation::Horizontal ? NodeOrientation::Vertical : NodeOrientation::Horizontal;

			Rect2I newArea;
			if(newRootOrientation == NodeOrientation::Horizontal)
			{
				newArea.X = (i32)oldSize.Width;
				newArea.Y = 0;
				newArea.Width = deltaX;
				newArea.Height = newSize.Height;
			}
			else
			{
				newArea.X = 0;
				newArea.Y = (i32)oldSize.Height;
				newArea.Width = newSize.Width;
				newArea.Height = deltaY;
			}

			Node& freeNode = mNodes[freeNodeId];
			freeNode.PreviousSiblingId = newRootNodeId;
			freeNode.Area = newArea;
			freeNode.State = NodeState::Free;
			freeNode.Orientation = newRootOrientation;

			Node& newRootNode = mNodes[newRootNodeId];
			newRootNode.NextSiblingId = freeNodeId;
			newRootNode.Area = Rect2I::kEmpty;
			newRootNode.State = NodeState::Container;
			newRootNode.Orientation = newRootOrientation;

			RegisterFreeNode(page, freeNodeId, Size2UI(newArea.Width, newArea.Height));

			u32 currentNodeId = oldRootId;
			while(currentNodeId != ~0u)
			{
				mNodes[currentNodeId].ParentNodeId = newRootNodeId;
				currentNodeId = mNodes[currentNodeId].NextSiblingId;
			}

			currentNodeId = mNodes[oldRootId].PreviousSiblingId;
			while(currentNodeId != ~0u)
			{
				mNodes[currentNodeId].ParentNodeId = newRootNodeId;
				currentNodeId = mNodes[currentNodeId].PreviousSiblingId;
			}
		}
	}
}

u32 TreeTextureAtlasLayout::FindBestFreeNode(Page& page, const Size2UI& size)
{
	const u32 bestBucketIndex = GetFreeNodeBucketForSize(page, size);
	const bool useWorstFit = bestBucketIndex == (u32)(page.FreeNodeBuckets.size() - 1); // Worst fit for bucket containing large objects

	for(u32 bucketIndex = bestBucketIndex; bucketIndex < (u32)(page.FreeNodeBuckets.size() - 1); ++bucketIndex)
	{
		u32 bestScore = useWorstFit ? 0 : std::numeric_limits<u32>::max();
		u32 bestNodeIndex = ~0u;
		u32 bestBucketEntryIndex = ~0u;

		FreeNodeBucket& bucket = page.FreeNodeBuckets[bucketIndex];
		for(u32 bucketEntryIndex = 0; bucketEntryIndex < (u32)bucket.FreeNodes.size(); ++bucketEntryIndex)
		{
			const u32 nodeIndex = bucket.FreeNodes[bucketEntryIndex];
			Node& node = mNodes[nodeIndex];

			// Merged nodes aren't removed from the free list, so we do it here
			if(node.State != NodeState::Free)
			{
				B3DSwapAndErase(bucket.FreeNodes, bucketEntryIndex);
				continue;
			}

			const i32 deltaX = (i32)size.Width - (i32)node.Area.Width;
			const i32 deltaY = (i32)size.Height - (i32)node.Area.Height;

			if(deltaX < 0 || deltaY < 0)
				continue;

			if(deltaX == 0 && deltaY == 0)
			{
				bestNodeIndex = nodeIndex;
				bestBucketEntryIndex = bucketEntryIndex;
				break;
			}

			const u32 score = Math::Min(deltaX, deltaY);
			if(useWorstFit && score > bestScore || !useWorstFit && score < bestScore)
			{
				bestScore = score;
				bestNodeIndex = nodeIndex;
				bestBucketEntryIndex = bucketEntryIndex;
			}
		}

		if(bestNodeIndex != ~0u)
		{
			B3DSwapAndErase(bucket.FreeNodes, bestBucketEntryIndex);
			return bestNodeIndex;
		}
	}

	return ~0u;
}

void TreeTextureAtlasLayout::RegisterFreeNode(Page& page, u32 nodeId, const Size2UI& size)
{
	B3D_ENSURE(mNodes[nodeId].State == NodeState::Free);

	const u32 bestBucketIndex = GetFreeNodeBucketForSize(page, size);
	page.FreeNodeBuckets[bestBucketIndex].FreeNodes.push_back(nodeId);
}

Size2UI TreeTextureAtlasLayout::AlignSize(const Size2UI& size) const
{
	Size2UI output;
	output.Width = Math::CeilToMultiple(size.Width, mSettings.Alignment.Width);
	output.Height = Math::CeilToMultiple(size.Height, mSettings.Alignment.Height);

	return output;
}

u32 TreeTextureAtlasLayout::GetFreeNodeBucketForSize(Page& page, const Size2UI& size) const
{
	const u32 largestDimension = Math::Max(size.Width, size.Height);
	for(u32 bucketIndex = 0; bucketIndex < (u32)page.FreeNodeBuckets.size(); bucketIndex++)
	{
		if(largestDimension <= page.FreeNodeBuckets[bucketIndex].Size)
			return bucketIndex;
	}

	return (u32)page.FreeNodeBuckets.size() - 1;
}

TreeTextureAtlasLayout::NodeSplitResult TreeTextureAtlasLayout::Split(const Node& nodeToSplit, const Size2UI& requiredSize) const
{
	NodeSplitResult result;

	if(requiredSize.Width > nodeToSplit.Area.Width || requiredSize.Height > nodeToSplit.Area.Height)
	{
		B3D_ENSURE(false);
		return result;
	}

	if(requiredSize.Width == nodeToSplit.Area.Width && requiredSize.Height == nodeToSplit.Area.Height)
	{
		result.LargerAreaOrientation = nodeToSplit.Orientation;
		return result;
	}

	const Rect2I rightLeftoverArea(
		nodeToSplit.Area.X + (i32)requiredSize.Width,
		nodeToSplit.Area.Y,
		nodeToSplit.Area.Width - requiredSize.Width,
		requiredSize.Height);

	const Rect2I bottomLeftoverArea(
		nodeToSplit.Area.X,
		nodeToSplit.Area.Y + (i32)requiredSize.Height,
		requiredSize.Width,
		nodeToSplit.Area.Height - requiredSize.Height);

	if(rightLeftoverArea.Width * rightLeftoverArea.Height < bottomLeftoverArea.Width * bottomLeftoverArea.Height)
	{
		result.SmallerLeftoverArea = rightLeftoverArea;
		result.LargerLeftoverArea = bottomLeftoverArea;
		result.LargerLeftoverArea.Width = nodeToSplit.Area.Width;
		result.LargerAreaOrientation = NodeOrientation::Vertical;
	}
	else
	{
		result.SmallerLeftoverArea = bottomLeftoverArea;
		result.LargerLeftoverArea = rightLeftoverArea;
		result.LargerLeftoverArea.Height = nodeToSplit.Area.Height;
		result.LargerAreaOrientation = NodeOrientation::Horizontal;
	}

	return result;
}

u32 TreeTextureAtlasLayout::AllocateNode()
{
	if(mUnusedNodeListHead < (u32)mNodes.size())
	{
		const u32 freeNodeId = mUnusedNodeListHead;
		mUnusedNodeListHead = mNodes[mUnusedNodeListHead].NextSiblingId;

		B3D_ENSURE(mNodes[freeNodeId].State == NodeState::Unused);
		mNodes[freeNodeId] = Node();

		return freeNodeId;
	}

	mNodes.push_back(Node());
	return (u32)mNodes.size() - 1;
}

void TreeTextureAtlasLayout::FreeNode(u32 nodeId)
{
	mNodes[nodeId].State = NodeState::Unused;
	mNodes[nodeId].NextSiblingId = mUnusedNodeListHead;
	mUnusedNodeListHead = nodeId;
}

void TreeTextureAtlasLayout::MergeWithNextSibling(u32 nodeId)
{
	Node& nodeToMerge = mNodes[nodeId];
	B3D_ENSURE(nodeToMerge.State == NodeState::Free);

	if(nodeToMerge.NextSiblingId == ~0u)
		return;

	const u32 nextSiblingId = nodeToMerge.NextSiblingId;
	Node& nextNode = mNodes[nextSiblingId];
	if(nextNode.State != NodeState::Free)
		return;

	if(nodeToMerge.Orientation == NodeOrientation::Horizontal)
	{
		B3D_ENSURE(nodeToMerge.Area.Y == nextNode.Area.Y);
		B3D_ENSURE(nodeToMerge.Area.Height == nextNode.Area.Height);

		nodeToMerge.Area.Width += nextNode.Area.Width;
	}
	else
	{
		B3D_ENSURE(nodeToMerge.Area.X == nextNode.Area.X);
		B3D_ENSURE(nodeToMerge.Area.Width == nextNode.Area.Width);

		nodeToMerge.Area.Height += nextNode.Area.Height;
	}

	const u32 nextNextSiblingId = nextNode.NextSiblingId;
	nodeToMerge.NextSiblingId = nextNextSiblingId;
	if(nextNextSiblingId != ~0u)
		mNodes[nextNextSiblingId].PreviousSiblingId = nodeId;
	
	FreeNode(nextSiblingId);
}

void TreeTextureAtlasLayout::Clear()
{
	mPages.clear();
	mNodes.clear();
	mUnusedNodeListHead = ~0u;
}

TreeTextureAtlasLayout::Page TreeTextureAtlasLayout::AllocatePage()
{
	Page page;
	page.RootNodeId = AllocateNode();

	page.FreeNodeBuckets[0].Size = mSettings.SmallSizeLimit;
	page.FreeNodeBuckets[1].Size = mSettings.LargeSizeLimit;
	page.FreeNodeBuckets[2].Size = ~0u;

	page.FreeNodeBuckets[0].Size = mSettings.SmallSizeLimit;
	page.FreeNodeBuckets[1].Size = mSettings.LargeSizeLimit;
	page.FreeNodeBuckets[2].Size = ~0u;

	Node& rootNode = mNodes[page.RootNodeId];
	rootNode = Node();
	rootNode.Area = Rect2I(0, 0, mSettings.Size.Width, mSettings.Size.Height);
	rootNode.State = NodeState::Free;

	FreeNodeBucket& rootNodeBucket = page.FreeNodeBuckets[GetFreeNodeBucketForSize(page, mSettings.Size)];
	rootNodeBucket.FreeNodes.push_back(0);

	return page;
}

void TreeTextureAtlasLayout::FreePage(u32 pageId)
{
	Page& page = mPages[pageId];
	Node& rootNode = mNodes[page.RootNodeId];
	B3D_ENSURE(rootNode.State == NodeState::Free);

	FreeNode(page.RootNodeId);
}
