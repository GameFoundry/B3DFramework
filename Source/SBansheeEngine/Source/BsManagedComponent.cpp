//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsManagedComponent.h"
#include "BsManagedComponentRTTI.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsMonoMethod.h"
#include "BsMemorySerializer.h"
#include "BsManagedSerializableObject.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsScriptManagedComponent.h"
#include "BsMonoAssembly.h"
#include "BsPlayInEditorManager.h"

namespace bs
{
	ManagedComponent::ManagedComponent()
		: mManagedInstance(nullptr), mManagedClass(nullptr), mRuntimeType(nullptr), mManagedHandle(0), mRunInEditor(false)
		, mRequiresReset(true), mMissingType(false), mOnCreatedThunk(nullptr), mOnInitializedThunk(nullptr)
		, mOnUpdateThunk(nullptr), mOnResetThunk(nullptr), mOnDestroyThunk(nullptr), mOnDisabledThunk(nullptr)
		, mOnEnabledThunk(nullptr), mOnTransformChangedThunk(nullptr), mCalculateBoundsMethod(nullptr)
	{ }

	ManagedComponent::ManagedComponent(const HSceneObject& parent, MonoReflectionType* runtimeType)
		: Component(parent), mManagedInstance(nullptr), mManagedClass(nullptr), mRuntimeType(runtimeType)
		, mManagedHandle(0), mRunInEditor(false), mRequiresReset(true), mMissingType(false), mOnCreatedThunk(nullptr)
		, mOnInitializedThunk(nullptr), mOnUpdateThunk(nullptr), mOnResetThunk(nullptr), mOnDestroyThunk(nullptr)
		, mOnDisabledThunk(nullptr), mOnEnabledThunk(nullptr), mOnTransformChangedThunk(nullptr)
		, mCalculateBoundsMethod(nullptr)
	{
		MonoUtil::getClassName(mRuntimeType, mNamespace, mTypeName);
		setName(mTypeName);
	}

	ManagedComponent::~ManagedComponent()
	{

	}

	ComponentBackupData ManagedComponent::backup(bool clearExisting)
	{
		ComponentBackupData backupData;

		// If type is not missing read data from actual managed instance, instead just 
		// return the data we backed up before the type was lost
		if (!mMissingType)
		{
			SPtr<ManagedSerializableObject> serializableObject = ManagedSerializableObject::createFromExisting(mManagedInstance);

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
			if (mManagedInstance != nullptr)
			{
				mManagedInstance = nullptr;
				MonoUtil::freeGCHandle(mManagedHandle);
				mManagedHandle = 0;
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

	void ManagedComponent::initialize(MonoObject* object)
	{
		mFullTypeName = mNamespace + "." + mTypeName;
		mManagedInstance = object;
		
		mManagedClass = nullptr;
		if (mManagedInstance != nullptr)
		{
			mManagedHandle = MonoUtil::newGCHandle(mManagedInstance);

			::MonoClass* monoClass = MonoUtil::getClass(object);
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

			mRunInEditor = mManagedClass->getAttribute(runInEditorAttrib) != nullptr;
		}
		else
			mRunInEditor = false;
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
		if (mManagedInstance != nullptr && mCalculateBoundsMethod != nullptr)
		{
			AABox box;
			Sphere sphere;

			void* params[2];
			params[0] = &box;
			params[1] = &sphere;

			MonoObject* areBoundsValidObj = mCalculateBoundsMethod->invokeVirtual(mManagedInstance, params);

			bool areBoundsValid;
			areBoundsValid = *(bool*)MonoUtil::unbox(areBoundsValidObj);

			bounds = Bounds(box, sphere);
			return areBoundsValid;
		}

		return Component::calculateBounds(bounds);
	}

	void ManagedComponent::update()
	{
		if (PlayInEditorManager::instance().getState() != PlayInEditorState::Playing && !mRunInEditor)
			return;

		assert(mManagedInstance != nullptr);

		if (mOnUpdateThunk != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnUpdateThunk, mManagedInstance);
		}
	}

	void ManagedComponent::triggerOnInitialize()
	{
		if (PlayInEditorManager::instance().getState() == PlayInEditorState::Stopped && !mRunInEditor)
			return;

		if (mOnInitializedThunk != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnInitializedThunk, mManagedInstance);
		}
	}

	void ManagedComponent::triggerOnReset()
	{
		assert(mManagedInstance != nullptr);

		if (mRequiresReset && mOnResetThunk != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnResetThunk, mManagedInstance);
		}

		mRequiresReset = false;
	}

	void ManagedComponent::triggerOnEnable()
	{
		if (PlayInEditorManager::instance().getState() == PlayInEditorState::Stopped && !mRunInEditor)
			return;

		assert(mManagedInstance != nullptr);

		if (mOnEnabledThunk != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnEnabledThunk, mManagedInstance);
		}
	}

	void ManagedComponent::_instantiate()
	{
		mObjInfo = nullptr;
		if (!ScriptAssemblyManager::instance().getSerializableObjectInfo(mNamespace, mTypeName, mObjInfo))
		{
			MonoObject* instance = ScriptAssemblyManager::instance().getMissingComponentClass()->createInstance(true);

			initialize(instance);
			mMissingType = true;
		}
		else
		{
			initialize(mObjInfo->mMonoClass->createInstance());
			mMissingType = false;
		}

		assert(mManagedInstance != nullptr);

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
		ScriptGameObjectManager::instance().createManagedScriptComponent(mManagedInstance, componentHandle);
	}

	void ManagedComponent::onCreated()
	{
		assert(mManagedInstance != nullptr);

		if (mSerializedObjectData != nullptr && !mMissingType)
		{
			mSerializedObjectData->deserialize(mManagedInstance, mObjInfo);
			mSerializedObjectData = nullptr;
		}

		if (mOnCreatedThunk != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnCreatedThunk, mManagedInstance);
		}

		triggerOnReset();
	}

	void ManagedComponent::onInitialized()
	{
		assert(mManagedInstance != nullptr);

		triggerOnInitialize();
		triggerOnReset();
	}

	void ManagedComponent::onDestroyed()
	{
		assert(mManagedInstance != nullptr);

		if (mOnDestroyThunk != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnDestroyThunk, mManagedInstance);
		}

		mManagedInstance = nullptr;
		MonoUtil::freeGCHandle(mManagedHandle);
	}

	void ManagedComponent::onEnabled()
	{
		triggerOnEnable();
	}

	void ManagedComponent::onDisabled()
	{
		if (PlayInEditorManager::instance().getState() == PlayInEditorState::Stopped && !mRunInEditor)
			return;

		assert(mManagedInstance != nullptr);

		if (mOnDisabledThunk != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnDisabledThunk, mManagedInstance);
		}
	}

	void ManagedComponent::onTransformChanged(TransformChangedFlags flags)
	{
		if (PlayInEditorManager::instance().getState() == PlayInEditorState::Stopped && !mRunInEditor)
			return;

		if(mOnTransformChangedThunk != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnTransformChangedThunk, mManagedInstance, flags);
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