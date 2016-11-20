//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsDegree.h"
#include "BsColor.h"
#include "BsLight.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Light. */
	class BS_SCR_BE_EXPORT ScriptLight : public ScriptObject <ScriptLight>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeLight")

		/**	Gets the wrapped native LightInternal object. */
		SPtr<Light> getInternal() const { return mLight; }

	private:
		ScriptLight(MonoObject* managedInstance, const HSceneObject& parentSO);
		~ScriptLight();

		/**	Destroys the internal light handler object. */
		void destroy();

		/** @copydoc ScriptObject::_onManagedInstanceDeleted */
		void _onManagedInstanceDeleted() override;

		SPtr<Light> mLight;
		UINT32 mLastUpdateHash;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_create(MonoObject* managedInstance, ScriptSceneObject* parentSO);

		static LightType internal_getType(ScriptLight* thisPtr);
		static void internal_setType(ScriptLight* thisPtr, LightType type);

		static bool internal_getCastsShadow(ScriptLight* thisPtr);
		static void internal_setCastsShadow(ScriptLight* thisPtr, bool castsShadow);

		static bool internal_getPhysicallyBasedAttenuation(ScriptLight* thisPtr);
		static void internal_setPhysicallyBasedAttenuation(ScriptLight* thisPtr, bool value);

		static void internal_getColor(ScriptLight* thisPtr, Color* color);
		static void internal_setColor(ScriptLight* thisPtr, Color color);

		static float internal_getRange(ScriptLight* thisPtr);
		static void internal_setRange(ScriptLight* thisPtr, float range);

		static float internal_getIntensity(ScriptLight* thisPtr);
		static void internal_setIntensity(ScriptLight* thisPtr, float intensity);

		static float internal_getSpotAngle(ScriptLight* thisPtr);
		static void internal_setSpotAngle(ScriptLight* thisPtr, float spotAngle);

		static float internal_getSpotFalloffAngle(ScriptLight* thisPtr);
		static void internal_setSpotFalloffAngle(ScriptLight* thisPtr, float spotFalloffAngle);

		static void internal_getBounds(ScriptLight* thisPtr, Sphere* bounds);

		static void internal_updateTransform(ScriptLight* thisPtr, ScriptSceneObject* parent);
		static void internal_onDestroy(ScriptLight* instance);
	};

	/** @} */
}