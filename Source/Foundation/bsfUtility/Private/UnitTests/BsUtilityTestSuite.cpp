//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/UnitTests/BsUtilityTestSuite.h"
#include "Private/UnitTests/BsFileSystemTestSuite.h"
#include "Utility/BsOctree.h"
#include "Utility/BsBitfield.h"

namespace bs
{
	struct DebugOctreeElem
	{
		AABox box;
		mutable OctreeElementId octreeId;
	};

	struct DebugOctreeData
	{
		Vector<DebugOctreeElem> elements;
	};

	struct DebugOctreeOptions
	{
		enum { LoosePadding = 16 };
		enum { MinElementsPerNode = 8 };
		enum { MaxElementsPerNode = 16 };
		enum { MaxDepth = 12};

		static simd::AABox getBounds(UINT32 elem, void* context)
		{
			DebugOctreeData* octreeData = (DebugOctreeData*)context;
			return simd::AABox(octreeData->elements[elem].box);
		}

		static void setElementId(UINT32 elem, const OctreeElementId& id, void* context)
		{
			DebugOctreeData* octreeData = (DebugOctreeData*)context;
			octreeData->elements[elem].octreeId = id;
		}
	};

	typedef Octree<UINT32, DebugOctreeOptions> DebugOctree;
	void UtilityTestSuite::startUp()
	{
		SPtr<TestSuite> fileSystemTests = create<FileSystemTestSuite>();
		add(fileSystemTests);
	}

	void UtilityTestSuite::shutDown()
	{
	}

	UtilityTestSuite::UtilityTestSuite()
	{
		BS_ADD_TEST(UtilityTestSuite::testOctree);
		BS_ADD_TEST(UtilityTestSuite::testBitfield)
	}

	void UtilityTestSuite::testBitfield()
	{
		static constexpr UINT32 COUNT = 100;
		static constexpr UINT32 EXTRA_COUNT = 32;

		Bitfield bitfield(true, COUNT);

		// Basic iteration
		UINT32 i = 0;
		for (auto iter : bitfield)
		{
			BS_TEST_ASSERT(iter == true)
				i++;
		}

		UINT32 curCount = COUNT;
		BS_TEST_ASSERT(i == curCount);

		// Dynamic additon
		bitfield.add(false);
		bitfield.add(false);
		bitfield.add(true);
		bitfield.add(false);
		curCount += 4;

		// Realloc
		curCount += EXTRA_COUNT;
		for (uint32_t j = 0; j < 32; j++)
			bitfield.add(false);

		BS_TEST_ASSERT(bitfield.size() == curCount);

		BS_TEST_ASSERT(bitfield[COUNT + 0] == false);
		BS_TEST_ASSERT(bitfield[COUNT + 1] == false);
		BS_TEST_ASSERT(bitfield[COUNT + 2] == true);
		BS_TEST_ASSERT(bitfield[COUNT + 3] == false);

		// Modify during iteration
		i = 0;
		for (auto iter : bitfield)
		{
			if (i >= 50 && i <= 70)
				iter = false;

			i++;
		}

		// Modify directly using []
		bitfield[5] = false;
		bitfield[6] = false;

		for (UINT32 j = 50; j < 70; j++)
			BS_TEST_ASSERT(bitfield[j] == false);

		BS_TEST_ASSERT(bitfield[5] == false);
		BS_TEST_ASSERT(bitfield[6] == false);

		// Removal
		bitfield.remove(10);
		bitfield.remove(10);
		curCount -= 2;

		for (UINT32 j = 48; j < 68; j++)
			BS_TEST_ASSERT(bitfield[j] == false);

		BS_TEST_ASSERT(bitfield[5] == false);
		BS_TEST_ASSERT(bitfield[6] == false);

		BS_TEST_ASSERT(bitfield.size() == curCount);

		// Find
		BS_TEST_ASSERT(bitfield.find(true) == 0);
		BS_TEST_ASSERT(bitfield.find(false) == 5);
	}

	void UtilityTestSuite::testOctree()
	{
		DebugOctreeData octreeData;
		DebugOctree octree(Vector3::ZERO, 800.0f, &octreeData);

		struct SizeAndCount
		{
			float sizeMin;
			float sizeMax;
			UINT32 count;
		};

		SizeAndCount types[]
		{
			{ 0.02f, 0.2f, 2000 }, // Very small objects
			{ 0.2f, 1.0f, 2000 }, // Small objects
			{ 1.0f, 5.0f, 5000 }, // Medium sized objects
			{ 5.0f, 30.0f, 4000 }, // Large objects
			{ 30.0f, 100.0f, 2000 } // Very large objects
		};

		float placementExtents = 750.0f;
		for(UINT32 i = 0; i < sizeof(types)/sizeof(types[0]); i++)
		{
			for (UINT32 j = 0; j < types[i].count; j++)
			{
				Vector3 position(
					((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents,
					((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents,
					((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents
				);

				Vector3 extents(
					types[i].sizeMin + ((rand() / (float)RAND_MAX)) * (types[i].sizeMax - types[i].sizeMin) * 0.5f,
					types[i].sizeMin + ((rand() / (float)RAND_MAX)) * (types[i].sizeMax - types[i].sizeMin) * 0.5f,
					types[i].sizeMin + ((rand() / (float)RAND_MAX)) * (types[i].sizeMax - types[i].sizeMin) * 0.5f
				);

				DebugOctreeElem elem;
				elem.box = AABox(position - extents, position + extents);

				UINT32 elemIdx = (UINT32)octreeData.elements.size();
				octreeData.elements.push_back(elem);
				octree.addElement(elemIdx);
			}
		}

		DebugOctreeElem manualElems[3];
		manualElems[0].box = AABox(Vector3(100.0f, 100.0f, 100.f), Vector3(110.0f, 115.0f, 110.0f));
		manualElems[1].box = AABox(Vector3(200.0f, 100.0f, 100.f), Vector3(250.0f, 150.0f, 150.0f));
		manualElems[2].box = AABox(Vector3(90.0f, 90.0f, 90.f), Vector3(105.0f, 105.0f, 110.0f));

		
		for(UINT32 i = 0; i < 3; i++)
		{
			UINT32 elemIdx = (UINT32)octreeData.elements.size();
			octreeData.elements.push_back(manualElems[i]);
			octree.addElement(elemIdx);
		}

		AABox queryBounds = manualElems[0].box;
		DebugOctree::BoxIntersectIterator interIter(octree, queryBounds);

		Vector<UINT32> overlapElements;
		while(interIter.moveNext())
		{
			UINT32 element = interIter.getElement();
			overlapElements.push_back(element);

			// Manually check for intersections
			BS_TEST_ASSERT(octreeData.elements[element].box.intersects(queryBounds));
		}

		// Ensure that all we have found all possible overlaps by manually testing all elements
		UINT32 elemIdx = 0;
		for(auto& entry : octreeData.elements)
		{
			if(entry.box.intersects(queryBounds))
			{
				auto iterFind = std::find(overlapElements.begin(), overlapElements.end(), elemIdx);
				BS_TEST_ASSERT(iterFind != overlapElements.end());
			}

			elemIdx++;
		}

		// Ensure nothing goes wrong during element removal
		for(auto& entry : octreeData.elements)
			octree.removeElement(entry.octreeId);
	}
}
