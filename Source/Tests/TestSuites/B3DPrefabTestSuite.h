//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Testing/B3DTestSuite.h"
#include "B3DUnitTestScenes.h"
#include "Resources/B3DResourceHandle.h"
#include "Scene/B3DSceneInstance.h"

namespace b3d
{
	class Prefab;

	class PrefabTestSuite : public TestSuite
	{
	public:
		PrefabTestSuite();

	private:
		void TestPrefabSaveLoad();
		void TestPrefabScenario1();
		void TestPrefabScenario2();
		void TestPrefabScenario3();
		void TestPrefabScenario4();
		void TestPrefabScenario5();
		void TestPrefabScenario6();
		void TestPrefabScenario7();
		void TestPrefabScenario8();
		void TestPrefabScenario9();
		void TestPrefabScenario10();

		void TestAssertPrefabScenario();

		SPtr<SceneInstance> mSceneHierarchy;
		UnitTestSceneB mScene;

		struct PrefabTestInformation
		{
			HPrefab Prefab;
			SPtr<UnitTestSceneB> PrefabInternalsScene;
		};

		Array<PrefabTestInformation, 4> mPrefabTestInformation;
	};
} // namespace b3d
