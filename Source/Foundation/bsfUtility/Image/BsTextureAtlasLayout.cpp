//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsTextureAtlasLayout.h"
#include "Debug/BsDebug.h"
#include "Utility/BsBitwise.h"

namespace bs
{
	bool TextureAtlasLayout::AddElement(UINT32 width, UINT32 height, UINT32& x, UINT32& y)
	{
		if(width == 0 || height == 0)
		{
			x = 0;
			y = 0;
			return true;
		}

		// Try adding without expanding, if that fails try to expand
		if(!addToNode(0, width, height, x, y, false))
		{
			if (!addToNode(0, width, height, x, y, true))
				return false;
		}

		// Update size to cover all nodes
		if(mPow2)
		{
			mWidth = std::max(mWidth, Bitwise::nextPow2(x + width));
			mHeight = std::max(mHeight, Bitwise::nextPow2(y + height));
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
		mNodes.Clear();
		mNodes.push_back(TexAtlasNode(0, 0, mWidth, mHeight));

		mWidth = mInitialWidth;
		mHeight = mInitialHeight;
	}

	bool TextureAtlasLayout::AddToNode(UINT32 nodeIdx, UINT32 width, UINT32 height, UINT32& x, UINT32& y, bool allowGrowth)
	{
		TexAtlasNode* node = &mNodes[nodeIdx];
		float aspect = node->width / (float)node->height;

		if (node->children[0] != (UINT32)-1)
		{
			if (addToNode(node->children[0], width, height, x, y, allowGrowth))
				return true;

			return AddToNode(node->children[1], width, height, x, y, allowGrowth);
		}
		else
		{
			if (node->nodeFull)
				return false;

			if (width > node->width || height > node->height)
				return false;

			if(!allowGrowth)
			{
				if (node->x + width > mWidth || node->y + height > mHeight)
					return false;
			}

			if (width == node->width && height == node->height)
			{
				x = node->x;
				y = node->y;
				node->nodeFull = true;

				return true;
			}

			float dw = (float)(node->width - width);
			float dh = (node->height - height) * aspect;

			UINT32 nextChildIdx = (UINT32)mNodes.Size();
			node->children[0] = nextChildIdx;
			node->children[1] = nextChildIdx + 1;

			TexAtlasNode nodeCopy = *node;
			node = nullptr; // Undefined past this point
			if (dw > dh)
			{
				mNodes.emplace_back(nodeCopy.x, nodeCopy.y, width, nodeCopy.height);
				mNodes.emplace_back(nodeCopy.x + width, nodeCopy.y, nodeCopy.width - width, nodeCopy.height);
			}
			else
			{
				mNodes.emplace_back(nodeCopy.x, nodeCopy.y, nodeCopy.width, height);
				mNodes.emplace_back(nodeCopy.x, nodeCopy.y + height, nodeCopy.width, nodeCopy.height - height);
			}

			return AddToNode(nodeCopy.children[0], width, height, x, y, allowGrowth);
		}
	}

	Vector<TextureAtlasUtility::Page> TextureAtlasUtility::createAtlasLayout(Vector<Element>& elements, UINT32 width,
		UINT32 height, UINT32 maxWidth, UINT32 maxHeight, bool pow2)
	{
		for (size_t i = 0; i < elements.Size(); i++)
		{
			elements[i].output.idx = (UINT32)i; // Preserve original index before sorting
			elements[i].output.page = -1;
		}

		std::sort(elements.Begin(), elements.end(),
			[](const Element& a, const Element& b)
		{
			return a.input.width * a.input.height > b.input.width * b.input.height;
		});

		Vector<TextureAtlasLayout> layouts;
		UINT32 remainingCount = (UINT32)elements.Size();
		while (remainingCount > 0)
		{
			layouts.push_back(TextureAtlasLayout(width, height, maxWidth, maxHeight, pow2));
			TextureAtlasLayout& curLayout = layouts.Back();

			// Find largest unassigned element that fits
			UINT32 sizeLimit = std::numeric_limits<UINT32>::max();
			while (true)
			{
				UINT32 largestId = -1;

				// Assumes elements are sorted from largest to smallest
				for (UINT32 i = 0; i < (UINT32)elements.Size(); i++)
				{
					if (elements[i].output.page == -1)
					{
						UINT32 size = elements[i].input.width * elements[i].input.height;
						if (size < sizeLimit)
						{
							largestId = i;
							break;
						}
					}
				}

				if (largestId == (UINT32)-1)
					break; // Nothing fits, start a new page

				Element& element = elements[largestId];

				// Check if an element is too large to ever fit
				if(element.input.width > maxWidth || element.input.height > maxHeight)
				{
					BS_LOG(Warning, Generic, "Some of the provided elements don't fit in an atlas of provided size. "
						"Returning empty array of pages.");
					return Vector<Page>();
				}

				if (curLayout.AddElement(element.input.width, element.input.height, element.output.x, element.output.y))
				{
					element.output.page = (UINT32)layouts.Size() - 1;
					remainingCount--;
				}
				else
					sizeLimit = element.input.width * element.input.height;
			}
		}

		Vector<Page> pages;
		for (auto& layout : layouts)
			pages.push_back({ layout.GetWidth(), layout.getHeight() });

		return pages;
	}
}
