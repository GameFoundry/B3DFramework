//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptCollider.h"
#include "BsScriptJoint.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for FixedJoint. */
	class BS_SCR_BE_EXPORT ScriptFixedJoint : public TScriptJoint<ScriptFixedJoint>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeFixedJoint")
	private:
		ScriptFixedJoint(MonoObject* instance, const SPtr<Joint>& joint);

		/** Returns the native fixed joint wrapped by this object. */
		FixedJoint* getFixedJoint() const;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, ScriptCommonJointData* commonData);
	};

	/** @} */
}