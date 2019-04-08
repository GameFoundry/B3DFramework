//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/UnitTests/BsUtilityTestSuite.h"
#include "Private/UnitTests/BsFileSystemTestSuite.h"
#include "Utility/BsOctree.h"
#include "Utility/BsBitfield.h"
#include "Utility/BsDynArray.h"
#include "Math/BsComplex.h"
#include "Utility/BsMinHeap.h"
#include "Utility/BsQuadtree.h"
#include "Utility/BsBitstream.h"
#include "Utility/BsUSPtr.h"

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

	struct DebugQuadtreeElem
	{
		Rect2 box;
		mutable QuadtreeElementId quadtreeId;
	};

	struct DebugQuadtreeData
	{
		Vector<DebugQuadtreeElem> elements;
	};

	struct DebugQuadtreeOptions
	{
		enum { LoosePadding = 8 };
		enum { MinElementsPerNode = 4 };
		enum { MaxElementsPerNode = 8 };
		enum { MaxDepth = 6 };

		static simd::Rect2 getBounds(UINT32 elem, void* context)
		{
			DebugQuadtreeData* quadtreeData = (DebugQuadtreeData*)context;
			return simd::Rect2(quadtreeData->elements[elem].box);
		}

		static void setElementId(UINT32 elem, const QuadtreeElementId& id, void* context)
		{
			DebugQuadtreeData* quadtreeData = (DebugQuadtreeData*)context;
			quadtreeData->elements[elem].quadtreeId = id;
		}
	};

	typedef Quadtree<UINT32, DebugQuadtreeOptions> DebugQuadtree;
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
		BS_ADD_TEST(UtilityTestSuite::testSmallVector)
		BS_ADD_TEST(UtilityTestSuite::testDynArray)
		BS_ADD_TEST(UtilityTestSuite::testComplex)
		BS_ADD_TEST(UtilityTestSuite::testMinHeap)
		BS_ADD_TEST(UtilityTestSuite::testQuadtree)
		BS_ADD_TEST(UtilityTestSuite::testVarInt)
		BS_ADD_TEST(UtilityTestSuite::testBitStream)
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

	void UtilityTestSuite::testSmallVector()
	{
		struct SomeElem
		{
			int a = 10;
			int b = 0;
		};

		// Make sure initial construction works
		SmallVector<SomeElem, 4> v(4);
		BS_TEST_ASSERT(v.size() == 4);
		BS_TEST_ASSERT(v.capacity() == 4);
		BS_TEST_ASSERT(v[0].a == 10);
		BS_TEST_ASSERT(v[3].a == 10);
		BS_TEST_ASSERT(v[3].b == 0);

		// Making the vector dynamic
		v.add({3, 4});
		BS_TEST_ASSERT(v.size() == 5);
		BS_TEST_ASSERT(v[0].a == 10);
		BS_TEST_ASSERT(v[3].a == 10);
		BS_TEST_ASSERT(v[3].b == 0);
		BS_TEST_ASSERT(v[4].a == 3);
		BS_TEST_ASSERT(v[4].b == 4);

		// Make a copy
		SmallVector<SomeElem, 4> v2 = v;
		BS_TEST_ASSERT(v2.size() == 5);
		BS_TEST_ASSERT(v2[0].a == 10);
		BS_TEST_ASSERT(v2[3].a == 10);
		BS_TEST_ASSERT(v2[3].b == 0);
		BS_TEST_ASSERT(v2[4].a == 3);
		BS_TEST_ASSERT(v2[4].b == 4);

		// Pop an element
		v2.pop();
		BS_TEST_ASSERT(v2.size() == 4);
		BS_TEST_ASSERT(v2[0].a == 10);
		BS_TEST_ASSERT(v2[3].a == 10);
		BS_TEST_ASSERT(v2[3].b == 0);

		// Make a static only copy
		SmallVector<SomeElem, 4> v3 = v2;
		BS_TEST_ASSERT(v3.size() == 4);
		BS_TEST_ASSERT(v3.capacity() == 4);
		BS_TEST_ASSERT(v3[0].a == 10);
		BS_TEST_ASSERT(v3[3].a == 10);
		BS_TEST_ASSERT(v3[3].b == 0);

		// Remove an element
		v.remove(2);
		BS_TEST_ASSERT(v.size() == 4);
		BS_TEST_ASSERT(v[0].a == 10);
		BS_TEST_ASSERT(v[2].a == 10);
		BS_TEST_ASSERT(v[3].a == 3);
		BS_TEST_ASSERT(v[3].b == 4);

		// Move a static vector
		SmallVector<SomeElem, 4> v4 = std::move(v3);
		BS_TEST_ASSERT(v3.size() == 0);
		BS_TEST_ASSERT(v4.size() == 4);
		BS_TEST_ASSERT(v4.capacity() == 4);
		BS_TEST_ASSERT(v4[0].a == 10);
		BS_TEST_ASSERT(v4[3].a == 10);
		BS_TEST_ASSERT(v4[3].b == 0);

		// Move a dynamic vector
		SmallVector<SomeElem, 4> v5 = std::move(v2);
		BS_TEST_ASSERT(v2.size() == 0);
		BS_TEST_ASSERT(v5.size() == 4);
		BS_TEST_ASSERT(v5[0].a == 10);
		BS_TEST_ASSERT(v5[3].a == 10);
		BS_TEST_ASSERT(v5[3].b == 0);

		// Move a dynamic vector into a dynamic vector
		v.add({33, 44});
		SmallVector<SomeElem, 4> v6 = std::move(v);
		BS_TEST_ASSERT(v.size() == 0);
		BS_TEST_ASSERT(v6.size() == 5);
		BS_TEST_ASSERT(v6[0].a == 10);
		BS_TEST_ASSERT(v6[3].a == 3);
		BS_TEST_ASSERT(v6[3].b == 4);
		BS_TEST_ASSERT(v6[4].a == 33);
		BS_TEST_ASSERT(v6[4].b == 44);
	}

	void UtilityTestSuite::testDynArray()
	{
		struct SomeElem
		{
			int a = 10;
			int b = 0;
		};

		// Make sure initial construction works
		DynArray<SomeElem> v(4);
		BS_TEST_ASSERT(v.size() == 4);
		BS_TEST_ASSERT(v.capacity() == 4);
		BS_TEST_ASSERT(v[0].a == 10);
		BS_TEST_ASSERT(v[3].a == 10);
		BS_TEST_ASSERT(v[3].b == 0);

		// Add an element
		v.add({3, 4});
		BS_TEST_ASSERT(v.size() == 5);
		BS_TEST_ASSERT(v[0].a == 10);
		BS_TEST_ASSERT(v[3].a == 10);
		BS_TEST_ASSERT(v[3].b == 0);
		BS_TEST_ASSERT(v[4].a == 3);
		BS_TEST_ASSERT(v[4].b == 4);

		// Make a copy
		DynArray<SomeElem> v2 = v;
		BS_TEST_ASSERT(v2.size() == 5);
		BS_TEST_ASSERT(v2[0].a == 10);
		BS_TEST_ASSERT(v2[3].a == 10);
		BS_TEST_ASSERT(v2[3].b == 0);
		BS_TEST_ASSERT(v2[4].a == 3);
		BS_TEST_ASSERT(v2[4].b == 4);

		// Pop an element
		v2.pop();
		BS_TEST_ASSERT(v2.size() == 4);
		BS_TEST_ASSERT(v2[0].a == 10);
		BS_TEST_ASSERT(v2[3].a == 10);
		BS_TEST_ASSERT(v2[3].b == 0);

		// Remove an element
		v.remove(2);
		BS_TEST_ASSERT(v.size() == 4);
		BS_TEST_ASSERT(v[0].a == 10);
		BS_TEST_ASSERT(v[2].a == 10);
		BS_TEST_ASSERT(v[3].a == 3);
		BS_TEST_ASSERT(v[3].b == 4);

		// Insert an element
		v.insert(v.begin() + 2, { 99, 100 });
		BS_TEST_ASSERT(v.size() == 5);
		BS_TEST_ASSERT(v[0].a == 10);
		BS_TEST_ASSERT(v[2].a == 99);
		BS_TEST_ASSERT(v[3].a == 10);
		BS_TEST_ASSERT(v[4].a == 3);
		BS_TEST_ASSERT(v[4].b == 4);

		// Insert a list
		v.insert(v.begin() + 1, {{ 55, 100 }, { 56, 100 }, { 57, 100 }});
		BS_TEST_ASSERT(v.size() == 8);
		BS_TEST_ASSERT(v[0].a == 10);
		BS_TEST_ASSERT(v[1].a == 55);
		BS_TEST_ASSERT(v[2].a == 56);
		BS_TEST_ASSERT(v[3].a == 57);
		BS_TEST_ASSERT(v[4].a == 10);
		BS_TEST_ASSERT(v[5].a == 99);
		BS_TEST_ASSERT(v[6].a == 10);
		BS_TEST_ASSERT(v[7].a == 3);
		BS_TEST_ASSERT(v[7].b == 4);

		// Erase a range of elements
		v.erase(v.begin() + 2, v.begin() + 5);
		BS_TEST_ASSERT(v.size() == 5);
		BS_TEST_ASSERT(v[0].a == 10);
		BS_TEST_ASSERT(v[1].a == 55);
		BS_TEST_ASSERT(v[2].a == 99);
		BS_TEST_ASSERT(v[3].a == 10);
		BS_TEST_ASSERT(v[4].a == 3);
		BS_TEST_ASSERT(v[4].b == 4);

		// Insert a range
		v.insert(v.begin() + 1, v2.begin() + 1, v2.begin() + 3);
		BS_TEST_ASSERT(v.size() == 7);
		BS_TEST_ASSERT(v[0].a == 10);
		BS_TEST_ASSERT(v[1].a == 10);
		BS_TEST_ASSERT(v[2].a == 10);
		BS_TEST_ASSERT(v[3].a == 55);
		BS_TEST_ASSERT(v[4].a == 99);
		BS_TEST_ASSERT(v[5].a == 10);
		BS_TEST_ASSERT(v[6].a == 3);
		BS_TEST_ASSERT(v[6].b == 4);

		// Shrink capacity
		v.shrink();
		BS_TEST_ASSERT(v.size() == v.capacity());
		BS_TEST_ASSERT(v[0].a == 10);
		BS_TEST_ASSERT(v[1].a == 10);
		BS_TEST_ASSERT(v[2].a == 10);
		BS_TEST_ASSERT(v[3].a == 55);
		BS_TEST_ASSERT(v[4].a == 99);
		BS_TEST_ASSERT(v[5].a == 10);
		BS_TEST_ASSERT(v[6].a == 3);
		BS_TEST_ASSERT(v[6].b == 4);

		// Move it
		DynArray<SomeElem> v3 = std::move(v2);
		BS_TEST_ASSERT(v2.size() == 0);
		BS_TEST_ASSERT(v3.size() == 4);
		BS_TEST_ASSERT(v3[0].a == 10);
		BS_TEST_ASSERT(v3[3].a == 10);
		BS_TEST_ASSERT(v3[3].b == 0);
	}
	
	void UtilityTestSuite::testComplex()
	{
		Complex<float> c(10.0, 4.0);
		BS_TEST_ASSERT(c.real() == 10.0);
		BS_TEST_ASSERT(c.imag() == 4.0);

		Complex<float> c2(15.0, 5.0);
		BS_TEST_ASSERT(c2.real() == 15.0);
		BS_TEST_ASSERT(c2.imag() == 5.0);

		Complex<float> c3 = c + c2;
		BS_TEST_ASSERT(c3.real() == 25.0);
		BS_TEST_ASSERT(c3.imag() == 9.0);

		Complex<float> c4 = c - c2;
		BS_TEST_ASSERT(c4.real() == -5.0);
		BS_TEST_ASSERT(c4.imag() == -1.0);

		Complex<float> c5 = c * c2;
		BS_TEST_ASSERT(c5.real() == 130.0);
		BS_TEST_ASSERT(c5.imag() == 110.0);

		Complex<float> c6 = c / c2;
		BS_TEST_ASSERT(c6.real() == 0.680000007f);
		BS_TEST_ASSERT(c6.imag() == 0.0399999991f);

		BS_TEST_ASSERT(Complex<float>::abs(c) == 10.7703295f);
		BS_TEST_ASSERT(Complex<float>::arg(c) == 0.380506366f);
		BS_TEST_ASSERT(Complex<float>::norm(c) == 116);

		Complex<float> c7 = Complex<float>::conj(c);
		BS_TEST_ASSERT(c7.real() == 10);
		BS_TEST_ASSERT(c7.imag() == -4);
		c7 = 0;

		c7 = Complex<float>::polar(2.0, 0.5);
		BS_TEST_ASSERT(c7.real() == 1.75516510f);
		BS_TEST_ASSERT(c7.imag() == 0.958851099f);
		c7 = 0;

		c7 = Complex<float>::cos(c);
		BS_TEST_ASSERT(c7.real() == -22.9135609f);
		BS_TEST_ASSERT(c7.imag() == 14.8462915f);
		c7 = 0;

		c7 = Complex<float>::cosh(c);
		BS_TEST_ASSERT(c7.real() == -7198.72949f);
		BS_TEST_ASSERT(c7.imag() == -8334.84180f);
		c7 = 0;

		c7 = Complex<float>::exp(c);
		BS_TEST_ASSERT(c7.real() == -14397.4580f);
		BS_TEST_ASSERT(c7.imag() == -16669.6836f);
		c7 = 0;

		c7 = Complex<float>::log(c);
		BS_TEST_ASSERT(c7.real() == 2.37679505f);
		BS_TEST_ASSERT(c7.imag() == 0.380506366f);
		c7 = 0;

		c7 = Complex<float>::pow(c, 2.0);
		BS_TEST_ASSERT(c7.real() == 84.0000000f);
		BS_TEST_ASSERT(c7.imag() == 79.9999924f);
		c7 = 0;

		c7 = Complex<float>::sin(c);
		BS_TEST_ASSERT(c7.real() == -14.8562555f);
		BS_TEST_ASSERT(c7.imag() == -22.8981915f);
		c7 = 0;

		c7 = Complex<float>::sinh(c);
		BS_TEST_ASSERT(c7.real() == -7198.72900f);
		BS_TEST_ASSERT(c7.imag() == -8334.84277f);
		c7 = 0;

		c7 = Complex<float>::sqrt(c);
		BS_TEST_ASSERT(c7.real() == 3.22260213f);
		BS_TEST_ASSERT(c7.imag() == 0.620616496f);
		c7 = 0;
	}
	
	void UtilityTestSuite::testMinHeap() 
	{
		struct SomeElem 
		{
			int a;
			int b;
		};

		MinHeap<SomeElem, int> m;
		m.resize(8);
		BS_TEST_ASSERT(m.valid() == true);

		SomeElem elements;
		elements.a = 4;
		elements.b = 5;

		m.insert(elements, 10);
		BS_TEST_ASSERT(m[0].key.a == 4);
		BS_TEST_ASSERT(m[0].key.b == 5);
		BS_TEST_ASSERT(m[0].value == 10);
		BS_TEST_ASSERT(m.size() == 1);

		int v = 11;
		m.insert(elements, v);
		BS_TEST_ASSERT(m[1].key.a == 4);
		BS_TEST_ASSERT(m[1].key.b == 5);
		BS_TEST_ASSERT(m[1].value == 11);
		BS_TEST_ASSERT(m.size() == 2);

		SomeElem minKey;
		int minValue;

		m.minimum(minKey, minValue);
		BS_TEST_ASSERT(minKey.a == 4);
		BS_TEST_ASSERT(minKey.b == 5);
		BS_TEST_ASSERT(minValue == 10);

		m.erase(elements, v);
		BS_TEST_ASSERT(m.size() == 1);
	}

	void UtilityTestSuite::testQuadtree() 
	{
		DebugQuadtreeData quadtreeData;
		DebugQuadtree quadtree(Vector2(0, 0), 800.0f, &quadtreeData);

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
		for (UINT32 i = 0; i < sizeof(types) / sizeof(types[0]); i++)
		{
			for (UINT32 j = 0; j < types[i].count; j++)
			{
				Vector2 position(
					((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents,
					((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents
				);

				Vector2 extents(
					types[i].sizeMin + ((rand() / (float)RAND_MAX)) * (types[i].sizeMax - types[i].sizeMin) * 0.5f,
					types[i].sizeMin + ((rand() / (float)RAND_MAX)) * (types[i].sizeMax - types[i].sizeMin) * 0.5f
				);

				DebugQuadtreeElem elem;
				elem.box = Rect2(position - extents, extents);

				UINT32 elemIdx = (UINT32)quadtreeData.elements.size();
				quadtreeData.elements.push_back(elem);
				quadtree.addElement(elemIdx);
			}
		}

		DebugQuadtreeElem manualElems[3];
		manualElems[0].box = Rect2(Vector2(100.0f, 100.0f), Vector2(110.0f, 115.0f));
		manualElems[1].box = Rect2(Vector2(200.0f, 100.0f), Vector2(250.0f, 150.0f));
		manualElems[2].box = Rect2(Vector2(90.0f, 90.0f), Vector2(105.0f, 105.0f));


		for (UINT32 i = 0; i < 3; i++)
		{
			UINT32 elemIdx = (UINT32)quadtreeData.elements.size();
			quadtreeData.elements.push_back(manualElems[i]);
			quadtree.addElement(elemIdx);
		}

		Rect2 queryBounds = manualElems[0].box;
		DebugQuadtree::BoxIntersectIterator interIter(quadtree, queryBounds);

		Vector<UINT32> overlapElements;
		while (interIter.moveNext())
		{
			UINT32 element = interIter.getElement();
			overlapElements.push_back(element);

			// Manually check for intersections
			assert(quadtreeData.elements[element].box.overlaps(queryBounds));
		}

		// Ensure that all we have found all possible overlaps by manually testing all elements
		UINT32 elemIdx = 0;
		for (auto& entry : quadtreeData.elements)
		{
			if (entry.box.overlaps(queryBounds))
			{
				auto iterFind = std::find(overlapElements.begin(), overlapElements.end(), elemIdx);
				assert(iterFind != overlapElements.end());
			}

			elemIdx++;
		}

		// Ensure nothing goes wrong during element removal
		for (auto& entry : quadtreeData.elements)
			quadtree.removeElement(entry.quadtreeId);
	}

	void UtilityTestSuite::testVarInt()
	{
		UINT32 u0 = 0;
		UINT32 u1 = 127;
		UINT32 u2 = 255;
		UINT32 u3 = 123456;

		INT32 i0 = 0;
		INT32 i1 = 127;
		INT32 i2 = -1;
		INT32 i3 = -123456;
		INT32 i4 = 123456;

		UINT8 output[50];

		UINT32 writeIdx = Bitwise::encodeVarInt(u0, output);
		BS_TEST_ASSERT(writeIdx == 1);

		writeIdx += Bitwise::encodeVarInt(u1, output + writeIdx);
		BS_TEST_ASSERT(writeIdx == 2);

		writeIdx += Bitwise::encodeVarInt(u2, output + writeIdx);
		BS_TEST_ASSERT(writeIdx == 4);

		writeIdx += Bitwise::encodeVarInt(u3, output + writeIdx);

		writeIdx += Bitwise::encodeVarInt(i0, output + writeIdx);
		writeIdx += Bitwise::encodeVarInt(i1, output + writeIdx);
		writeIdx += Bitwise::encodeVarInt(i2, output + writeIdx);
		writeIdx += Bitwise::encodeVarInt(i3, output + writeIdx);
		writeIdx += Bitwise::encodeVarInt(i4, output + writeIdx);

		UINT32 readIdx = 0;
		UINT32 uv;
		readIdx += Bitwise::decodeVarInt(uv, output + readIdx, writeIdx - readIdx);
		BS_TEST_ASSERT(uv == u0);
		BS_TEST_ASSERT(writeIdx > readIdx);

		readIdx += Bitwise::decodeVarInt(uv, output + readIdx, writeIdx - readIdx);
		BS_TEST_ASSERT(uv == u1);
		BS_TEST_ASSERT(writeIdx > readIdx);

		readIdx += Bitwise::decodeVarInt(uv, output + readIdx, writeIdx - readIdx);
		BS_TEST_ASSERT(uv == u2);
		BS_TEST_ASSERT(writeIdx > readIdx);

		readIdx += Bitwise::decodeVarInt(uv, output + readIdx, writeIdx - readIdx);
		BS_TEST_ASSERT(uv == u3);
		BS_TEST_ASSERT(writeIdx > readIdx);

		INT32 iv;
		readIdx += Bitwise::decodeVarInt(iv, output + readIdx, writeIdx - readIdx);
		BS_TEST_ASSERT(iv == i0);
		BS_TEST_ASSERT(writeIdx > readIdx);

		readIdx += Bitwise::decodeVarInt(iv, output + readIdx, writeIdx - readIdx);
		BS_TEST_ASSERT(iv == i1);
		BS_TEST_ASSERT(writeIdx > readIdx);

		readIdx += Bitwise::decodeVarInt(iv, output + readIdx, writeIdx - readIdx);
		BS_TEST_ASSERT(iv == i2);
		BS_TEST_ASSERT(writeIdx > readIdx);

		readIdx += Bitwise::decodeVarInt(iv, output + readIdx, writeIdx - readIdx);
		BS_TEST_ASSERT(iv == i3);
		BS_TEST_ASSERT(writeIdx > readIdx);

		readIdx += Bitwise::decodeVarInt(iv, output + readIdx, writeIdx - readIdx);
		BS_TEST_ASSERT(iv == i4);
		BS_TEST_ASSERT(writeIdx == readIdx);
	}

	void UtilityTestSuite::testBitStream()
	{
		uint32_t v0 = 12345;
		bool v1 = true;
		uint32_t v2 = 67890;
		bool v3 = true;
		bool v4 = false;
		uint32_t v5 = 987;
		String v6 = "Some test string";
		int32_t v7 = -777;
		uint64_t v8 = 1919191919191919ULL;
		float v9 = 0.3333f;
		float v10 = 10.54321f;

		uint64_t v11 = 5555555555ULL;

		Bitstream bs;

		bs.write(v0); // 0  - 32
		bs.write(v1); // 32 - 33
		bs.write(v2); // 33 - 65
		bs.write(v3); // 65 - 66
		bs.write(v4); // 66 - 67

		bs.writeBits((uint8_t*)&v5, 10); // 67 - 77
		bs.write(v6); // 77 - 213
		bs.writeVarInt(v7); // 213 - 229
		bs.writeVarIntDelta(v7, 0); // 229 - 246
		bs.writeVarInt(v8); // 246 - 310
		bs.writeVarIntDelta(v8, v8); // 310 - 311
		bs.writeNorm(v9); // 311 - 327
		bs.writeRange(v10, 5.0f, 15.0f); // 327 - 343
		bs.writeRange(v5, 500U, 1000U); // 343 - 352

		bs.align(); // 352
		bs.write(v11); // 352 - 416

		BS_TEST_ASSERT(bs.size() == 416);

		uint32_t uv;
		uint64_t ulv;
		int32_t iv;
		bool bv;
		float fv;
		String sv;

		bs.seek(0);
		bs.read(uv);
		BS_TEST_ASSERT(uv == v0);

		bs.read(bv);
		BS_TEST_ASSERT(bv == v1);

		bs.read(uv);
		BS_TEST_ASSERT(uv == v2);

		bs.read(bv);
		BS_TEST_ASSERT(bv == v3);

		bs.read(bv);
		BS_TEST_ASSERT(bv == v4);

		uv = 0;
		bs.readBits((uint8_t*)&uv, 10);
		BS_TEST_ASSERT(uv == v5);

		bs.read(sv);
		BS_TEST_ASSERT(sv == v6);

		bs.readVarInt(iv);
		BS_TEST_ASSERT(iv == v7);

		bs.readVarIntDelta(iv, 0);
		BS_TEST_ASSERT(iv == v7);

		bs.readVarInt(ulv);
		BS_TEST_ASSERT(ulv == v8);

		bs.readVarIntDelta(v8, v8);
		BS_TEST_ASSERT(ulv == v8);

		bs.readNorm(fv);
		BS_TEST_ASSERT(Math::approxEquals(fv, v9, 0.01f));

		bs.readRange(fv, 5.0f, 15.0f);
		BS_TEST_ASSERT(Math::approxEquals(fv, v10, 0.01f));

		bs.readRange(uv, 500U, 1000U);
		BS_TEST_ASSERT(uv == v5);

		bs.align();
		bs.read(ulv);
		BS_TEST_ASSERT(ulv == v11);
	}
}
