//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTextureAtlasLayout.h"
#include "BsDebug.h"
#include "BsBitwise.h"

namespace bs
{
	TextureAtlasLayout::TexAtlasNode::TexAtlasNode()
		:x(0), y(0), width(0), height(0), children{ (UINT32)-1, (UINT32)-1 }, nodeFull(false)
	{ }

	TextureAtlasLayout::TexAtlasNode::TexAtlasNode(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
		: x(x), y(y), width(width), height(height), children{ (UINT32)-1, (UINT32)-1 }, nodeFull(false)
	{ }

	TextureAtlasLayout::TextureAtlasLayout()
		: mWidth(0), mHeight(0), mMaxWidth(0), mMaxHeight(0), mPow2(false)
	{ }

	TextureAtlasLayout::TextureAtlasLayout(UINT32 width, UINT32 height, UINT32 maxWidth, UINT32 maxHeight, bool pow2)
		: mWidth(width), mHeight(height), mMaxWidth(maxWidth), mMaxHeight(maxHeight), mPow2(pow2)
	{
		mNodes.push_back(TexAtlasNode(0, 0, maxWidth, maxHeight));
	}

	bool TextureAtlasLayout::addElement(UINT32 width, UINT32 height, UINT32& x, UINT32& y)
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

	bool TextureAtlasLayout::addToNode(UINT32 nodeIdx, UINT32 width, UINT32 height, UINT32& x, UINT32& y, bool allowGrowth)
	{
		TexAtlasNode* node = &mNodes[nodeIdx];
		float aspect = node->width / (float)node->height;

		if (node->children[0] != -1)
		{
			if (addToNode(node->children[0], width, height, x, y, allowGrowth))
				return true;

			return addToNode(node->children[1], width, height, x, y, allowGrowth);
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

			UINT32 nextChildIdx = (UINT32)mNodes.size();
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

			return addToNode(nodeCopy.children[0], width, height, x, y, allowGrowth);
		}
	}

	Vector<TextureAtlasUtility::Page> TextureAtlasUtility::createAtlasLayout(Vector<Element>& elements, UINT32 width, 
		UINT32 height, UINT32 maxWidth, UINT32 maxHeight, bool pow2)
	{
		for (size_t i = 0; i < elements.size(); i++)
		{
			elements[i].output.idx = i; // Preserve original index before sorting
			elements[i].output.page = -1;
		}

		std::sort(elements.begin(), elements.end(), 
			[](const Element& a, const Element& b)
		{
			return a.input.width * a.input.height > b.input.width * b.input.height;
		});

		Vector<TextureAtlasLayout> layouts;
		UINT32 remainingCount = (UINT32)elements.size();
		while (remainingCount > 0)
		{
			layouts.push_back(TextureAtlasLayout(width, height, maxWidth, maxHeight, pow2));
			TextureAtlasLayout& curLayout = layouts.back();

			// Find largest unassigned element that fits
			UINT32 sizeLimit = std::numeric_limits<UINT32>::max();
			while (true)
			{
				UINT32 largestId = -1;

				// Assumes elements are sorted from largest to smallest
				for (UINT32 i = 0; i < (UINT32)elements.size(); i++)
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

				if (largestId == -1)
					break; // Nothing fits, start a new page

				Element& element = elements[largestId];

				// Check if an element is too large to ever fit
				if(element.input.width > maxWidth || element.input.height > maxHeight)
				{
					LOGWRN("Some of the provided elements don't fit in an atlas of provided size. Returning empty array of pages.");
					return Vector<Page>();
				}

				if (curLayout.addElement(element.input.width, element.input.height, element.output.x, element.output.y))
				{
					element.output.page = (UINT32)layouts.size() - 1;
					remainingCount--;
				}
				else
					sizeLimit = element.input.width * element.input.height;
			}
		}

		Vector<Page> pages;
		for (auto& layout : layouts)
			pages.push_back({ layout.getWidth(), layout.getHeight() });

		return pages;
	}
}
