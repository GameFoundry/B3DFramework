//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	class GUIPointerEventTrackerTestSuite : public TestSuite
	{
	public:
		GUIPointerEventTrackerTestSuite();

	private:
		void TestContainersAreEnteredThroughChildren();
		void TestMovingBetweenSiblingsKeepsContainerEntered();
		void TestPressReachesContainers();
		void TestLeavingEndsPress();
		void TestPressCapturesEnteredElements();
		void TestMouseOverStopsAtHandler();
		void TestMouseOutReachesEveryDepartedElement();
		void TestMouseDownStopsAtHandler();
		void TestMouseUpRequiresPress();
		void TestDrag();
		void TestCursor();
		void TestDragAndDrop();
		void TestPointerLeavingWindow();
		void TestDestroyedElementsAreForgotten();
	};
} // namespace b3d
