//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsMonoUtil.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Types of resources accessible from script code. */
	enum class ScriptResourceType // Note: Must be the same as C# enum ResourceType
	{
		Texture, SpriteTexture, Mesh, Font, Shader, ShaderInclude, Material, Prefab, 
		PlainText, ScriptCode, StringTable, GUISkin, PhysicsMaterial, PhysicsMesh, AudioClip, AnimationClip, Undefined
	};

	/**	Base class for all resource interop objects. */
	class BS_SCR_BE_EXPORT ScriptResourceBase : public PersistentScriptObjectBase
	{
	public:
		/**	Returns a generic handle to the internal wrapped resource. */
		virtual HResource getGenericHandle() const = 0;

		/**	Sets the internal resource this object wraps. */
		virtual void setResource(const HResource& resource) = 0;

		/** @copydoc ScriptObjectBase::beginRefresh */
		virtual ScriptObjectBackup beginRefresh() override;

		/** @copydoc ScriptObjectBase::endRefresh */
		virtual void endRefresh(const ScriptObjectBackup& backupData) override;

	protected:
		friend class ScriptResourceManager;

		ScriptResourceBase(MonoObject* instance);
		virtual ~ScriptResourceBase() {}

		/**	
		 * Triggered by the script resource managed when the native resource handle this object point to has been destroyed.
		 */
		virtual void notifyResourceDestroyed() { }

		/** Destroys the interop object, unless refresh is in progress in which case it is just prepared for re-creation. */
		void destroy();

		bool mRefreshInProgress;
	};

	/**	Base class for a specific resource's interop object. */
	template<class ScriptClass, class ResType>
	class BS_SCR_BE_EXPORT TScriptResource : public ScriptObject <ScriptClass, ScriptResourceBase>
	{
	public:
		/**	Returns a generic handle to the internal wrapped resource. */
		HResource getGenericHandle() const override { return mResource; }

		/**	Sets the internal resource this object wraps. */
		void setResource(const HResource& resource) override { mResource = static_resource_cast<ResType>(resource); }

		/**	Returns a handle to the internal wrapped resource. */
		const ResourceHandle<ResType>& getHandle() const { return mResource; }

	protected:
		friend class ScriptResourceManager;

		TScriptResource(MonoObject* instance, const ResourceHandle<ResType>& resource)
			:ScriptObject<ScriptClass, ScriptResourceBase>(instance), mResource(resource)
		{
			mManagedHandle = MonoUtil::newGCHandle(instance);

			BS_DEBUG_ONLY(mHandleValid = true);
		}

		virtual ~TScriptResource() {}

		/**
		 * Called after assembly reload starts to give the object a chance to restore the data backed up by the previous
		 * beginRefresh() call.
		 */
		virtual void endRefresh(const ScriptObjectBackup& backupData) override
		{
			BS_ASSERT(!mHandleValid);
			mManagedHandle = MonoUtil::newGCHandle(this->mManagedInstance);

			ScriptObject<ScriptClass, ScriptResourceBase>::endRefresh(backupData);
		}

		/**	
		 * Triggered by the script resource managed when the native resource handle this object point to has been destroyed.
		 */
		void notifyResourceDestroyed() override
		{
			MonoUtil::freeGCHandle(mManagedHandle);
			BS_DEBUG_ONLY(mHandleValid = false);
		}

		/**	Called when the managed instance gets finalized by the CLR. */
		void _onManagedInstanceDeleted() override
		{
			MonoUtil::freeGCHandle(mManagedHandle);
			BS_DEBUG_ONLY(mHandleValid = false);

			this->destroy();
		}

		ResourceHandle<ResType> mResource;
		uint32_t mManagedHandle;
		BS_DEBUG_ONLY(bool mHandleValid);
	};

	/**	Interop class between C++ & CLR for Resource. */
	class BS_SCR_BE_EXPORT ScriptResource : public ScriptObject<ScriptResource, ScriptResourceBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Resource")

		/**	Converts a RTTI id belonging to a resource type into a ScriptResourceType. */
		static ScriptResourceType getTypeFromTypeId(UINT32 typeId);

		/**	Converts a ScriptResourceType into a RTTI id belonging to that resource type. */
		static UINT32 getTypeIdFromType(ScriptResourceType type);

		/**	Converts a RTTI id belonging to a resource type into a managed class representing that type. */
		static MonoClass* getClassFromTypeId(UINT32 typeId);
	private:
		ScriptResource(MonoObject* instance)
			:ScriptObject(instance)
		{ }

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoString* internal_getName(ScriptResourceBase* nativeInstance);
		static MonoString* internal_getUUID(ScriptResourceBase* nativeInstance);
		static void internal_release(ScriptResourceBase* nativeInstance);
	};

	/** @} */
}