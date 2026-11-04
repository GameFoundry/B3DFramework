//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	class GUITextLayoutTestSuite : public TestSuite
	{
	public:
		GUITextLayoutTestSuite();

	private:
		void TestPositionsFollowTextOrderAcrossFontPages();
		void TestSelectionAreas();
		void TestStepsAcrossNewlines();
		void TestWordWrap();
		void TestAlignment();
		void TestFractionalLineHeightLeavesNoGaps();
		void TestPointsOutsideText();
		void TestEmptyText();
	};
} // namespace b3d
