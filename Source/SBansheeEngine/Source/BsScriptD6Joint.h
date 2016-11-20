//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptCollider.h"
#include "BsScriptJoint.h"
#include "BsD6Joint.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	struct ScriptD6JointData;

	/** Interop class between C++ & CLR for D6Joint. */
	class BS_SCR_BE_EXPORT ScriptD6Joint : public TScriptJoint<ScriptD6Joint>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeD6Joint")
	private:
		ScriptD6Joint(MonoObject* instance, const SPtr<Joint>& joint);

		/** Returns the native D6 joint wrapped by this object. */
		D6Joint* getD6Joint() const;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, ScriptCommonJointData* commonData, ScriptD6JointData* data);
		static void internal_SetMotion(ScriptD6Joint* thisPtr, D6Joint::Axis axis, D6Joint::Motion motion);
		static float internal_GetTwist(ScriptD6Joint* thisPtr);
		static float internal_GetSwingY(ScriptD6Joint* thisPtr);
		static float internal_GetSwingZ(ScriptD6Joint* thisPtr);
		static void internal_SetLimitLinear(ScriptD6Joint* thisPtr, MonoObject* limit);
		static void internal_SetLimitTwist(ScriptD6Joint* thisPtr, MonoObject* limit);
		static void internal_SetLimitSwing(ScriptD6Joint* thisPtr, MonoObject* limit);
		static void internal_SetDrive(ScriptD6Joint* thisPtr, D6Joint::DriveType type, MonoObject* drive);
		static void internal_SetDrivePosition(ScriptD6Joint* thisPtr, Vector3* position);
		static void internal_SetDriveRotation(ScriptD6Joint* thisPtr, Quaternion* rotation);
		static void internal_SetDriveLinearVelocity(ScriptD6Joint* thisPtr, Vector3* velocity);
		static void internal_SetDriveAngularVelocity(ScriptD6Joint* thisPtr, Vector3* velocity);
	};

	/** Used for passing common Joint initialization data between native and managed code. */
	struct ScriptD6JointData // Note: Must match C# struct ScriptD6JointData
	{
		MonoObject* linearLimit;
		MonoObject* twistLimit;
		MonoObject* swingLimit;
		MonoArray* motion;
		MonoArray* drives;
		Vector3 drivePosition;
		Quaternion driveRotation;
		Vector3 driveLinearVelocity;
		Vector3 driveAngularVelocity;

		/** Converts this structure into a descriptor used for initializing a joint. */
		void toDesc(D6_JOINT_DESC& desc) const;
	};

	/** @} */
}