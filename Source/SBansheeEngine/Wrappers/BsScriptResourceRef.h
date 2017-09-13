//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	/**	Interop class between C++ & CLR for ResourceRef. */
	class BS_SCR_BE_EXPORT ScriptResourceRef : public ScriptObject<ScriptResourceRef>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "ResourceRef")

		/**
		 * Creates a new managed ResourceRef for the provided resource.
		 *
		 * @param[in]	handle	Handle to the resource to wrap.
		 */
		template<class T>
		static MonoObject* create(const WeakResourceHandle<T>& handle)
		{
			return createInternal(handle);
		}

		/**	Returns a weak handle to the resource referenced by this object. */
		WeakResourceHandle<Resource> getHandle() const { return mResource; }

	private:
		friend class ScriptResourceRefBase;

		ScriptResourceRef(MonoObject* instance, const WeakResourceHandle<Resource>& handle);

		/**
		 * Creates a new managed ResourceRef for the provided resource type.
		 *
		 * @param[in]	handle	Handle to the resource to wrap.
		 */
		static MonoObject* createInternal(const WeakResourceHandle<Resource>& handle);

		WeakResourceHandle<Resource> mResource;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static bool internal_IsLoaded(ScriptResourceRef* nativeInstance);
		static MonoObject* internal_GetResource(ScriptResourceRef* nativeInstance);
		static void internal_GetUUID(ScriptResourceRef* thisPtr, UUID* uuid);
	};

	/** @} */
}