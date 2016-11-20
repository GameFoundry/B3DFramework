//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTexAtlasGenerator.h"
#include "BsDebug.h"

namespace bs
{
	class TexAtlasNode
	{
	public:
		TexAtlasNode()
			:x(0), y(0), width(0), height(0), children(nullptr), nodeFull(false)
		{ }

		TexAtlasNode(UINT32 _x, UINT32 _y, UINT32 _width, UINT32 _height)
			:x(_x), y(_y), width(_width), height(_height), children(nullptr), nodeFull(false)
		{ }

		~TexAtlasNode()
		{
			if(children != nullptr)
				bs_deleteN(children, 2);
		}

		UINT32 x, y, width, height;
		TexAtlasNode* children;
		bool nodeFull;

		bool insert(TexAtlasElementDesc& element)
		{
			float aspect = width / (float)height;

			return insert(element, aspect);
		}

		bool insert(TexAtlasElementDesc& element, float aspect)
		{
			if (children != nullptr)
			{
				if (children[0].insert(element))
					return true;

				return children[1].insert(element);
			}
			else
			{
				if(nodeFull)
					return false;

				if (element.input.width > width || element.input.height > height)
					return false;

				if (element.input.width == width && element.input.height == height)
				{
					element.output.x = x;
					element.output.y = y;
					nodeFull = true;

					return true;
				}

				float dw = (float)(width - element.input.width);
				float dh = (height - element.input.height) * aspect;

				children = bs_newN<TexAtlasNode>(2);

				if (dw > dh)
				{
					children[0].x = x;
					children[0].y = y;
					children[0].width = element.input.width;
					children[0].height = height;

					children[1].x = x + element.input.width;
					children[1].y = y;
					children[1].width = width - element.input.width;
					children[1].height = height;
				}
				else
				{
					children[0].x = x;
					children[0].y = y;
					children[0].width = width;
					children[0].height = element.input.height;

					children[1].x = x;
					children[1].y = y + element.input.height;
					children[1].width = width;
					children[1].height = height - element.input.height;
				}

				return children[0].insert(element);
			}
		}
	};

	TexAtlasGenerator::TexAtlasGenerator(bool square, UINT32 maxTexWidth, UINT32 maxTexHeight, bool fixedSize)
		:mSquare(square), mFixedSize(fixedSize), mMaxTexWidth(maxTexWidth), mMaxTexHeight(maxTexHeight)
	{
		if(square)
		{
			if(maxTexWidth > maxTexHeight)
				maxTexWidth = maxTexHeight;

			if(maxTexHeight > maxTexWidth)
				maxTexHeight = maxTexWidth;
		}
	}

	Vector<TexAtlasPageDesc> TexAtlasGenerator::createAtlasLayout(Vector<TexAtlasElementDesc>& elements) const
	{
		for(size_t i = 0; i < elements.size(); i++)
			elements[i].output.page = -1;

		//sortBySize(elements);
		int numPages = generatePagesForSize(elements, mMaxTexWidth, mMaxTexHeight);

		if(numPages == -1)
		{
			LOGWRN("Some of the provided elements don't fit in an atlas of provided size. Returning empty array of pages.");
			return Vector<TexAtlasPageDesc>();
		}

		if(numPages == 0)
			return Vector<TexAtlasPageDesc>();

		UINT32 lastPageWidth = mMaxTexWidth;
		UINT32 lastPageHeight = mMaxTexHeight;
		INT32 lastPageIdx = numPages - 1;

		// If size isn't fixed, try to reduce the size of the last page
		if(!mFixedSize)
		{
			while (true)
			{
				if (lastPageWidth <= 1 || lastPageHeight <= 1)
					break;

				int newLastPageWidth = lastPageWidth;
				int newLastPageHeight = lastPageHeight;

				if(mSquare)
				{
					if (newLastPageWidth > newLastPageHeight)
						newLastPageWidth /= 2;
					else
						newLastPageHeight /= 2;
				}
				else
				{
					if (newLastPageWidth > newLastPageHeight)
						newLastPageWidth /= 2;
					else
						newLastPageHeight /= 2;
				}

				// Clear page indexes so we know which pages to process
				for(size_t i = 0; i < elements.size(); i++)
				{
					if(elements[i].output.page >= lastPageIdx)
						elements[i].output.page = -1;
				}

				if(generatePagesForSize(elements, newLastPageWidth, newLastPageHeight, lastPageIdx) == 1)
				{
					lastPageWidth = newLastPageWidth;
					lastPageHeight = newLastPageHeight;
				}
				else
				{
					// We're done but we need to re-do all the pages with the last valid size
					for(size_t i = 0; i < elements.size(); i++)
					{
						if(elements[i].output.page >= lastPageIdx)
							elements[i].output.page = -1;
					}

					generatePagesForSize(elements, lastPageWidth, lastPageHeight, lastPageIdx);

					break;
				}
			}
		}

		// Handle degenerate case
		for(size_t i = 0; i < elements.size(); i++)
		{
			if(elements[i].output.page == -1 && elements[i].input.width == 0 && elements[i].input.height == 0)
			{
				elements[i].output.x = 0;
				elements[i].output.y = 0;
			}
		}

		// Create page descriptors and return
		Vector<TexAtlasPageDesc> pages;
		for(int i = 0; i < numPages - 1; i++)
		{
			TexAtlasPageDesc pageDesc;
			pageDesc.width = mMaxTexWidth;
			pageDesc.height = mMaxTexHeight;

			pages.push_back(pageDesc);
		}

		TexAtlasPageDesc lastPageDesc;
		lastPageDesc.width = lastPageWidth;
		lastPageDesc.height = lastPageHeight;

		pages.push_back(lastPageDesc);

		return pages;
	}

	int TexAtlasGenerator::generatePagesForSize(Vector<TexAtlasElementDesc>& elements, UINT32 width, UINT32 height, UINT32 startPage) const
	{
		if(elements.size() == 0)
			return 0;

		int currentPage = startPage;
		int numPages = 0;
		while (true)
		{
			int largestTexId = findLargestTextureWithoutPage(elements); // Start with the largest available texture

			if (largestTexId == -1) // No more textures, we're done
				return numPages;

			TexAtlasElementDesc& currentElem = elements[largestTexId];

			// If the texture is larger than the atlas size then it can never fit
			while (width < currentElem.input.width || height < currentElem.input.height)
				return -1;

			TexAtlasNode atlasNode(0, 0, width, height);
			atlasNode.insert(elements[largestTexId]);
			elements[largestTexId].output.page = currentPage;

			// Now that the first (largest) texture has been added, do the same for every other texture until atlas is full
			while (true)
			{
				int addedTextureId = addLargestTextureWithoutPageThatFits(elements, atlasNode); // Try to add next largest texture
				if (addedTextureId == -1)
					break;

				elements[addedTextureId].output.page = currentPage;
			}

			currentPage++;
			numPages++;
		}
	}

	int TexAtlasGenerator::addLargestTextureWithoutPageThatFits(Vector<TexAtlasElementDesc>& elements, TexAtlasNode& node) const
	{
		UINT32 sizeLimit = std::numeric_limits<UINT32>::max();
		while (true)
		{
			UINT32 largestSize = 0;
			INT32 largestId = -1;

			for (size_t i = 0; i < elements.size(); i++)
			{
				if (elements[i].output.page == -1) // Signifies that we haven't processed this node yet
				{
					UINT32 size = elements[i].input.width * elements[i].input.height;
					if (size > largestSize && size < sizeLimit)
					{
						largestSize = size;
						largestId = (INT32)i;
					}
				}
			}

			if (largestId == -1)
				return -1;

			if (node.insert(elements[largestId]))
				return largestId;
			else
				sizeLimit = largestSize;
		}
	}

	int TexAtlasGenerator::findLargestTextureWithoutPage(const Vector<TexAtlasElementDesc>& elements) const
	{
		INT32 largestId = -1;
		UINT32 largestSize = 0;

		for (size_t i = 0; i < elements.size(); i++)
		{
			if (elements[i].output.page == -1) // Signifies that we haven't processed this node yet
			{
				UINT32 size = elements[i].input.width * elements[i].input.height;
				if (size > largestSize)
				{
					largestSize = size;
					largestId = (INT32)i;
				}
			}
		}

		return largestId;
	}

	void TexAtlasGenerator::sortBySize(Vector<TexAtlasElementDesc>& elements) const
	{
		for (size_t i = 0; i < elements.size(); i++)
		{
			INT32 largestId = -1;
			UINT32 largestSize = 0;

			for (size_t j = i; j < elements.size(); j++)
			{
				UINT32 size = elements[j].input.width * elements[j].input.height;

				if (size > largestSize)
				{
					largestSize = size;
					largestId = (INT32)j;
				}
			}

			if(largestId != -1)
			{
				TexAtlasElementDesc temp = elements[i];
				elements[i] = elements[largestId];
				elements[largestId] = temp;
			}
		}
	}
}