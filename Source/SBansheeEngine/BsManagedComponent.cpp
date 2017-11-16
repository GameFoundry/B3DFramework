//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsManagedComponent.h"
#include "RTTI/BsManagedComponentRTTI.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsMonoMethod.h"
#include "Serialization/BsMemorySerializer.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "BsScriptGameObjectManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Wrappers/BsScriptManagedComponent.h"
#include "BsMonoAssembly.h"
#include "BsPlayInEditorManager.h"

namespace bs
{
	ManagedComponent::ManagedComponent()
	{ }

	ManagedComponent::ManagedComponent(const HSceneObject& parent, MonoReflectionType* runtimeType)
		: Component(parent), mRuntimeType(runtimeType)
	{
		MonoUtil::getClassName(mRuntimeType, mNamespace, mTypeName);
		setName(mTypeName);
	}

	ManagedComponent::~ManagedComponent()
	{

	}

	MonoObject* ManagedComponent::getManagedInstance() const
	{
		return MonoUtil::getObjectFromGCHandle(mGCHandle);
	}

	ComponentBackupData ManagedComponent::backup(bool clearExisting)
	{
		ComponentBackupData backupData;

		// If type is not missing read data from actual managed instance, instead just 
		// return the data we backed up before the type was lost
		if (!mMissingType)
		{
			MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);
			SPtr<ManagedSerializableObject> serializableObject = ManagedSerializableObject::createFromExisting(instance);

			// Serialize the object information and its fields. We cannot just serialize the entire object because
			// the managed instance had to be created in a previous step. So we handle creation of the top level object manually.
			
			if (serializableObject != nullptr)
			{
				MemorySerializer ms;

				backupData.size = 0;
				backupData.data = ms.encode(serializableObject.get(), backupData.size);
			}
			else
			{
				backupData.size = 0;
				backupData.data = nullptr;
			}
		}
		else
		{
			MemorySerializer ms;

			backupData.size = 0;

			if (mSerializedObjectData != nullptr)
				backupData.data = ms.encode(mSerializedObjectData.get(), backupData.size);
			else
				backupData.data = nullptr;
		}

		if (clearExisting)
		{
			if (mGCHandle != 0)
			{
				MonoUtil::freeGCHandle(mGCHandle);
				mGCHandle = 0;
			}

			mManagedClass = nullptr;
			mRuntimeType = nullptr;
			mOnCreatedThunk = nullptr;
			mOnInitializedThunk = nullptr;
			mOnUpdateThunk = nullptr;
			mOnDestroyThunk = nullptr;
			mOnEnabledThunk = nullptr;
			mOnDisabledThunk = nullptr;
			mOnTransformChangedThunk = nullptr;
			mCalculateBoundsMethod = nullptr;
		}

		return backupData;
	}

	void ManagedComponent::restore(MonoObject* instance, const ComponentBackupData& data, bool missingType)
	{
		initialize(instance);
		mObjInfo = nullptr;

		if (instance != nullptr && data.data != nullptr)
		{
			MemorySerializer ms;

			GameObjectManager::instance().startDeserialization();
			SPtr<ManagedSerializableObject> serializableObject = std::static_pointer_cast<ManagedSerializableObject>(ms.decode(data.data, data.size));
			GameObjectManager::instance().endDeserialization();

			if (!missingType)
			{
				ScriptAssemblyManager::instance().getSerializableObjectInfo(mNamespace, mTypeName, mObjInfo);

				serializableObject->deserialize(instance, mObjInfo);
			}
			else
				mSerializedObjectData = serializableObject;
		}

		if (!missingType)
			mSerializedObjectData = nullptr;

		mMissingType = missingType;
		mRequiresReset = true;
	}

	void ManagedComponent::initialize(MonoObject* instance)
	{
		mFullTypeName = mNamespace + "." + mTypeName;
		
		mManagedClass = nullptr;
		if (instance != nullptr)
		{
			mGCHandle = MonoUtil::newGCHandle(instance, false);

			::MonoClass* monoClass = MonoUtil::getClass(instance);
			mRuntimeType = MonoUtil::getType(monoClass);

			mManagedClass = MonoManager::instance().findClass(monoClass);
		}

		mOnCreatedThunk = nullptr;
		mOnInitializedThunk = nullptr;
		mOnUpdateThunk = nullptr;
		mOnResetThunk = nullptr;
		mOnDestroyThunk = nullptr;
		mOnDisabledThunk = nullptr;
		mOnEnabledThunk = nullptr;
		mOnTransformChangedThunk = nullptr;
		mCalculateBoundsMethod = nullptr;

		while(mManagedClass != nullptr)
		{
			if (mOnCreatedThunk == nullptr)
			{
				MonoMethod* onCreatedMethod = mManagedClass->getMethod("OnCreate", 0);
				if (onCreatedMethod != nullptr)
					mOnCreatedThunk = (OnInitializedThunkDef)onCreatedMethod->getThunk();
			}

			if (mOnInitializedThunk == nullptr)
			{
				MonoMethod* onInitializedMethod = mManagedClass->getMethod("OnInitialize", 0);
				if (onInitializedMethod != nullptr)
					mOnInitializedThunk = (OnInitializedThunkDef)onInitializedMethod->getThunk();
			}

			if (mOnUpdateThunk == nullptr)
			{
				MonoMethod* onUpdateMethod = mManagedClass->getMethod("OnUpdate", 0);
				if (onUpdateMethod != nullptr)
					mOnUpdateThunk = (OnUpdateThunkDef)onUpdateMethod->getThunk();
			}

			if (mOnResetThunk == nullptr)
			{
				MonoMethod* onResetMethod = mManagedClass->getMethod("OnReset", 0);
				if (onResetMethod != nullptr)
					mOnResetThunk = (OnResetThunkDef)onResetMethod->getThunk();
			}

			if (mOnDestroyThunk == nullptr)
			{
				MonoMethod* onDestroyMethod = mManagedClass->getMethod("OnDestroy", 0);
				if (onDestroyMethod != nullptr)
					mOnDestroyThunk = (OnDestroyedThunkDef)onDestroyMethod->getThunk();
			}

			if (mOnDisabledThunk == nullptr)
			{
				MonoMethod* onDisableMethod = mManagedClass->getMethod("OnDisable", 0);
				if (onDisableMethod != nullptr)
					mOnDisabledThunk = (OnDisabledThunkDef)onDisableMethod->getThunk();
			}

			if (mOnEnabledThunk == nullptr)
			{
				MonoMethod* onEnableMethod = mManagedClass->getMethod("OnEnable", 0);
				if (onEnableMethod != nullptr)
					mOnEnabledThunk = (OnInitializedThunkDef)onEnableMethod->getThunk();
			}

			if (mOnTransformChangedThunk == nullptr)
			{
				MonoMethod* onTransformChangedMethod = mManagedClass->getMethod("OnTransformChanged", 1);
				if (onTransformChangedMethod != nullptr)
					mOnTransformChangedThunk = (OnTransformChangedThunkDef)onTransformChangedMethod->getThunk();
			}

			if(mCalculateBoundsMethod == nullptr)
				mCalculateBoundsMethod = mManagedClass->getMethod("CalculateBounds", 2);

			// Search for methods on base class if there is one
			MonoClass* baseClass = mManagedClass->getBaseClass();
			if (baseClass != ScriptManagedComponent::getMetaData()->scriptClass)
				mManagedClass = baseClass;
			else
				break;
		}

		if (mManagedClass != nullptr)
		{
			MonoAssembly* bansheeEngineAssembly = MonoManager::instance().getAssembly(ENGINE_ASSEMBLY);
			if (bansheeEngineAssembly == nullptr)
				BS_EXCEPT(InvalidStateException, String(ENGINE_ASSEMBLY) + " assembly is not loaded.");

			MonoClass* runInEditorAttrib = bansheeEngineAssembly->getClass("BansheeEngine", "RunInEditor");
			if (runInEditorAttrib == nullptr)
				BS_EXCEPT(InvalidStateException, "Cannot find RunInEditor managed class.");

			bool runInEditor = mManagedClass->getAttribute(runInEditorAttrib) != nullptr;
			if (runInEditor)
				setFlag(ComponentFlag::AlwaysRun, true);
		}
	}

	bool ManagedComponent::typeEquals(const Component& other)
	{
		if(Component::typeEquals(other))
		{
			const ManagedComponent& otherMC = static_cast<const ManagedComponent&>(other);

			// Not comparing MonoReflectionType directly because this needs to be able to work before instantiation
			return mNamespace == otherMC.getManagedNamespace() && mTypeName == otherMC.getManagedTypeName();
		}

		return false;
	}

	bool ManagedComponent::calculateBounds(Bounds& bounds)
	{
		MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);
		if (instance != nullptr && mCalculateBoundsMethod != nullptr)
		{
			AABox box;
			Sphere sphere;

			void* params[2];
			params[0] = &box;
			params[1] = &sphere;

			MonoObject* areBoundsValidObj = mCalculateBoundsMethod->invokeVirtual(instance, params);

			bool areBoundsValid;
			areBoundsValid = *(bool*)MonoUtil::unbox(areBoundsValidObj);

			bounds = Bounds(box, sphere);
			return areBoundsValid;
		}

		return Component::calculateBounds(bounds);
	}

	void ManagedComponent::update()
	{
		if (mOnUpdateThunk != nullptr)
		{
			MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnUpdateThunk, instance);
		}
	}

	void ManagedComponent::triggerOnReset()
	{
		if (mRequiresReset && mOnResetThunk != nullptr)
		{
			MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnResetThunk, instance);
		}

		mRequiresReset = false;
	}

	void ManagedComponent::_instantiate()
	{
		mObjInfo = nullptr;

		MonoObject* instance;
		if (!ScriptAssemblyManager::instance().getSerializableObjectInfo(mNamespace, mTypeName, mObjInfo))
		{
			instance = ScriptAssemblyManager::instance().getMissingComponentClass()->createInstance(true);

			initialize(instance);
			mMissingType = true;
		}
		else
		{
			instance = mObjInfo->mMonoClass->createInstance();

			initialize(instance);
			mMissingType = false;
		}

		// Find handle to self
		HManagedComponent componentHandle;
		if (SO() != nullptr)
		{
			const Vector<HComponent>& components = SO()->getComponents();
			for (auto& component : components)
			{
				if (component.get() == this)
				{
					componentHandle = component;
					break;
				}
			}
		}

		assert(componentHandle != nullptr);
		ScriptGameObjectManager::instance().createManagedScriptComponent(instance, componentHandle);
	}

	void ManagedComponent::onCreated()
	{
		MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);

		if (mSerializedObjectData != nullptr && !mMissingType)
		{
			mSerializedObjectData->deserialize(instance, mObjInfo);
			mSerializedObjectData = nullptr;
		}

		if (mOnCreatedThunk != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnCreatedThunk, instance);
		}

		triggerOnReset();
	}

	void ManagedComponent::onInitialized()
	{
		if (mOnInitializedThunk != nullptr)
		{
			MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnInitializedThunk, instance);
		}

		triggerOnReset();
	}

	void ManagedComponent::onDestroyed()
	{
		if (mOnDestroyThunk != nullptr)
		{
			MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnDestroyThunk, instance);
		}

		MonoUtil::freeGCHandle(mGCHandle);
		mGCHandle = 0;
	}

	void ManagedComponent::onEnabled()
	{
		if (mOnEnabledThunk != nullptr)
		{
			MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnEnabledThunk, instance);
		}
	}

	void ManagedComponent::onDisabled()
	{
		if (mOnDisabledThunk != nullptr)
		{
			MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnDisabledThunk, instance);
		}
	}

	void ManagedComponent::onTransformChanged(TransformChangedFlags flags)
	{
		if(mOnTransformChangedThunk != nullptr)
		{
			MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnTransformChangedThunk, instance, flags);
		}
	}

	RTTITypeBase* ManagedComponent::getRTTIStatic()
	{
		return ManagedComponentRTTI::instance();
	}

	RTTITypeBase* ManagedComponent::getRTTI() const
	{
		return ManagedComponent::getRTTIStatic();
	}
}