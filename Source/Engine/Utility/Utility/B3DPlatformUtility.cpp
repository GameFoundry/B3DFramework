//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DPlatformUtility.h"

#include <atomic>

// Functionality shared by every platform. The rest of PlatformUtility is implemented per platform.
namespace b3d
{
	namespace
	{
		std::atomic<bool> gAreEnsureBreaksDisabled(false);
	} // namespace

	void PlatformUtility::DisableEnsureBreaks()
	{
		gAreEnsureBreaksDisabled.store(true, std::memory_order_relaxed);
	}

	bool PlatformUtility::AreEnsureBreaksDisabled()
	{
		return gAreEnsureBreaksDisabled.load(std::memory_order_relaxed);
	}
} // namespace b3d
