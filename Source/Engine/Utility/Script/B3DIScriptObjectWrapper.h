//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	class IScriptExportable;

	/** @addtogroup Script
	 *  @{
	 */

	/**
	 * Provides interoperability between a native object and a script object. This interface in particular is responsible for tracking their lifetimes:
	 *  - If the script object goes out of scope, the native object might be released if nothing else is referencing it.
	 *	- If the native object goes out of scope, this wrapper will be notified so it may either destroy the script object or safely fail method calls that try to use it in script.
	 *
	 * It is expected each native class exported to script will implement its own version of the script object wrapper, which would provide
	 * methods to create the script object, and allow the script object to call methods and access data on the native object.
	 */
	class B3D_EXPORT IScriptObjectWrapper
	{
	public:
		IScriptObjectWrapper(IScriptExportable* nativeObject);
		virtual ~IScriptObjectWrapper() = default;

		/** Returns the native object that is linked to the script object managed by the wrapper. */
		IScriptExportable* GetNativeObject() const { return mNativeObject; }

		/** Notifies the wrapper that the native object it is managing is about to be destroyed. */
		virtual void NotifyNativeObjectDestroyed();

		/** Notifies the wrapper that the script object is about to be destroyed. */
		virtual void NotifyScriptObjectDestroyed(bool isDestroyedDueToScriptReload);

	protected:
		friend class IScriptExportable;

		IScriptExportable* mNativeObject = nullptr;
	};

	/** @} */
} // namespace b3d
