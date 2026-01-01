//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Keeps track of all registered test suites. */
	class B3D_EXPORT TestSuiteRegistry : public Module<TestSuiteRegistry>
	{
	public:
		/** Register a test suite. */
		void RegisterSuite(const SPtr<TestSuite>& suite);

		/** Get all registered suites. */
		const Vector<SPtr<TestSuite>>& GetSuites() const { return mSuites; }

		/** Clear all registered suites. */
		void Clear() { mSuites.clear(); }

	private:
		Vector<SPtr<TestSuite>> mSuites;
	};

	/** @} */
} // namespace b3d
