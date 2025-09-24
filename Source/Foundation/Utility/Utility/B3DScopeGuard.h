//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2023 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************
#pragma once
#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/** Executes a function when the object goes out of scope. */
	struct ScopeGuard : INonCopyable
	{
		ScopeGuard(const Function<void()>& callback) :
			Callback(callback)
		{ }

		ScopeGuard(Function<void()>&& callback) :
			Callback(std::move(callback))
		{ }

		~ScopeGuard() { Callback(); }

	private:
		Function<void()> Callback;
	};


	/** Ensures a piece of code executes when the current scope ends. */
#define B3D_SCOPE_CLEANUP(x) auto B3D_CONCAT(scopeGuard_, __LINE__) = ScopeGuard([&] { x; })

	/** @} */
} // namespace b3d
