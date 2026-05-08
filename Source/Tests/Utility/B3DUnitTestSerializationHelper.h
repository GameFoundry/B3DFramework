//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "B3DPrerequisites.h"
#include "B3DUnitTestSerializableObjects.h"
#include "Testing/B3DTestSuite.h"

namespace b3d
{
	struct UnitTestSerializationHelpers
	{
		/** Asserts that provided array sizes match, and all element's match (based on their equality operators). */
		template <typename T>
		static void TestAssertArraysMatch(TestSuite& testSuite, const T& lhs, const T& rhs);

		/** Asserts that provided array sizes match, and all element's match (based on their equality operators). Elements are assumed to be pointers and will be dereferenced before comparison. */
		template <typename T>
		static void TestAssertArrayContentsMatch(TestSuite& testSuite, const T& lhs, const T& rhs);

		/** Asserts that provided maps contain an exact match set of keys (and no other), and all element's match (based on their equality operators). */
		template <typename T>
		static void TestAssertMapsMatch(TestSuite& testSuite, const T& lhs, const T& rhs);

		/** Asserts that provided maps contain an exact match set of keys (and no other), and all element's match (based on their equality operators). Values are assumed to be pointers and will be dereferenced before comparison. */
		static void TestAssertObjectsMatch(TestSuite& testSuite, const TShared<UnitTestSerializationObjectA>& lhs, const TShared<UnitTestSerializationObjectA>& rhs, bool isDelta);
	};
} // namespace b3d
