//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Threading/BsAsyncOp.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** 
	 * Object you may use to check on the results of an asynchronous operation. Contains uninitialized data until 
	 * IsComplete returns true.
	 */
	class BS_SCR_BE_EXPORT BS_SCRIPT_EXPORT(n:AsyncOp) AsyncOpEx
	{
	public:
		AsyncOpEx(const AsyncOp& op, const std::function<MonoObject*(const AsyncOp&)>& convertCallback);

		/** @copydoc AsyncOp::hasCompleted */
		BS_SCRIPT_EXPORT(n:IsComplete,pr:getter)
		bool isComplete() const;

		/** Retrieves the value returned by the async operation. Only valid if IsComplete returns true. */
		BS_SCRIPT_EXPORT(n:ReturnValue,pr:getter)
		MonoObject* getReturnValue() const;

		/** @copydoc AsyncOp::blockUntilComplete */
		BS_SCRIPT_EXPORT(n:BlockUntilComplete)
		void blockUntilComplete() const;

	private:
		AsyncOp mAsyncOp;
		std::function<MonoObject*(const AsyncOp&)> mConvertCallback;
	};

	/** @endcond */
	/** @} */
}