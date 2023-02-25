//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2023 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************
#pragma once
#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
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

	/** @} */
} // namespace bs
