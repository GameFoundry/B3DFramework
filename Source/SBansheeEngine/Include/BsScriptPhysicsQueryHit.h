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

	/** Interop struct between C++ & CLR for PhysicsQueryHit. */
	struct ScriptPhysicsQueryHit // Must match C# ScriptPhysicsQueryHit struct layout
	{
		Vector3 point;
		Vector3 normal;
		Vector2 uv;
		float distance;
		UINT32 triangleIdx;
		MonoObject* collider;
	};

	/** Helper class for dealing with PhysicsQueryHit structure. */
	class BS_SCR_BE_EXPORT ScriptPhysicsQueryHitHelper : public ScriptObject<ScriptPhysicsQueryHitHelper>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "ScriptPhysicsQueryHit")

		/** Converts native physics query hit to its managed counterpart. */
		static ScriptPhysicsQueryHit create(const PhysicsQueryHit& data);

		/** Unboxes a boxed managed ScriptPhysicsQueryHit struct and returns the native version of the structure. */
		static ScriptPhysicsQueryHit unbox(MonoObject* obj);

		/** Boxes a native ScriptPhysicsQueryHit struct and returns a managed object containing it. */
		static MonoObject* box(const ScriptPhysicsQueryHit& value);

	private:
		ScriptPhysicsQueryHitHelper(MonoObject* instance);
	};

	/** @} */
}