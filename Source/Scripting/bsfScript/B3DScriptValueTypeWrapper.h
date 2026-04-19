//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptResourceWrapper.h"

namespace b3d
{
	class ScriptResourceWrapper;

	/** @addtogroup bsfScript
	 *  @{
	 */

	/** Extends TScriptObjectWrapper by providing functionality required for types passed as values. */
	template<typename NativeType, typename SelfType, typename BaseType = ScriptObjectWrapper>
	class TScriptValueTypeWrapper : public TScriptObjectWrapper<SelfType, BaseType>
	{
	public:
		TScriptValueTypeWrapper(const NativeType& nativeObject)
			: TScriptObjectWrapper<SelfType, BaseType>(nullptr), mNativeObject(nativeObject)
		{ }

		NativeType& GetNativeObject() { return mNativeObject; }
		virtual ScriptObjectLifetimeTrackingMode GetLifetimeTrackingMode() const { return ScriptObjectLifetimeTrackingMode::WeakHandle; }

		/**
		 * Creates a new script object and a script object wrapper of @p SelfType, and associates them with the provided native object. Should not be called if @p nativeObject
		 * already has an associated script object.
		 */
		static MonoObject* CreateScriptObjectAndWrapper(const NativeType& nativeObject)
		{
			MonoObject* const scriptObject = SelfType::CreateScriptObject(false);
			ScriptObjectWrapper::Create<SelfType>(nativeObject, scriptObject);

			return scriptObject;
		}

	protected:
		NativeType mNativeObject;
	};

	/** @} */
} // namespace b3d
