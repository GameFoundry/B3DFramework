//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptCollider.h"
#include "BsScriptJoint.h"
#include "BsHingeJoint.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	struct ScriptHingeJointData;

	/** Interop class between C++ & CLR for HingeJoint. */
	class BS_SCR_BE_EXPORT ScriptHingeJoint : public TScriptJoint<ScriptHingeJoint>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeHingeJoint")
	private:
		ScriptHingeJoint(MonoObject* instance, const SPtr<Joint>& joint);

		/** Returns the native hinge joint wrapped by this object. */
		HingeJoint* getHingeJoint() const;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, ScriptCommonJointData* commonData, ScriptHingeJointData* data);
		static float internal_GetAngle(ScriptHingeJoint* thisPtr);
		static float internal_GetSpeed(ScriptHingeJoint* thisPtr);
		static void internal_SetLimit(ScriptHingeJoint* thisPtr, MonoObject* limit);
		static void internal_SetDrive(ScriptHingeJoint* thisPtr, MonoObject* drive);
		static void internal_SetEnableLimit(ScriptHingeJoint* thisPtr, bool value);
		static void internal_SetEnableDrive(ScriptHingeJoint* thisPtr, bool value);
	};

	/** Used for passing common Joint initialization data between native and managed code. */
	struct ScriptHingeJointData // Note: Must match C# struct ScriptHingeJointData
	{
		MonoObject* limit;
		MonoObject* drive;
		HingeJoint::Flag flags;

		/** Converts this structure into a descriptor used for initializing a joint. */
		void toDesc(HINGE_JOINT_DESC& desc) const;
	};

	/** @} */
}