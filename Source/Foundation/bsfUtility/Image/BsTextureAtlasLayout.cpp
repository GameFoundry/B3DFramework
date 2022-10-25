//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsTextureAtlasLayout.h"
#include "Debug/BsDebug.h"
#include "Utility/BsBitwise.h"

using namespace bs;

bool TextureAtlasLayout::AddElement(u32 width, u32 height, u32& x, u32& y)
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

void TextureAtlasLayout::Clear()
{
	mNodes.clear();
	mNodes.push_back(TexAtlasNode(0, 0, mWidth, mHeight));

	mWidth = mInitialWidth;
	mHeight = mInitialHeight;
}

bool TextureAtlasLayout::AddToNode(u32 nodeIdx, u32 width, u32 height, u32& x, u32& y, bool allowGrowth)
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

	Vector<TextureAtlasLayout> layouts;
	u32 remainingCount = (u32)elements.size();
	while(remainingCount > 0)
	{
		layouts.push_back(TextureAtlasLayout(width, height, maxWidth, maxHeight, pow2));
		TextureAtlasLayout& curLayout = layouts.back();

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
				BS_LOG(Warning, Generic, "Some of the provided elements don't fit in an atlas of provided size. "
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
