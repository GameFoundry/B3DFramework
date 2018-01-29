//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Private/UnitTests/BsUtilityTestSuite.h"
#include "Private/UnitTests/BsFileSystemTestSuite.h"
#include "Utility/BsOctree.h"

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
