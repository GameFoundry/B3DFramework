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

using namespace bs;

struct DebugOctreeElem
{
	AABox Box;
	mutable OctreeElementId OctreeId;
};

struct DebugOctreeData
{
	Vector<DebugOctreeElem> Elements;
};

struct DebugOctreeOptions
{
	enum
	{
		LoosePadding = 16
	};

	enum
	{
		MinElementsPerNode = 8
	};

	enum
	{
		MaxElementsPerNode = 16
	};

	enum
	{
		MaxDepth = 12
	};

	static simd::AABox GetBounds(u32 elem, void* context)
	{
		DebugOctreeData* octreeData = (DebugOctreeData*)context;
		return simd::AABox(octreeData->Elements[elem].Box);
	}

	static void SetElementId(u32 elem, const OctreeElementId& id, void* context)
	{
		DebugOctreeData* octreeData = (DebugOctreeData*)context;
		octreeData->Elements[elem].OctreeId = id;
	}
};

typedef Octree<u32, DebugOctreeOptions> DebugOctree;

struct DebugQuadtreeElem
{
	Rect2 Box;
	mutable QuadtreeElementId QuadtreeId;
};

struct DebugQuadtreeData
{
	Vector<DebugQuadtreeElem> Elements;
};

struct DebugQuadtreeOptions
{
	enum
	{
		LoosePadding = 8
	};

	enum
	{
		MinElementsPerNode = 4
	};

	enum
	{
		MaxElementsPerNode = 8
	};

	enum
	{
		MaxDepth = 6
	};

	static simd::Rect2 GetBounds(u32 elem, void* context)
	{
		DebugQuadtreeData* quadtreeData = (DebugQuadtreeData*)context;
		return simd::Rect2(quadtreeData->Elements[elem].Box);
	}

	static void SetElementId(u32 elem, const QuadtreeElementId& id, void* context)
	{
		DebugQuadtreeData* quadtreeData = (DebugQuadtreeData*)context;
		quadtreeData->Elements[elem].QuadtreeId = id;
	}
};

typedef Quadtree<u32, DebugQuadtreeOptions> DebugQuadtree;

void UtilityTestSuite::StartUp()
{
	SPtr<TestSuite> fileSystemTests = Create<FileSystemTestSuite>();
	Add(fileSystemTests);
}

void UtilityTestSuite::ShutDown()
{
}

UtilityTestSuite::UtilityTestSuite()
{
	B3D_ADD_TEST(UtilityTestSuite::TestOctree);
	B3D_ADD_TEST(UtilityTestSuite::TestBitfield);
	B3D_ADD_TEST(UtilityTestSuite::TestSmallVector);
	B3D_ADD_TEST(UtilityTestSuite::TestDynArray);
	B3D_ADD_TEST(UtilityTestSuite::TestComplex);
	B3D_ADD_TEST(UtilityTestSuite::TestMinHeap);
	B3D_ADD_TEST(UtilityTestSuite::TestQuadtree)
	B3D_ADD_TEST(UtilityTestSuite::TestVarInt)
	B3D_ADD_TEST(UtilityTestSuite::TestBitStream)
}

void UtilityTestSuite::TestBitfield()
{
	static constexpr u32 kCount = 100;
	static constexpr u32 kExtraCount = 32;

	Bitfield bitfield(true, kCount);

	// Basic iteration
	u32 i = 0;
	for(auto iter : bitfield)
	{
		B3D_TEST_ASSERT(iter == true)
		i++;
	}

	u32 curCount = kCount;
	B3D_TEST_ASSERT(i == curCount);

	// Dynamic additon
	bitfield.Add(false);
	bitfield.Add(false);
	bitfield.Add(true);
	bitfield.Add(false);
	curCount += 4;

	// Realloc
	curCount += kExtraCount;
	for(uint32_t j = 0; j < 32; j++)
		bitfield.Add(false);

	B3D_TEST_ASSERT(bitfield.Size() == curCount);

	B3D_TEST_ASSERT(bitfield[kCount + 0] == false);
	B3D_TEST_ASSERT(bitfield[kCount + 1] == false);
	B3D_TEST_ASSERT(bitfield[kCount + 2] == true);
	B3D_TEST_ASSERT(bitfield[kCount + 3] == false);

	// Modify during iteration
	i = 0;
	for(auto iter : bitfield)
	{
		if(i >= 50 && i <= 70)
			iter = false;

		i++;
	}

	// Modify directly using []
	bitfield[5] = false;
	bitfield[6] = false;

	for(u32 j = 50; j < 70; j++)
		B3D_TEST_ASSERT(bitfield[j] == false);

	B3D_TEST_ASSERT(bitfield[5] == false);
	B3D_TEST_ASSERT(bitfield[6] == false);

	// Removal
	bitfield.Remove(10);
	bitfield.Remove(10);
	curCount -= 2;

	for(u32 j = 48; j < 68; j++)
		B3D_TEST_ASSERT(bitfield[j] == false);

	B3D_TEST_ASSERT(bitfield[5] == false);
	B3D_TEST_ASSERT(bitfield[6] == false);

	B3D_TEST_ASSERT(bitfield.Size() == curCount);

	// Find
	B3D_TEST_ASSERT(bitfield.Find(true) == 0);
	B3D_TEST_ASSERT(bitfield.Find(false) == 5);
}

void UtilityTestSuite::TestOctree()
{
	DebugOctreeData octreeData;
	DebugOctree octree(Vector3::kZero, 800.0f, &octreeData);

	struct SizeAndCount
	{
		float SizeMin;
		float SizeMax;
		u32 Count;
	};

	SizeAndCount types[]{
		{ 0.02f, 0.2f, 2000 }, // Very small objects
		{ 0.2f, 1.0f, 2000 }, // Small objects
		{ 1.0f, 5.0f, 5000 }, // Medium sized objects
		{ 5.0f, 30.0f, 4000 }, // Large objects
		{ 30.0f, 100.0f, 2000 } // Very large objects
	};

	float placementExtents = 750.0f;
	for(u32 i = 0; i < sizeof(types) / sizeof(types[0]); i++)
	{
		for(u32 j = 0; j < types[i].Count; j++)
		{
			Vector3 position(
				((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents,
				((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents,
				((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents);

			Vector3 extents(
				types[i].SizeMin + ((rand() / (float)RAND_MAX)) * (types[i].SizeMax - types[i].SizeMin) * 0.5f,
				types[i].SizeMin + ((rand() / (float)RAND_MAX)) * (types[i].SizeMax - types[i].SizeMin) * 0.5f,
				types[i].SizeMin + ((rand() / (float)RAND_MAX)) * (types[i].SizeMax - types[i].SizeMin) * 0.5f);

			DebugOctreeElem elem;
			elem.Box = AABox(position - extents, position + extents);

			u32 elemIdx = (u32)octreeData.Elements.size();
			octreeData.Elements.push_back(elem);
			octree.AddElement(elemIdx);
		}
	}

	DebugOctreeElem manualElems[3];
	manualElems[0].Box = AABox(Vector3(100.0f, 100.0f, 100.f), Vector3(110.0f, 115.0f, 110.0f));
	manualElems[1].Box = AABox(Vector3(200.0f, 100.0f, 100.f), Vector3(250.0f, 150.0f, 150.0f));
	manualElems[2].Box = AABox(Vector3(90.0f, 90.0f, 90.f), Vector3(105.0f, 105.0f, 110.0f));

	for(u32 i = 0; i < 3; i++)
	{
		u32 elemIdx = (u32)octreeData.Elements.size();
		octreeData.Elements.push_back(manualElems[i]);
		octree.AddElement(elemIdx);
	}

	AABox queryBounds = manualElems[0].Box;
	DebugOctree::BoxIntersectIterator interIter(octree, queryBounds);

	Vector<u32> overlapElements;
	while(interIter.MoveNext())
	{
		u32 element = interIter.GetElement();
		overlapElements.push_back(element);

		// Manually check for intersections
		B3D_TEST_ASSERT(octreeData.Elements[element].Box.Intersects(queryBounds));
	}

	// Ensure that all we have found all possible overlaps by manually testing all elements
	u32 elemIdx = 0;
	for(auto& entry : octreeData.Elements)
	{
		if(entry.Box.Intersects(queryBounds))
		{
			auto iterFind = std::find(overlapElements.begin(), overlapElements.end(), elemIdx);
			B3D_TEST_ASSERT(iterFind != overlapElements.end());
		}

		elemIdx++;
	}

	// Ensure nothing goes wrong during element removal
	for(auto& entry : octreeData.Elements)
		octree.RemoveElement(entry.OctreeId);
}

void UtilityTestSuite::TestSmallVector()
{
	struct SomeElem
	{
		int A = 10;
		int B = 0;
	};

	// Make sure initial construction works
	SmallVector<SomeElem, 4> v(4);
	B3D_TEST_ASSERT(v.size() == 4);
	B3D_TEST_ASSERT(v.capacity() == 4);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[3].A == 10);
	B3D_TEST_ASSERT(v[3].B == 0);

	// Making the vector dynamic
	v.Add({ 3, 4 });
	B3D_TEST_ASSERT(v.size() == 5);
	B3D_TEST_ASSERT(v[0].a == 10);
	B3D_TEST_ASSERT(v[3].a == 10);
	B3D_TEST_ASSERT(v[3].b == 0);
	B3D_TEST_ASSERT(v[4].a == 3);
	B3D_TEST_ASSERT(v[4].b == 4);

	// Make a copy
	SmallVector<SomeElem, 4> v2 = v;
	B3D_TEST_ASSERT(v2.size() == 5);
	B3D_TEST_ASSERT(v2[0].a == 10);
	B3D_TEST_ASSERT(v2[3].a == 10);
	B3D_TEST_ASSERT(v2[3].b == 0);
	B3D_TEST_ASSERT(v2[4].a == 3);
	B3D_TEST_ASSERT(v2[4].b == 4);

	// Pop an element
	v2.Pop();
	B3D_TEST_ASSERT(v2.size() == 4);
	B3D_TEST_ASSERT(v2[0].a == 10);
	B3D_TEST_ASSERT(v2[3].a == 10);
	B3D_TEST_ASSERT(v2[3].b == 0);

	// Make a static only copy
	SmallVector<SomeElem, 4> v3 = v2;
	B3D_TEST_ASSERT(v3.size() == 4);
	B3D_TEST_ASSERT(v3.capacity() == 4);
	B3D_TEST_ASSERT(v3[0].a == 10);
	B3D_TEST_ASSERT(v3[3].a == 10);
	B3D_TEST_ASSERT(v3[3].b == 0);

	// Remove an element
	v.Remove(2);
	B3D_TEST_ASSERT(v.size() == 4);
	B3D_TEST_ASSERT(v[0].a == 10);
	B3D_TEST_ASSERT(v[2].a == 10);
	B3D_TEST_ASSERT(v[3].a == 3);
	B3D_TEST_ASSERT(v[3].b == 4);

	// Move a static vector
	SmallVector<SomeElem, 4> v4 = std::move(v3);
	B3D_TEST_ASSERT(v3.size() == 0);
	B3D_TEST_ASSERT(v4.size() == 4);
	B3D_TEST_ASSERT(v4.capacity() == 4);
	B3D_TEST_ASSERT(v4[0].a == 10);
	B3D_TEST_ASSERT(v4[3].a == 10);
	B3D_TEST_ASSERT(v4[3].b == 0);

	// Move a dynamic vector
	SmallVector<SomeElem, 4> v5 = std::move(v2);
	B3D_TEST_ASSERT(v2.size() == 0);
	B3D_TEST_ASSERT(v5.size() == 4);
	B3D_TEST_ASSERT(v5[0].a == 10);
	B3D_TEST_ASSERT(v5[3].a == 10);
	B3D_TEST_ASSERT(v5[3].b == 0);

	// Move a dynamic vector into a dynamic vector
	v.Add({ 33, 44 });
	SmallVector<SomeElem, 4> v6 = std::move(v);
	B3D_TEST_ASSERT(v.size() == 0);
	B3D_TEST_ASSERT(v6.size() == 5);
	B3D_TEST_ASSERT(v6[0].a == 10);
	B3D_TEST_ASSERT(v6[3].a == 3);
	B3D_TEST_ASSERT(v6[3].b == 4);
	B3D_TEST_ASSERT(v6[4].a == 33);
	B3D_TEST_ASSERT(v6[4].b == 44);
}

void UtilityTestSuite::TestDynArray()
{
	struct SomeElem
	{
		int A = 10;
		int B = 0;
	};

	// Make sure initial construction works
	DynArray<SomeElem> v(4);
	B3D_TEST_ASSERT(v.Size() == 4);
	B3D_TEST_ASSERT(v.Capacity() == 4);
	B3D_TEST_ASSERT(v[0].a == 10);
	B3D_TEST_ASSERT(v[3].a == 10);
	B3D_TEST_ASSERT(v[3].b == 0);

	// Add an element
	v.Add({ 3, 4 });
	B3D_TEST_ASSERT(v.Size() == 5);
	B3D_TEST_ASSERT(v[0].a == 10);
	B3D_TEST_ASSERT(v[3].a == 10);
	B3D_TEST_ASSERT(v[3].b == 0);
	B3D_TEST_ASSERT(v[4].a == 3);
	B3D_TEST_ASSERT(v[4].b == 4);

	// Make a copy
	DynArray<SomeElem> v2 = v;
	B3D_TEST_ASSERT(v2.Size() == 5);
	B3D_TEST_ASSERT(v2[0].a == 10);
	B3D_TEST_ASSERT(v2[3].a == 10);
	B3D_TEST_ASSERT(v2[3].b == 0);
	B3D_TEST_ASSERT(v2[4].a == 3);
	B3D_TEST_ASSERT(v2[4].b == 4);

	// Pop an element
	v2.Pop();
	B3D_TEST_ASSERT(v2.Size() == 4);
	B3D_TEST_ASSERT(v2[0].a == 10);
	B3D_TEST_ASSERT(v2[3].a == 10);
	B3D_TEST_ASSERT(v2[3].b == 0);

	// Remove an element
	v.Remove(2);
	B3D_TEST_ASSERT(v.Size() == 4);
	B3D_TEST_ASSERT(v[0].a == 10);
	B3D_TEST_ASSERT(v[2].a == 10);
	B3D_TEST_ASSERT(v[3].a == 3);
	B3D_TEST_ASSERT(v[3].b == 4);

	// Insert an element
	v.Insert(v.begin() + 2, { 99, 100 });
	B3D_TEST_ASSERT(v.Size() == 5);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[2].A == 99);
	B3D_TEST_ASSERT(v[3].A == 10);
	B3D_TEST_ASSERT(v[4].a == 3);
	B3D_TEST_ASSERT(v[4].b == 4);

	// Insert a list
	v.Insert(v.begin() + 1, { { 55, 100 }, { 56, 100 }, { 57, 100 } });
	B3D_TEST_ASSERT(v.Size() == 8);
	B3D_TEST_ASSERT(v[0].a == 10);
	B3D_TEST_ASSERT(v[1].a == 55);
	B3D_TEST_ASSERT(v[2].a == 56);
	B3D_TEST_ASSERT(v[3].a == 57);
	B3D_TEST_ASSERT(v[4].a == 10);
	B3D_TEST_ASSERT(v[5].a == 99);
	B3D_TEST_ASSERT(v[6].a == 10);
	B3D_TEST_ASSERT(v[7].a == 3);
	B3D_TEST_ASSERT(v[7].b == 4);

	// Erase a range of elements
	v.Erase(v.begin() + 2, v.begin() + 5);
	B3D_TEST_ASSERT(v.Size() == 5);
	B3D_TEST_ASSERT(v[0].a == 10);
	B3D_TEST_ASSERT(v[1].a == 55);
	B3D_TEST_ASSERT(v[2].a == 99);
	B3D_TEST_ASSERT(v[3].a == 10);
	B3D_TEST_ASSERT(v[4].a == 3);
	B3D_TEST_ASSERT(v[4].b == 4);

	// Insert a range
	v.Insert(v.begin() + 1, v2.begin() + 1, v2.begin() + 3);
	B3D_TEST_ASSERT(v.Size() == 7);
	B3D_TEST_ASSERT(v[0].a == 10);
	B3D_TEST_ASSERT(v[1].a == 10);
	B3D_TEST_ASSERT(v[2].a == 10);
	B3D_TEST_ASSERT(v[3].a == 55);
	B3D_TEST_ASSERT(v[4].a == 99);
	B3D_TEST_ASSERT(v[5].a == 10);
	B3D_TEST_ASSERT(v[6].a == 3);
	B3D_TEST_ASSERT(v[6].b == 4);

	// Shrink capacity
	v.Shrink();
	B3D_TEST_ASSERT(v.Size() == v.Capacity());
	B3D_TEST_ASSERT(v[0].a == 10);
	B3D_TEST_ASSERT(v[1].a == 10);
	B3D_TEST_ASSERT(v[2].a == 10);
	B3D_TEST_ASSERT(v[3].a == 55);
	B3D_TEST_ASSERT(v[4].a == 99);
	B3D_TEST_ASSERT(v[5].a == 10);
	B3D_TEST_ASSERT(v[6].a == 3);
	B3D_TEST_ASSERT(v[6].b == 4);

	// Move it
	DynArray<SomeElem> v3 = std::move(v2);
	B3D_TEST_ASSERT(v2.Size() == 0);
	B3D_TEST_ASSERT(v3.Size() == 4);
	B3D_TEST_ASSERT(v3[0].a == 10);
	B3D_TEST_ASSERT(v3[3].a == 10);
	B3D_TEST_ASSERT(v3[3].b == 0);
}

void UtilityTestSuite::TestComplex()
{
	Complex<float> c(10.0, 4.0);
	B3D_TEST_ASSERT(c.real() == 10.0);
	B3D_TEST_ASSERT(c.imag() == 4.0);

	Complex<float> c2(15.0, 5.0);
	B3D_TEST_ASSERT(c2.real() == 15.0);
	B3D_TEST_ASSERT(c2.imag() == 5.0);

	Complex<float> c3 = c + c2;
	B3D_TEST_ASSERT(c3.real() == 25.0);
	B3D_TEST_ASSERT(c3.imag() == 9.0);

	Complex<float> c4 = c - c2;
	B3D_TEST_ASSERT(c4.real() == -5.0);
	B3D_TEST_ASSERT(c4.imag() == -1.0);

	Complex<float> c5 = c * c2;
	B3D_TEST_ASSERT(c5.real() == 130.0);
	B3D_TEST_ASSERT(c5.imag() == 110.0);

	Complex<float> c6 = c / c2;
	B3D_TEST_ASSERT(c6.real() == 0.680000007f);
	B3D_TEST_ASSERT(c6.imag() == 0.0399999991f);

	B3D_TEST_ASSERT(Complex<float>::abs(c) == 10.7703295f);
	B3D_TEST_ASSERT(Complex<float>::arg(c) == 0.380506366f);
	B3D_TEST_ASSERT(Complex<float>::norm(c) == 116);

	Complex<float> c7 = Complex<float>::conj(c);
	B3D_TEST_ASSERT(c7.real() == 10);
	B3D_TEST_ASSERT(c7.imag() == -4);
	c7 = 0;

	c7 = Complex<float>::polar(2.0, 0.5);
	B3D_TEST_ASSERT(c7.real() == 1.75516510f);
	B3D_TEST_ASSERT(c7.imag() == 0.958851099f);
	c7 = 0;

	c7 = Complex<float>::cos(c);
	B3D_TEST_ASSERT(c7.real() == -22.9135609f);
	B3D_TEST_ASSERT(c7.imag() == 14.8462915f);
	c7 = 0;

	c7 = Complex<float>::cosh(c);
	B3D_TEST_ASSERT(c7.real() == -7198.72949f);
	B3D_TEST_ASSERT(c7.imag() == -8334.84180f);
	c7 = 0;

	c7 = Complex<float>::exp(c);
	B3D_TEST_ASSERT(c7.real() == -14397.4580f);
	B3D_TEST_ASSERT(c7.imag() == -16669.6836f);
	c7 = 0;

	c7 = Complex<float>::log(c);
	B3D_TEST_ASSERT(c7.real() == 2.37679505f);
	B3D_TEST_ASSERT(c7.imag() == 0.380506366f);
	c7 = 0;

	c7 = Complex<float>::pow(c, 2.0);
	B3D_TEST_ASSERT(c7.real() == 84.0000000f);
	B3D_TEST_ASSERT(c7.imag() == 79.9999924f);
	c7 = 0;

	c7 = Complex<float>::sin(c);
	B3D_TEST_ASSERT(c7.real() == -14.8562555f);
	B3D_TEST_ASSERT(c7.imag() == -22.8981915f);
	c7 = 0;

	c7 = Complex<float>::sinh(c);
	B3D_TEST_ASSERT(c7.real() == -7198.72900f);
	B3D_TEST_ASSERT(c7.imag() == -8334.84277f);
	c7 = 0;

	c7 = Complex<float>::sqrt(c);
	B3D_TEST_ASSERT(c7.real() == 3.22260213f);
	B3D_TEST_ASSERT(c7.imag() == 0.620616496f);
	c7 = 0;
}

void UtilityTestSuite::TestMinHeap()
{
	struct SomeElem
	{
		int A;
		int B;
	};

	MinHeap<SomeElem, int> m;
	m.resize(8);
	B3D_TEST_ASSERT(m.valid() == true);

	SomeElem elements;
	elements.A = 4;
	elements.B = 5;

	m.insert(elements, 10);
	B3D_TEST_ASSERT(m[0].key.A == 4);
	B3D_TEST_ASSERT(m[0].key.B == 5);
	B3D_TEST_ASSERT(m[0].value == 10);
	B3D_TEST_ASSERT(m.size() == 1);

	int v = 11;
	m.insert(elements, v);
	B3D_TEST_ASSERT(m[1].key.A == 4);
	B3D_TEST_ASSERT(m[1].key.B == 5);
	B3D_TEST_ASSERT(m[1].value == 11);
	B3D_TEST_ASSERT(m.size() == 2);

	SomeElem minKey;
	int minValue;

	m.minimum(minKey, minValue);
	B3D_TEST_ASSERT(minKey.A == 4);
	B3D_TEST_ASSERT(minKey.B == 5);
	B3D_TEST_ASSERT(minValue == 10);

	m.erase(elements, v);
	B3D_TEST_ASSERT(m.size() == 1);
}

void UtilityTestSuite::TestQuadtree()
{
	DebugQuadtreeData quadtreeData;
	DebugQuadtree quadtree(Vector2(0, 0), 800.0f, &quadtreeData);

	struct SizeAndCount
	{
		float SizeMin;
		float SizeMax;
		u32 Count;
	};

	SizeAndCount types[]{
		{ 0.02f, 0.2f, 2000 }, // Very small objects
		{ 0.2f, 1.0f, 2000 }, // Small objects
		{ 1.0f, 5.0f, 5000 }, // Medium sized objects
		{ 5.0f, 30.0f, 4000 }, // Large objects
		{ 30.0f, 100.0f, 2000 } // Very large objects
	};

	float placementExtents = 750.0f;
	for(u32 i = 0; i < sizeof(types) / sizeof(types[0]); i++)
	{
		for(u32 j = 0; j < types[i].Count; j++)
		{
			Vector2 position(
				((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents,
				((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents);

			Vector2 extents(
				types[i].SizeMin + ((rand() / (float)RAND_MAX)) * (types[i].SizeMax - types[i].SizeMin) * 0.5f,
				types[i].SizeMin + ((rand() / (float)RAND_MAX)) * (types[i].SizeMax - types[i].SizeMin) * 0.5f);

			DebugQuadtreeElem elem;
			elem.Box = Rect2(position - extents, extents);

			u32 elemIdx = (u32)quadtreeData.Elements.size();
			quadtreeData.Elements.push_back(elem);
			quadtree.AddElement(elemIdx);
		}
	}

	DebugQuadtreeElem manualElems[3];
	manualElems[0].Box = Rect2(Vector2(100.0f, 100.0f), Vector2(110.0f, 115.0f));
	manualElems[1].Box = Rect2(Vector2(200.0f, 100.0f), Vector2(250.0f, 150.0f));
	manualElems[2].Box = Rect2(Vector2(90.0f, 90.0f), Vector2(105.0f, 105.0f));

	for(u32 i = 0; i < 3; i++)
	{
		u32 elemIdx = (u32)quadtreeData.Elements.size();
		quadtreeData.Elements.push_back(manualElems[i]);
		quadtree.AddElement(elemIdx);
	}

	Rect2 queryBounds = manualElems[0].Box;
	DebugQuadtree::BoxIntersectIterator interIter(quadtree, queryBounds);

	Vector<u32> overlapElements;
	while(interIter.moveNext())
	{
		u32 element = interIter.getElement();
		overlapElements.push_back(element);

		// Manually check for intersections
		B3D_ASSERT(quadtreeData.Elements[element].Box.Overlaps(queryBounds));
	}

	// Ensure that all we have found all possible overlaps by manually testing all elements
	u32 elemIdx = 0;
	for(auto& entry : quadtreeData.Elements)
	{
		if(entry.Box.Overlaps(queryBounds))
		{
			auto iterFind = std::find(overlapElements.begin(), overlapElements.end(), elemIdx);
			B3D_ASSERT(iterFind != overlapElements.end());
		}

		elemIdx++;
	}

	// Ensure nothing goes wrong during element removal
	for(auto& entry : quadtreeData.Elements)
		quadtree.removeElement(entry.quadtreeId);
}

void UtilityTestSuite::TestVarInt()
{
	u32 u0 = 0;
	u32 u1 = 127;
	u32 u2 = 255;
	u32 u3 = 123456;

	i32 i0 = 0;
	i32 i1 = 127;
	i32 i2 = -1;
	i32 i3 = -123456;
	i32 i4 = 123456;

	u8 output[50];

	u32 writeIdx = Bitwise::EncodeVarInt(u0, output);
	B3D_TEST_ASSERT(writeIdx == 1);

	writeIdx += Bitwise::EncodeVarInt(u1, output + writeIdx);
	B3D_TEST_ASSERT(writeIdx == 2);

	writeIdx += Bitwise::EncodeVarInt(u2, output + writeIdx);
	B3D_TEST_ASSERT(writeIdx == 4);

	writeIdx += Bitwise::EncodeVarInt(u3, output + writeIdx);

	writeIdx += Bitwise::EncodeVarInt(i0, output + writeIdx);
	writeIdx += Bitwise::EncodeVarInt(i1, output + writeIdx);
	writeIdx += Bitwise::EncodeVarInt(i2, output + writeIdx);
	writeIdx += Bitwise::EncodeVarInt(i3, output + writeIdx);
	writeIdx += Bitwise::EncodeVarInt(i4, output + writeIdx);

	u32 readIdx = 0;
	u32 uv;
	readIdx += Bitwise::DecodeVarInt(uv, output + readIdx, writeIdx - readIdx);
	B3D_TEST_ASSERT(uv == u0);
	B3D_TEST_ASSERT(writeIdx > readIdx);

	readIdx += Bitwise::DecodeVarInt(uv, output + readIdx, writeIdx - readIdx);
	B3D_TEST_ASSERT(uv == u1);
	B3D_TEST_ASSERT(writeIdx > readIdx);

	readIdx += Bitwise::DecodeVarInt(uv, output + readIdx, writeIdx - readIdx);
	B3D_TEST_ASSERT(uv == u2);
	B3D_TEST_ASSERT(writeIdx > readIdx);

	readIdx += Bitwise::DecodeVarInt(uv, output + readIdx, writeIdx - readIdx);
	B3D_TEST_ASSERT(uv == u3);
	B3D_TEST_ASSERT(writeIdx > readIdx);

	i32 iv;
	readIdx += Bitwise::DecodeVarInt(iv, output + readIdx, writeIdx - readIdx);
	B3D_TEST_ASSERT(iv == i0);
	B3D_TEST_ASSERT(writeIdx > readIdx);

	readIdx += Bitwise::DecodeVarInt(iv, output + readIdx, writeIdx - readIdx);
	B3D_TEST_ASSERT(iv == i1);
	B3D_TEST_ASSERT(writeIdx > readIdx);

	readIdx += Bitwise::DecodeVarInt(iv, output + readIdx, writeIdx - readIdx);
	B3D_TEST_ASSERT(iv == i2);
	B3D_TEST_ASSERT(writeIdx > readIdx);

	readIdx += Bitwise::DecodeVarInt(iv, output + readIdx, writeIdx - readIdx);
	B3D_TEST_ASSERT(iv == i3);
	B3D_TEST_ASSERT(writeIdx > readIdx);

	readIdx += Bitwise::DecodeVarInt(iv, output + readIdx, writeIdx - readIdx);
	B3D_TEST_ASSERT(iv == i4);
	B3D_TEST_ASSERT(writeIdx == readIdx);
}

void UtilityTestSuite::TestBitStream()
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

	bs.Write(v0); // 0  - 32
	bs.Write(v1); // 32 - 33
	bs.Write(v2); // 33 - 65
	bs.Write(v3); // 65 - 66
	bs.Write(v4); // 66 - 67

	bs.WriteBits((uint8_t*)&v5, 10); // 67 - 77
	bs.Write(v6); // 77 - 213
	bs.WriteVarInt(v7); // 213 - 229
	bs.WriteVarIntDelta(v7, 0); // 229 - 246
	bs.WriteVarInt(v8); // 246 - 310
	bs.WriteVarIntDelta(v8, v8); // 310 - 311
	bs.WriteNorm(v9); // 311 - 327
	bs.WriteRange(v10, 5.0f, 15.0f); // 327 - 343
	bs.WriteRange(v5, 500U, 1000U); // 343 - 352

	bs.Align(); // 352
	bs.Write(v11); // 352 - 416

	B3D_TEST_ASSERT(bs.Size() == 416);

	uint32_t uv;
	uint64_t ulv;
	int32_t iv;
	bool bv;
	float fv;
	String sv;

	bs.Seek(0);
	bs.Read(uv);
	B3D_TEST_ASSERT(uv == v0);

	bs.Read(bv);
	B3D_TEST_ASSERT(bv == v1);

	bs.read(uv);
	B3D_TEST_ASSERT(uv == v2);

	bs.read(bv);
	B3D_TEST_ASSERT(bv == v3);

	bs.read(bv);
	B3D_TEST_ASSERT(bv == v4);

	uv = 0;
	bs.readBits((uint8_t*)&uv, 10);
	B3D_TEST_ASSERT(uv == v5);

	bs.read(sv);
	B3D_TEST_ASSERT(sv == v6);

	bs.readVarInt(iv);
	B3D_TEST_ASSERT(iv == v7);

	bs.readVarIntDelta(iv, 0);
	B3D_TEST_ASSERT(iv == v7);

	bs.readVarInt(ulv);
	B3D_TEST_ASSERT(ulv == v8);

	bs.readVarIntDelta(v8, v8);
	B3D_TEST_ASSERT(ulv == v8);

	bs.readNorm(fv);
	B3D_TEST_ASSERT(Math::ApproxEquals(fv, v9, 0.01f));

	bs.readRange(fv, 5.0f, 15.0f);
	B3D_TEST_ASSERT(Math::ApproxEquals(fv, v10, 0.01f));

	bs.readRange(uv, 500U, 1000U);
	B3D_TEST_ASSERT(uv == v5);

	bs.align();
	bs.read(ulv);
	B3D_TEST_ASSERT(ulv == v11);
}
