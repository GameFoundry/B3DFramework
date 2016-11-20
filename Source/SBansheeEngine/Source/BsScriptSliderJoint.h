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

	struct ScriptSliderJointData;

	/** Interop class between C++ & CLR for SliderJoint. */
	class BS_SCR_BE_EXPORT ScriptSliderJoint : public TScriptJoint<ScriptSliderJoint>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeSliderJoint")
	private:
		ScriptSliderJoint(MonoObject* instance, const SPtr<Joint>& joint);

		/** Returns the native slider joint wrapped by this object. */
		SliderJoint* getSliderJoint() const;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, ScriptCommonJointData* commonData, ScriptSliderJointData* data);
		static float internal_GetPosition(ScriptSliderJoint* thisPtr);
		static float internal_GetSpeed(ScriptSliderJoint* thisPtr);
		static void internal_SetLimit(ScriptSliderJoint* thisPtr, MonoObject* limit);
		static void internal_SetEnableLimit(ScriptSliderJoint* thisPtr, bool enable);
	};

	/** Used for passing common Joint initialization data between native and managed code. */
	struct ScriptSliderJointData // Note: Must match C# struct ScriptSliderJointData
	{
		MonoObject* limit;
		bool enableLimit;

		/** Converts this structure into a descriptor used for initializing a joint. */
		void toDesc(SLIDER_JOINT_DESC& desc) const;
	};

	/** @} */
}