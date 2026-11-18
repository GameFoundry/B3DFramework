//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Tests for the evaluation of animations by AnimationScene. */
	class AnimationTestSuite : public TestSuite
	{
	public:
		AnimationTestSuite();

	private:
		/**
		 * Evaluates the morph shape weights of a mesh with two channels of two shapes each, at frame weights 0.5 and 1.0. The channels have
		 * different weights and frame curves that run in opposite directions. At each channel position the two shapes of a channel must
		 * weigh what blends from the base shape to the first shape and on to the second, times the channel's weight.
		 */
		void TestMorphShapeWeights();
	};
} // namespace b3d
