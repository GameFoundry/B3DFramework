//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Tests D3D12 descriptor-table and dynamic root-CBV bindings. */
	class D3D12ParameterTestSuite : public TestSuite
	{
	public:
		D3D12ParameterTestSuite();

	private:
		/** Checks that static buffers do not consume dynamic root-CBV positions. */
		void TestUniformBufferLayout();
	};
}
