//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsJoint.h"
#include "BsD6Joint.h"
#include "BsHingeJoint.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Helper class for dealing with D6Joint::Drive structure. */
	class BS_SCR_BE_EXPORT ScriptD6JointDrive : public ScriptObject<ScriptD6JointDrive>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "D6JointDrive")

		/** Converts managed limit to its native counterpart. */
		static D6Joint::Drive convert(MonoObject* object);

	private:
		ScriptD6JointDrive(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef MonoObject*(__stdcall *GetNativeDataThunkDef) (MonoObject*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};

	/** Helper class for dealing with HingeJoint::Drive structure. */
	class BS_SCR_BE_EXPORT ScriptHingeJointDrive : public ScriptObject<ScriptHingeJointDrive>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "HingeJointDrive")

		/** Converts managed limit to its native counterpart. */
		static HingeJoint::Drive convert(MonoObject* object);

	private:
		ScriptHingeJointDrive(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef MonoObject*(__stdcall *GetNativeDataThunkDef) (MonoObject*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};

	/** Helper class for dealing with LimitLinearRange structure. */
	class BS_SCR_BE_EXPORT ScriptLimitLinearRange : public ScriptObject<ScriptLimitLinearRange>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "LimitLinearRange")

		/** Converts managed limit to its native counterpart. */
		static LimitLinearRange convert(MonoObject* object);

	private:
		ScriptLimitLinearRange(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef MonoObject*(__stdcall *GetNativeDataThunkDef) (MonoObject*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};

	/** Helper class for dealing with LimitLinear structure. */
	class BS_SCR_BE_EXPORT ScriptLimitLinear : public ScriptObject<ScriptLimitLinear>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "LimitLinear")

		/** Converts managed limit to its native counterpart. */
		static LimitLinear convert(MonoObject* object);

	private:
		ScriptLimitLinear(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef MonoObject*(__stdcall *GetNativeDataThunkDef) (MonoObject*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};

	/** Helper class for dealing with LimitAngularRange structure. */
	class BS_SCR_BE_EXPORT ScriptLimitAngularRange : public ScriptObject<ScriptLimitAngularRange>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "LimitAngularRange")

		/** Converts managed limit to its native counterpart. */
		static LimitAngularRange convert(MonoObject* object);

	private:
		ScriptLimitAngularRange(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef MonoObject*(__stdcall *GetNativeDataThunkDef) (MonoObject*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};

	/** Helper class for dealing with LimitConeRange structure. */
	class BS_SCR_BE_EXPORT ScriptLimitConeRange : public ScriptObject<ScriptLimitConeRange>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "LimitConeRange")

		/** Converts managed limit to its native counterpart. */
		static LimitConeRange convert(MonoObject* object);

	private:
		ScriptLimitConeRange(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef MonoObject*(__stdcall *GetNativeDataThunkDef) (MonoObject*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};

	/** @} */
}