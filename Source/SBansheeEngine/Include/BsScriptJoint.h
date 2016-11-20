//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsJoint.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	class ScriptJoint;

	/** Base class for all Joint interop objects. */
	class BS_SCR_BE_EXPORT ScriptJointBase : public ScriptObjectBase
	{
	public:
		/** Returns the native Joint object. */
		virtual Joint* getJoint() const { return mJoint.get(); };
	protected:
		friend ScriptJoint;

		ScriptJointBase(MonoObject* instance);
		virtual ~ScriptJointBase() {}

		/** Initializes the interop object with a native joint. Must be called right after construction. */
		void initialize(const SPtr<Joint>& joint);

		/** Destroys the internal joint object. */
		void destroyJoint();

		SPtr<Joint> mJoint;
	};

	/** A more specialized version of ScriptObject that allows the constructor to set the native joint. */
	template <class Type>
	class TScriptJoint : public ScriptObject<Type, ScriptJointBase>
	{
	public:
		virtual ~TScriptJoint() {}

	protected:
		TScriptJoint(MonoObject* instance, const SPtr<Joint>& joint)
			:ScriptObject<Type, ScriptJointBase>(instance)
		{
			this->initialize(joint);
		}
	};

	/** Interop class between C++ & CLR for Joint. */
	class BS_SCR_BE_EXPORT ScriptJoint : public TScriptJoint<ScriptJoint>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeJoint")

	private:
		friend class ScriptJointBase;

		ScriptJoint(MonoObject* instance);

		/** Triggered the joint breaks. */
		static void onJointBreak(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_Destroy(ScriptJointBase* thisPtr);
		static void internal_SetBody(ScriptJointBase* thisPtr, JointBody body, ScriptRigidbody* value);
		static void internal_SetPosition(ScriptJointBase* thisPtr, JointBody body, Vector3* position);
		static void internal_SetRotation(ScriptJointBase* thisPtr, JointBody body, Quaternion* rotation);
		static void internal_SetBreakForce(ScriptJointBase* thisPtr, float force);
		static void internal_SetBreakTorque(ScriptJointBase* thisPtr, float torque);
		static void internal_SetEnableCollision(ScriptJointBase* thisPtr, bool value);

		typedef void(__stdcall *OnJointBreakThunkDef) (MonoObject*, MonoException**);

		static OnJointBreakThunkDef onJointBreakThunk;
	};

	/** Used for passing common Joint initialization data between native and managed code. */
	struct ScriptCommonJointData // Note: Must match C# struct ScriptCommonJointData
	{
		MonoArray* bodies;
		MonoArray* positions;
		MonoArray* rotations;
		float breakForce;
		float breakTorque;
		bool enableCollision;

		/** Converts this structure into a descriptor used for initializing a joint. */
		void toDesc(JOINT_DESC& desc) const;
	};

	/** @} */
}