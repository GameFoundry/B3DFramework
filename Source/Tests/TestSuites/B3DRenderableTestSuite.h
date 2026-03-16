//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	class RenderableTestSuite : public TestSuite
	{
	public:
		RenderableTestSuite();

	private:
		void TestRenderableSerialization();
	};
} // namespace b3d
