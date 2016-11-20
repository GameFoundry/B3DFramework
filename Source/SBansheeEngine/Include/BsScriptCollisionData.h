//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsPhysicsCommon.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop struct between C++ & CLR for CollisionData. */
	struct ScriptCollisionData // Must match C# ScriptCollisionData struct layout
	{
		MonoObject* colliderA;
		MonoObject* colliderB;
		MonoArray* contactPoints;
	};

	/** Helper class for dealing with CollisionData structure. */
	class BS_SCR_BE_EXPORT ScriptCollisionDataHelper : public ScriptObject<ScriptCollisionDataHelper>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "ScriptCollisionData")

		/** Converts native collision data to its managed counterpart. */
		static ScriptCollisionData create(const CollisionData& data);

		/** Unboxes a boxed managed ScriptCollisionData struct and returns the native version of the structure. */
		static ScriptCollisionData unbox(MonoObject* obj);

		/** Boxes a native ScriptCollisionData struct and returns a managed object containing it. */
		static MonoObject* box(const ScriptCollisionData& value);

	private:
		ScriptCollisionDataHelper(MonoObject* instance);
	};

	/** Helper class for dealing with ContactPoint structure. */
	class BS_SCR_BE_EXPORT ScriptContactPointHelper : public ScriptObject<ScriptContactPointHelper>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "ContactPoint")

		/** Unboxes a boxed managed ContactPoint struct and returns the native version of the structure. */
		static ContactPoint unbox(MonoObject* obj);

		/** Boxes a native ContactPoint struct and returns a managed object containing it. */
		static MonoObject* box(const ContactPoint& value);

	private:
		ScriptContactPointHelper(MonoObject* instance);
	};

	/** @} */
}