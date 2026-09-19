//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	class CoreTestSuite : public TestSuite
	{
	public:
		CoreTestSuite();

	private:
		void TestAnimCurveIntegration();
		void TestLookupTable();
		void TestBinarySerialization();
		void TestDataBlockSerialization();
		void TestSerializedObject();
		void TestReadOnlySerialization();
		void TestRTTIObjectWrapperComparison();
		void TestRTTIObjectWrapperFieldFilter();
		void TestBinaryDelta();
		void TestPersistentCacheLockedEntry();
		/** Verifies outdated cache packages are discarded and replaced at the same path. */
		void TestPersistentCacheVersion();
		/** Verifies merging defaults, aliases and attributes without mutating conflicting descriptions. */
		void TestShaderParameterDescription();
		/** Verifies shared shader descriptions and serialization of shader reflection. */
		void TestShaderReflection();
		/** Verifies stable shader cache paths and cooked-store version metadata. */
		void TestShaderRegistryCacheVersion();
#if !B3D_PLATFORM_PS5
		/** Verifies adapter creation requires explicit variation compilation. */
		void TestMaterialParameterAdapterCompilation();
		/** Verifies BSL parameter merging and source metadata against compiled GPU programs. */
		void TestShaderReflectionCompilation();
#endif
#if B3D_PLATFORM_MACOS
		void TestMacOSDesktopInput();
#endif
	};
} // namespace b3d
