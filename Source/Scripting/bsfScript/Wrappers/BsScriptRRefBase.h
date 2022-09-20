//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Image/BsTexture.h"
#include "Wrappers/BsScriptResource.h"
#include "BsMonoClass.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for RRefBase and RRef<T>. */
	class BS_SCR_BE_EXPORT ScriptRRefBase : public ScriptObject<ScriptRRefBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "RRefBase")

		/**	Returns a weak handle to the resource referenced by this object. */
		ResourceHandle<Resource> GetHandle() const { return mResource; }

		/** Returns the managed version of this object. */
		MonoObject* GetManagedInstance() const;

		/**
		 * Creates a new managed RRefBase for the provided resource.
		 *
		 * @param[in]	handle	Handle to the resource to wrap.
		 * @param[in]	rawType	Class of the RRef type to use for wrapping the resource. If null then the resource
		 *						will be wrapped in a non-specific RRefBase object. Otherwise it will be wrapped in a
		 *						templated RRef<T> object. In the latter case caller is responsible for ensuring the
		 *						template parameter of RRef matches the actual resource type.
		 */
		template<class T>
		static ScriptRRefBase* Create(const ResourceHandle<T>& handle, ::MonoClass* rawType = nullptr)
		{
			return CreateInternal(handle, rawType);
		}

		/** Creates a RRef type with the provided class bound as its template parameter. */
		static ::MonoClass* BindGenericParam(::MonoClass* param);

	private:
		friend class ScriptResourceManager;

		ScriptRRefBase(MonoObject* instance, ResourceHandle<Resource> handle);
		~ScriptRRefBase();

		/** @copydoc ScriptObjectBase::_clearManagedInstance */
		void ClearManagedInstanceInternal() override;

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
		void OnManagedInstanceDeletedInternal(bool assemblyRefresh) override;

		/** Clears the internal cached ScriptResource reference. Should be called if the resource got destroyed. */
		void ClearResource() { mScriptResource = nullptr; }

		/** @copydoc create() */
		static ScriptRRefBase* CreateInternal(const ResourceHandle<Resource>& handle, ::MonoClass* rawType = nullptr);

		ResourceHandle<Resource> mResource;
		ScriptResourceBase* mScriptResource = nullptr;
		UINT32 mGCHandle;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static bool InternalIsLoaded(ScriptRRefBase* thisPtr);
		static MonoObject* InternalGetResource(ScriptRRefBase* thisPtr);
		static void InternalGetUuid(ScriptRRefBase* thisPtr, UUID* uuid);
		static MonoObject* InternalCastAs(ScriptRRefBase* thisPtr, MonoReflectionType* type);
	};

	/** @} */
}
