//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUtilityTestSuite.h"

#include "B3DECSTestSuite.h"
#include "B3DFileSystemTestSuite.h"
#include "Utility/B3DBitfield.h"
#include "Utility/B3DTArray.h"
#include "Math/B3DComplex.h"
#include "Math/B3DMath.h"
#include "Reflection/B3DRTTIIterator.h"
#include "String/B3DHashedString.h"
#include "Threading/B3DSignal.h"
#include "Threading/B3DThreadPool.h"
#include "Utility/B3DMinHeap.h"
#include "Utility/B3DSpatialTree.h"
#include "Utility/B3DBitstream.h"
#include "Utility/B3DQueue.h"
#include "Threading/B3DThread.h"

using namespace b3d;

struct DebugOctreeElement
{
	AABox Box;
	mutable SpatialTreeElementId OctreeId;
};

struct DebugOctreeData
{
	Vector<DebugOctreeElement> Elements;
};

struct DebugOctreeOptions
{
	enum
	{
		LoosePadding = 16,
		MinimumElementsPerNode = 8,
		MaximumElementsPerNode = 16,
		MaximumDepth = 12
	};

	static simd::AABox GetBounds(u32 elementIndex, void* context)
	{
		DebugOctreeData* octreeData = (DebugOctreeData*)context;
		return simd::AABox(octreeData->Elements[elementIndex].Box);
	}

	static void SetElementId(u32 elementIndex, const SpatialTreeElementId& id, void* context)
	{
		DebugOctreeData* octreeData = (DebugOctreeData*)context;
		octreeData->Elements[elementIndex].OctreeId = id;
	}
};

typedef TOctTree<u32, DebugOctreeOptions> DebugOctree;

struct DebugQuadtreeElement
{
	Area2 Box;
	mutable SpatialTreeElementId QuadtreeId;
};

struct DebugQuadtreeData
{
	Vector<DebugQuadtreeElement> Elements;
};

struct DebugQuadtreeOptions
{
	enum
	{
		LoosePadding = 8,
		MinimumElementsPerNode = 4,
		MaximumElementsPerNode = 8,
		MaximumDepth = 6,
	};

	static simd::Area2 GetBounds(u32 elem, void* context)
	{
		DebugQuadtreeData* quadtreeData = (DebugQuadtreeData*)context;
		return simd::Area2(quadtreeData->Elements[elem].Box);
	}

	static void SetElementId(u32 elem, const SpatialTreeElementId& id, void* context)
	{
		DebugQuadtreeData* quadtreeData = (DebugQuadtreeData*)context;
		quadtreeData->Elements[elem].QuadtreeId = id;
	}
};

typedef TQuadTree<u32, DebugQuadtreeOptions> DebugQuadtree;

UtilityTestSuite::UtilityTestSuite()
	: TestSuite("UtilityTestSuite")
{
	B3D_ADD_TEST(UtilityTestSuite::TestOctree);
	B3D_ADD_TEST(UtilityTestSuite::TestBitfield);
	B3D_ADD_TEST(UtilityTestSuite::TestInlineArray);
	B3D_ADD_TEST(UtilityTestSuite::TestArray);
	B3D_ADD_TEST(UtilityTestSuite::TestComplex);
	B3D_ADD_TEST(UtilityTestSuite::TestMinHeap);
	B3D_ADD_TEST(UtilityTestSuite::TestQuadtree)
	B3D_ADD_TEST(UtilityTestSuite::TestVarInt)
	B3D_ADD_TEST(UtilityTestSuite::TestBitStream)
	B3D_ADD_TEST(UtilityTestSuite::TestRTTIIterator)
	B3D_ADD_TEST(UtilityTestSuite::TestMPSCQueue)
	B3D_ADD_TEST(UtilityTestSuite::TestSPSCQueue)
	B3D_ADD_TEST(UtilityTestSuite::TestHashedString)
}

void UtilityTestSuite::TestBitfield()
{
	static constexpr u32 kCount = 100;
	static constexpr u32 kExtraCount = 32;

	TBitfield bitfield(true, kCount);

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

			DebugOctreeElement elem;
			elem.Box = AABox(position - extents, position + extents);

			u32 elemIdx = (u32)octreeData.Elements.size();
			octreeData.Elements.push_back(elem);
			octree.AddElement(elemIdx);
		}
	}

	DebugOctreeElement manualElems[3];
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
	DebugOctree::AreaIntersectIterator interIter(octree, queryBounds);

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

void UtilityTestSuite::TestInlineArray()
{
	struct SomeElem
	{
		int A = 10;
		int B = 0;
	};

	// Make sure initial construction works
	TInlineArray<SomeElem, 4> v(4);
	B3D_TEST_ASSERT(v.size() == 4);
	B3D_TEST_ASSERT(v.capacity() == 4);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[3].A == 10);
	B3D_TEST_ASSERT(v[3].B == 0);

	// Making the vector dynamic
	v.Add({ 3, 4 });
	B3D_TEST_ASSERT(v.size() == 5);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[3].A == 10);
	B3D_TEST_ASSERT(v[3].B == 0);
	B3D_TEST_ASSERT(v[4].A == 3);
	B3D_TEST_ASSERT(v[4].B == 4);

	// Make a copy
	TInlineArray<SomeElem, 4> v2 = v;
	B3D_TEST_ASSERT(v2.size() == 5);
	B3D_TEST_ASSERT(v2[0].A == 10);
	B3D_TEST_ASSERT(v2[3].A == 10);
	B3D_TEST_ASSERT(v2[3].B == 0);
	B3D_TEST_ASSERT(v2[4].A == 3);
	B3D_TEST_ASSERT(v2[4].B == 4);

	// Pop an element
	v2.Pop();
	B3D_TEST_ASSERT(v2.size() == 4);
	B3D_TEST_ASSERT(v2[0].A == 10);
	B3D_TEST_ASSERT(v2[3].A == 10);
	B3D_TEST_ASSERT(v2[3].B == 0);

	// Make a static only copy
	TInlineArray<SomeElem, 4> v3 = v2;
	B3D_TEST_ASSERT(v3.size() == 4);
	B3D_TEST_ASSERT(v3.capacity() == 4);
	B3D_TEST_ASSERT(v3[0].A == 10);
	B3D_TEST_ASSERT(v3[3].A == 10);
	B3D_TEST_ASSERT(v3[3].B == 0);

	// Remove an element
	v.Remove(2);
	B3D_TEST_ASSERT(v.size() == 4);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[2].A == 10);
	B3D_TEST_ASSERT(v[3].A == 3);
	B3D_TEST_ASSERT(v[3].B == 4);

	// Move a static vector
	TInlineArray<SomeElem, 4> v4 = std::move(v3);
	B3D_TEST_ASSERT(v3.size() == 0);
	B3D_TEST_ASSERT(v4.size() == 4);
	B3D_TEST_ASSERT(v4.capacity() == 4);
	B3D_TEST_ASSERT(v4[0].A == 10);
	B3D_TEST_ASSERT(v4[3].A == 10);
	B3D_TEST_ASSERT(v4[3].B == 0);

	// Move a dynamic vector
	TInlineArray<SomeElem, 4> v5 = std::move(v2);
	B3D_TEST_ASSERT(v2.size() == 0);
	B3D_TEST_ASSERT(v5.size() == 4);
	B3D_TEST_ASSERT(v5[0].A == 10);
	B3D_TEST_ASSERT(v5[3].A == 10);
	B3D_TEST_ASSERT(v5[3].B == 0);

	// Move a dynamic vector into a dynamic vector
	v.Add({ 33, 44 });
	TInlineArray<SomeElem, 4> v6 = std::move(v);
	B3D_TEST_ASSERT(v.size() == 0);
	B3D_TEST_ASSERT(v6.size() == 5);
	B3D_TEST_ASSERT(v6[0].A == 10);
	B3D_TEST_ASSERT(v6[3].A == 3);
	B3D_TEST_ASSERT(v6[3].B == 4);
	B3D_TEST_ASSERT(v6[4].A == 33);
	B3D_TEST_ASSERT(v6[4].B == 44);
}

void UtilityTestSuite::TestArray()
{
	struct SomeElem
	{
		int A = 10;
		int B = 0;
	};

	// Make sure initial construction works
	TArray<SomeElem> v(4);
	B3D_TEST_ASSERT(v.Size() == 4);
	B3D_TEST_ASSERT(v.Capacity() == 4);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[3].A == 10);
	B3D_TEST_ASSERT(v[3].B == 0);

	// Add an element
	v.Add({ 3, 4 });
	B3D_TEST_ASSERT(v.Size() == 5);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[3].A == 10);
	B3D_TEST_ASSERT(v[3].B == 0);
	B3D_TEST_ASSERT(v[4].A == 3);
	B3D_TEST_ASSERT(v[4].B == 4);

	// Make a copy
	TArray<SomeElem> v2 = v;
	B3D_TEST_ASSERT(v2.Size() == 5);
	B3D_TEST_ASSERT(v2[0].A == 10);
	B3D_TEST_ASSERT(v2[3].A == 10);
	B3D_TEST_ASSERT(v2[3].B == 0);
	B3D_TEST_ASSERT(v2[4].A == 3);
	B3D_TEST_ASSERT(v2[4].B == 4);

	// Pop an element
	v2.Pop();
	B3D_TEST_ASSERT(v2.Size() == 4);
	B3D_TEST_ASSERT(v2[0].A == 10);
	B3D_TEST_ASSERT(v2[3].A == 10);
	B3D_TEST_ASSERT(v2[3].B == 0);

	// Remove an element
	v.Remove(2);
	B3D_TEST_ASSERT(v.Size() == 4);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[2].A == 10);
	B3D_TEST_ASSERT(v[3].A == 3);
	B3D_TEST_ASSERT(v[3].B == 4);

	// Insert an element
	v.Insert(v.begin() + 2, { 99, 100 });
	B3D_TEST_ASSERT(v.Size() == 5);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[2].A == 99);
	B3D_TEST_ASSERT(v[3].A == 10);
	B3D_TEST_ASSERT(v[4].A == 3);
	B3D_TEST_ASSERT(v[4].B == 4);

	// Insert a list
	v.Insert(v.begin() + 1, { { 55, 100 }, { 56, 100 }, { 57, 100 } });
	B3D_TEST_ASSERT(v.Size() == 8);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[1].A == 55);
	B3D_TEST_ASSERT(v[2].A == 56);
	B3D_TEST_ASSERT(v[3].A == 57);
	B3D_TEST_ASSERT(v[4].A == 10);
	B3D_TEST_ASSERT(v[5].A == 99);
	B3D_TEST_ASSERT(v[6].A == 10);
	B3D_TEST_ASSERT(v[7].A == 3);
	B3D_TEST_ASSERT(v[7].B == 4);

	// Erase a range of elements
	v.Erase(v.begin() + 2, v.begin() + 5);
	B3D_TEST_ASSERT(v.Size() == 5);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[1].A == 55);
	B3D_TEST_ASSERT(v[2].A == 99);
	B3D_TEST_ASSERT(v[3].A == 10);
	B3D_TEST_ASSERT(v[4].A == 3);
	B3D_TEST_ASSERT(v[4].B == 4);

	// Insert a range
	v.Insert(v.begin() + 1, v2.begin() + 1, v2.begin() + 3);
	B3D_TEST_ASSERT(v.Size() == 7);
	B3D_TEST_ASSERT(v[0].A == 10);
	B3D_TEST_ASSERT(v[1].A == 10);
	B3D_TEST_ASSERT(v[2].A == 10);
	B3D_TEST_ASSERT(v[3].A == 55);
	B3D_TEST_ASSERT(v[4].A == 99);
	B3D_TEST_ASSERT(v[5].A == 10);
	B3D_TEST_ASSERT(v[6].A == 3);
	B3D_TEST_ASSERT(v[6].B == 4);

	// Shrink capacity
	//v.Shrink();
	//B3D_TEST_ASSERT(v.Size() == v.Capacity());
	//B3D_TEST_ASSERT(v[0].A == 10);
	//B3D_TEST_ASSERT(v[1].A == 10);
	//B3D_TEST_ASSERT(v[2].A == 10);
	//B3D_TEST_ASSERT(v[3].A == 55);
	//B3D_TEST_ASSERT(v[4].A == 99);
	//B3D_TEST_ASSERT(v[5].A == 10);
	//B3D_TEST_ASSERT(v[6].A == 3);
	//B3D_TEST_ASSERT(v[6].B == 4);

	// Move it
	TArray<SomeElem> v3 = std::move(v2);
	B3D_TEST_ASSERT(v2.Size() == 0);
	B3D_TEST_ASSERT(v3.Size() == 4);
	B3D_TEST_ASSERT(v3[0].A == 10);
	B3D_TEST_ASSERT(v3[3].A == 10);
	B3D_TEST_ASSERT(v3[3].B == 0);
}

void UtilityTestSuite::TestComplex()
{
	// Use a tolerance for floating point comparisons to handle precision differences across build configurations
	constexpr float kTolerance = 0.01f;

	Complex<float> c(10.0, 4.0);
	B3D_TEST_ASSERT(c.Real() == 10.0);
	B3D_TEST_ASSERT(c.Imag() == 4.0);

	Complex<float> c2(15.0, 5.0);
	B3D_TEST_ASSERT(c2.Real() == 15.0);
	B3D_TEST_ASSERT(c2.Imag() == 5.0);

	Complex<float> c3 = c + c2;
	B3D_TEST_ASSERT(c3.Real() == 25.0);
	B3D_TEST_ASSERT(c3.Imag() == 9.0);

	Complex<float> c4 = c - c2;
	B3D_TEST_ASSERT(c4.Real() == -5.0);
	B3D_TEST_ASSERT(c4.Imag() == -1.0);

	Complex<float> c5 = c * c2;
	B3D_TEST_ASSERT(c5.Real() == 130.0);
	B3D_TEST_ASSERT(c5.Imag() == 110.0);

	Complex<float> c6 = c / c2;
	B3D_TEST_ASSERT(Math::ApproxEquals(c6.Real(), 0.68f, kTolerance));
	B3D_TEST_ASSERT(Math::ApproxEquals(c6.Imag(), 0.04f, kTolerance));

	B3D_TEST_ASSERT(Math::ApproxEquals(Complex<float>::Abs(c), 10.7703295f, kTolerance));
	B3D_TEST_ASSERT(Math::ApproxEquals(Complex<float>::Arg(c), 0.380506366f, kTolerance));
	B3D_TEST_ASSERT(Complex<float>::Norm(c) == 116);

	Complex<float> c7 = Complex<float>::Conj(c);
	B3D_TEST_ASSERT(c7.Real() == 10);
	B3D_TEST_ASSERT(c7.Imag() == -4);
	c7 = 0;

	c7 = Complex<float>::Polar(2.0, 0.5);
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Real(), 1.75516510f, kTolerance));
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Imag(), 0.958851099f, kTolerance));
	c7 = 0;

	c7 = Complex<float>::Cos(c);
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Real(), -22.9135609f, kTolerance));
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Imag(), 14.8462915f, kTolerance));
	c7 = 0;

	c7 = Complex<float>::Cosh(c);
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Real(), -7198.72949f, kTolerance));
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Imag(), -8334.84180f, kTolerance));
	c7 = 0;

	c7 = Complex<float>::Exp(c);
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Real(), -14397.4580f, kTolerance));
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Imag(), -16669.6836f, kTolerance));
	c7 = 0;

	c7 = Complex<float>::Log(c);
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Real(), 2.37679505f, kTolerance));
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Imag(), 0.380506366f, kTolerance));
	c7 = 0;

	c7 = Complex<float>::Pow(c, 2.0);
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Real(), 84.0000000f, kTolerance));
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Imag(), 80.0f, kTolerance));
	c7 = 0;

	c7 = Complex<float>::Sin(c);
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Real(), -14.8562555f, kTolerance));
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Imag(), -22.8981915f, kTolerance));
	c7 = 0;

	c7 = Complex<float>::Sinh(c);
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Real(), -7198.72900f, kTolerance));
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Imag(), -8334.84277f, kTolerance));
	c7 = 0;

	c7 = Complex<float>::Sqrt(c);
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Real(), 3.22260213f, kTolerance));
	B3D_TEST_ASSERT(Math::ApproxEquals(c7.Imag(), 0.620616496f, kTolerance));
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
	m.Resize(8);
	B3D_TEST_ASSERT(m.Valid() == true);

	SomeElem elements;
	elements.A = 4;
	elements.B = 5;

	m.Insert(elements, 10);
	B3D_TEST_ASSERT(m[0].Key.A == 4);
	B3D_TEST_ASSERT(m[0].Key.B == 5);
	B3D_TEST_ASSERT(m[0].Value == 10);
	B3D_TEST_ASSERT(m.Size() == 1);

	int v = 11;
	m.Insert(elements, v);
	B3D_TEST_ASSERT(m[1].Key.A == 4);
	B3D_TEST_ASSERT(m[1].Key.B == 5);
	B3D_TEST_ASSERT(m[1].Value == 11);
	B3D_TEST_ASSERT(m.Size() == 2);

	SomeElem minKey;
	int minValue;

	m.Minimum(minKey, minValue);
	B3D_TEST_ASSERT(minKey.A == 4);
	B3D_TEST_ASSERT(minKey.B == 5);
	B3D_TEST_ASSERT(minValue == 10);

	m.Erase(elements, v);
	B3D_TEST_ASSERT(m.Size() == 1);
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
			Size2 extents(
				types[i].SizeMin + ((rand() / (float)RAND_MAX)) * (types[i].SizeMax - types[i].SizeMin) * 0.5f,
				types[i].SizeMin + ((rand() / (float)RAND_MAX)) * (types[i].SizeMax - types[i].SizeMin) * 0.5f);

			Vector2 position(
				((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents - extents.Width,
				((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents - extents.Height);

			DebugQuadtreeElement elem;
			elem.Box = Area2(position, extents * 2.0f);

			u32 elemIdx = (u32)quadtreeData.Elements.size();
			quadtreeData.Elements.push_back(elem);
			quadtree.AddElement(elemIdx);
		}
	}

	DebugQuadtreeElement manualElems[3];
	manualElems[0].Box = Area2(Vector2(100.0f, 100.0f), Size2(110.0f, 115.0f));
	manualElems[1].Box = Area2(Vector2(200.0f, 100.0f), Size2(250.0f, 150.0f));
	manualElems[2].Box = Area2(Vector2(90.0f, 90.0f), Size2(105.0f, 105.0f));

	for(u32 i = 0; i < 3; i++)
	{
		u32 elemIdx = (u32)quadtreeData.Elements.size();
		quadtreeData.Elements.push_back(manualElems[i]);
		quadtree.AddElement(elemIdx);
	}

	Area2 queryBounds = manualElems[0].Box;
	DebugQuadtree::AreaIntersectIterator interIter(quadtree, queryBounds);

	Vector<u32> overlapElements;
	while(interIter.MoveNext())
	{
		u32 element = interIter.GetElement();
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
		quadtree.RemoveElement(entry.QuadtreeId);
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

	bs.Read(uv);
	B3D_TEST_ASSERT(uv == v2);

	bs.Read(bv);
	B3D_TEST_ASSERT(bv == v3);

	bs.Read(bv);
	B3D_TEST_ASSERT(bv == v4);

	uv = 0;
	bs.ReadBits((uint8_t*)&uv, 10);
	B3D_TEST_ASSERT(uv == v5);

	bs.Read(sv);
	B3D_TEST_ASSERT(sv == v6);

	bs.ReadVarInt(iv);
	B3D_TEST_ASSERT(iv == v7);

	bs.ReadVarIntDelta(iv, 0);
	B3D_TEST_ASSERT(iv == v7);

	bs.ReadVarInt(ulv);
	B3D_TEST_ASSERT(ulv == v8);

	bs.ReadVarIntDelta(v8, v8);
	B3D_TEST_ASSERT(ulv == v8);

	bs.ReadNorm(fv);
	B3D_TEST_ASSERT(Math::ApproxEquals(fv, v9, 0.01f));

	bs.ReadRange(fv, 5.0f, 15.0f);
	B3D_TEST_ASSERT(Math::ApproxEquals(fv, v10, 0.01f));

	bs.ReadRange(uv, 500U, 1000U);
	B3D_TEST_ASSERT(uv == v5);

	bs.Align();
	bs.Read(ulv);
	B3D_TEST_ASSERT(ulv == v11);
}

void UtilityTestSuite::TestRTTIIterator()
{
	Vector<int> values = { 5, 10, 33, 24, 16 };

	TRTTIIterator<Vector<int>, true> rttiVectorIterator(values);
	B3D_TEST_ASSERT(rttiVectorIterator.SupportsSeekToIndex());
	B3D_TEST_ASSERT(!rttiVectorIterator.SupportsSeekToKey());

	auto vectorIterator = values.begin();
	for(; rttiVectorIterator.IsValid(); ++rttiVectorIterator, ++vectorIterator)
	{
		B3D_TEST_ASSERT(*rttiVectorIterator == *vectorIterator)
	}

	rttiVectorIterator.SeekToEnd();
	rttiVectorIterator = 100;
	rttiVectorIterator.SeekToEnd();
	rttiVectorIterator = 200;
	rttiVectorIterator.SeekToEnd();
	rttiVectorIterator = 500;
	B3D_TEST_ASSERT(rttiVectorIterator.GetElementCount() == 8)

	rttiVectorIterator.SeekToBeginning();
	vectorIterator = values.begin();
	for(; rttiVectorIterator.IsValid(); ++rttiVectorIterator, ++vectorIterator)
	{
		B3D_TEST_ASSERT(*rttiVectorIterator == *vectorIterator)
	}

	rttiVectorIterator.SeekToIndex(3);
	B3D_TEST_ASSERT(*rttiVectorIterator == values[3])

	rttiVectorIterator.SeekToIndex(5);
	B3D_TEST_ASSERT(*rttiVectorIterator == 100)

	rttiVectorIterator = 1000;
	B3D_TEST_ASSERT(*rttiVectorIterator == 1000)
	B3D_TEST_ASSERT(rttiVectorIterator.GetElementCount() == 8)

	rttiVectorIterator.SeekToIndex(50);
	B3D_TEST_ASSERT(!rttiVectorIterator.IsValid())

	rttiVectorIterator.SeekToEnd();
	rttiVectorIterator = 5000;
	B3D_TEST_ASSERT(*rttiVectorIterator == 5000)
	B3D_TEST_ASSERT(rttiVectorIterator.GetElementCount() == 9)

	Map<int, int> mapValues;
	mapValues[5] = 500;
	mapValues[10] = 1000;
	mapValues[33] = 3300;
	mapValues[24] = 2400;
	mapValues[16] = 1600;

	TRTTIIterator<Map<int, int>, true> rttiMapIterator(mapValues);
	B3D_TEST_ASSERT(rttiMapIterator.SupportsSeekToKey())

	auto mapIterator = mapValues.begin();
	for(; rttiMapIterator.IsValid(); ++rttiMapIterator, ++mapIterator)
	{
		B3D_TEST_ASSERT(*rttiMapIterator == *mapIterator)
	}

	rttiMapIterator.SeekToEnd();
	rttiMapIterator = std::make_pair(100, 10000);
	rttiMapIterator.SeekToEnd();
	rttiMapIterator = std::make_pair(200, 20000);
	rttiMapIterator.SeekToEnd();
	rttiMapIterator = std::make_pair(500, 50000);

	B3D_TEST_ASSERT(rttiMapIterator.GetElementCount() == 8)

	rttiMapIterator.SeekToBeginning();
	mapIterator = mapValues.begin();
	for(; rttiMapIterator.IsValid(); ++rttiMapIterator, ++mapIterator)
	{
		B3D_TEST_ASSERT(*rttiMapIterator == *mapIterator)
	}

	rttiMapIterator.SeekToBeginning();
	const void* pair5 = rttiMapIterator.GetValue();
	rttiMapIterator.SeekToKey(pair5);
	B3D_TEST_ASSERT((*rttiMapIterator).first == 5)
	B3D_TEST_ASSERT((*rttiMapIterator).second == mapValues[5])

	const auto pair33 = std::make_pair(33, 3300);
	rttiMapIterator.SeekToKey(&pair33);
	B3D_TEST_ASSERT((*rttiMapIterator).first == pair33.first)
	B3D_TEST_ASSERT((*rttiMapIterator).second == mapValues[33])

	const auto pair100 = std::make_pair(100, 10000);
	rttiMapIterator.SeekToKey(&pair100);
	B3D_TEST_ASSERT((*rttiMapIterator).first == pair100.first)
	B3D_TEST_ASSERT((*rttiMapIterator).second == mapValues[100])

	rttiMapIterator = std::make_pair(800, 33333);
	B3D_TEST_ASSERT((*rttiMapIterator).first == 800)
	B3D_TEST_ASSERT((*rttiMapIterator).second == 33333)
	B3D_TEST_ASSERT((*rttiMapIterator).second == mapValues[800])
	B3D_TEST_ASSERT(mapValues.find(100) == mapValues.end())
	B3D_TEST_ASSERT(rttiMapIterator.GetElementCount() == 8)

	const auto pair70 = std::make_pair(70, 0);
	rttiMapIterator.SeekToKey(&pair70);
	B3D_TEST_ASSERT(!rttiMapIterator.IsValid())

	rttiMapIterator.SeekToEnd();
	rttiMapIterator = std::make_pair(1200, 66666);
	B3D_TEST_ASSERT((*rttiMapIterator).first == 1200)
	B3D_TEST_ASSERT((*rttiMapIterator).second == 66666)
	B3D_TEST_ASSERT((*rttiMapIterator).second == mapValues[1200])
	B3D_TEST_ASSERT(rttiMapIterator.GetElementCount() == 9)
}

void UtilityTestSuite::TestMPSCQueue()
{
	static constexpr u32 kProducerThreadCount = 12;
	static constexpr u32 kEntriesPerThread = 1000;

	TQueue<u32, QueueThreadingPolicy::MPSC> queue;

	std::atomic<u32> finishedProducerThreads{0};

	TArray<u32> readValues;
	Thread* consumerThread = B3DNew<Thread>([&queue, &readValues, &finishedProducerThreads]
	{
		while(true)
		{
			TOptional<u32> maybeValue = queue.Dequeue();
			if(maybeValue.has_value())
			{
				readValues.Add(maybeValue.value());
				continue;
			}

			// All producers finished: drain any items enqueued between the last
			// failed Dequeue and observing the counter, then exit.
			if(finishedProducerThreads >= kProducerThreadCount)
			{
				while((maybeValue = queue.Dequeue()).has_value())
					readValues.Add(maybeValue.value());
				break;
			}
		}
	});

	Thread* producerThreads[kProducerThreadCount] { nullptr };
	for(u32 threadIndex = 0; threadIndex < kProducerThreadCount; threadIndex++)
	{
		const u32 startIndex = threadIndex * kEntriesPerThread;
		producerThreads[threadIndex] = B3DNew<Thread>([&queue, startIndex, &finishedProducerThreads]
		{
			for(u32 i = 0; i < kEntriesPerThread; i++)
				queue.Enqueue(startIndex + i);

			++finishedProducerThreads;
		});
	}

	consumerThread->WaitUntilComplete();

	std::sort(readValues.begin(), readValues.end());

	for(u32 i = 0; i < (kProducerThreadCount * kEntriesPerThread); ++i)
		B3D_TEST_ASSERT(i == readValues[i])

	for(u32 threadIndex = 0; threadIndex < kProducerThreadCount; threadIndex++)
	{
		B3DDelete(producerThreads[threadIndex]);
		producerThreads[threadIndex] = nullptr;
	}

	B3DDelete(consumerThread);
}

void UtilityTestSuite::TestSPSCQueue()
{
	TQueue<u32, QueueThreadingPolicy::SPSC> queue;

	std::atomic<bool> producerThreadFinished{false};

	TArray<u32> readValues;
	Thread* consumerThread = B3DNew<Thread>([&queue, &readValues, &producerThreadFinished]
	{
		while(true)
		{
			TOptional<u32> maybeValue = queue.Dequeue();
			if(maybeValue.has_value())
			{
				readValues.Add(maybeValue.value());
				continue;
			}

			// Producer finished: drain any items enqueued between the last
			// failed Dequeue and observing the flag, then exit.
			if(producerThreadFinished)
			{
				while((maybeValue = queue.Dequeue()).has_value())
					readValues.Add(maybeValue.value());
				break;
			}
		}
	});

	static constexpr u32 kEntryCount = 10000;
	Thread* producerThread = B3DNew<Thread>([&queue, &producerThreadFinished]
	{
		for(u32 i = 0; i < kEntryCount; i++)
			queue.Enqueue(i);

		producerThreadFinished = true;
	});

	consumerThread->WaitUntilComplete();

	for(u32 i = 0; i < kEntryCount; ++i)
		B3D_TEST_ASSERT(i == readValues[i])

	B3DDelete(producerThread);
	B3DDelete(consumerThread);
}

void UtilityTestSuite::TestHashedString()
{
	HashedString a = "Test string";
	HashedString b = "Test string";
	HashedString c = "Different test string";

	B3D_TEST_ASSERT(a == b)
	B3D_TEST_ASSERT(a != c)
	B3D_TEST_ASSERT(strcmp(a.GetCharacters(), "Test string") == 0)
	B3D_TEST_ASSERT(strlen(a.GetCharacters()) == 11)
	B3D_TEST_ASSERT(a.GetLength() == 12)


}
