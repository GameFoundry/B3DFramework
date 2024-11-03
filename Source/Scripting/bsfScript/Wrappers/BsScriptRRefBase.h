//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Image/BsTexture.h"
#include "BsScriptResourceWrapper.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	class ScriptResourceWrapper;
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Extends TScriptObjectWrapper by providing functionality required for types passed as values. */
	template<typename NativeType, typename SelfType, typename BaseType = ScriptObjectWrapper>
	class TScriptValueTypeWrapper : public TScriptObjectWrapper<SelfType, BaseType> // TODO - Move to its own file?
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

	/**	Interop class between C++ & CLR for RRefBase and RRef<T>. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRRefBase : public TScriptValueTypeWrapper<HResource, ScriptRRefBase>
	{
		using Super = TScriptValueTypeWrapper;
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "RRefBase")

		ScriptRRefBase(const HResource& nativeObject);

		static void SetupScriptBindings();

		/**
		 * Returns null as resource references cannot be created statically. Their script object type is mutable depending on the resource type they are referencing. Use CreateScriptObject() that accepts
		 * a resource handle instead.
		 */
		static MonoObject* CreateScriptObject(bool construct)
		{
			return nullptr;
		}
		/**
		 * Creates a new resource reference script object for the provided resource.
		 *
		 * @param	handle		Handle to the resource to wrap.
		 * @param	rawType		Class of the RRef type to use for wrapping the resource. If null then the resource
		 *						will be wrapped in a non-specific RRefBase object. Otherwise it will be wrapped in a
		 *						templated RRef<T> object. In the latter case caller is responsible for ensuring the
		 *						template parameter of RRef matches the actual resource type.
		 */
		static MonoObject* CreateScriptObject(const HResource& handle, ::MonoClass* rawType = nullptr);

		/** Creates a RRef type with the provided class bound as its template parameter. */
		static ::MonoClass* BindGenericParam(::MonoClass* param);

	private:
		friend class ScriptResourceManager;

		void NotifyScriptObjectDestroyed(bool isDestroyedDueToScriptReload) override;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static bool InternalIsLoaded(ScriptRRefBase* self);
		static MonoObject* InternalGetResource(ScriptRRefBase* self);
		static void InternalGetUuid(ScriptRRefBase* self, UUID* uuid);
		static MonoObject* InternalCastAs(ScriptRRefBase* self, MonoReflectionType* type);
	};

	/** @} */
} // namespace bs
