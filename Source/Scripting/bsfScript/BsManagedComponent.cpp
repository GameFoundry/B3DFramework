//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsManagedComponent.h"
#include "RTTI/BsManagedComponentRTTI.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsMonoMethod.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "BsScriptGameObjectManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Wrappers/BsScriptManagedComponent.h"
#include "BsMonoAssembly.h"
#include "BsPlayInEditor.h"
#include "Utility/BsUtility.h"
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	ManagedComponent::ManagedComponent(const HSceneObject& parent, MonoReflectionType* runtimeType)
		: Component(parent), mRuntimeType(runtimeType)
	{
		MonoUtil::getClassName(mRuntimeType, mNamespace, mTypeName);
		setName(mTypeName);
	}

	MonoObject* ManagedComponent::getManagedInstance() const
	{
		if(mOwner)
			return mOwner->getManagedInstance();

		return nullptr;
	}

	RawBackupData ManagedComponent::backup(bool clearExisting)
	{
		RawBackupData backupData;

		// If type is not missing read data from actual managed instance, instead just
		// return the data we backed up before the type was lost
		if (!mMissingType)
		{
			MonoObject* instance = mOwner->getManagedInstance();
			SPtr<ManagedSerializableObject> serializableObject = ManagedSerializableObject::createFromExisting(instance);

			// Serialize the object information and its fields. We cannot just serialize the entire object because
			// the managed instance had to be created in a previous step. So we handle creation of the top level object manually.
			
			if (serializableObject != nullptr)
			{
				SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>();
				BinarySerializer bs;

				bs.encode(serializableObject.get(), stream);

				backupData.size = (UINT32)stream->size();
				backupData.data = stream->disownMemory();
			}
			else
			{
				backupData.size = 0;
				backupData.data = nullptr;
			}
		}
		else
		{
				SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>();

			if (mSerializedObjectData != nullptr)
			{
				BinarySerializer bs;
				bs.encode(mSerializedObjectData.get(), stream);
			}

			backupData.size = (UINT32)stream->size();
			backupData.data = stream->disownMemory();
		}

		if (clearExisting)
		{
			mManagedClass = nullptr;
			mRuntimeType = nullptr;
			mOnCreatedThunk = nullptr;
			mOnInitializedThunk = nullptr;
			mOnUpdateThunk = nullptr;
			mOnDestroyThunk = nullptr;
			mOnResetThunk = nullptr;
			mOnEnabledThunk = nullptr;
			mOnDisabledThunk = nullptr;
			mOnTransformChangedThunk = nullptr;
			mCalculateBoundsMethod = nullptr;
		}

		return backupData;
	}

	void ManagedComponent::restore(const RawBackupData& data, bool missingType)
	{
		initialize(mOwner);
		mObjInfo = nullptr;

		MonoObject* instance = mOwner->getManagedInstance();
		if (instance != nullptr && data.data != nullptr)
		{
			BinarySerializer bs;

			CoreSerializationContext serzContext;
			serzContext.goState = bs_shared_ptr_new<GameObjectDeserializationState>();

			auto serializableObject = std::static_pointer_cast<ManagedSerializableObject>(
				bs.decode(bs_shared_ptr_new<MemoryDataStream>(data.data, data.size), data.size,
					BinarySerializerFlag::None, &serzContext));

			serzContext.goState->resolve();

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

	void ManagedComponent::initialize(ScriptManagedComponent* owner)
	{
		mOwner = owner;
		mFullTypeName = mNamespace + "." + mTypeName;
		
		MonoObject* instance = owner->getManagedInstance();
		mManagedClass = nullptr;
		if (instance != nullptr)
		{
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
			MonoAssembly* engineAssembly = MonoManager::instance().getAssembly(ENGINE_ASSEMBLY);
			if (engineAssembly == nullptr)
				BS_EXCEPT(InvalidStateException, String(ENGINE_ASSEMBLY) + " assembly is not loaded.");

			MonoClass* runInEditorAttrib = engineAssembly->getClass(ENGINE_NS, "RunInEditor");
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
		MonoObject* instance = nullptr;
		
		if(mOwner)
			instance = mOwner->getManagedInstance();

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
			MonoObject* instance = mOwner->getManagedInstance();

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnUpdateThunk, instance);
		}
	}

	void ManagedComponent::triggerOnReset()
	{
		if (mRequiresReset && mOnResetThunk != nullptr)
		{
			MonoObject* instance = mOwner->getManagedInstance();

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
			instance = ScriptAssemblyManager::instance().getBuiltinClasses().missingComponentClass->createInstance(true);
			mMissingType = true;
		}
		else
		{
			instance = mObjInfo->mMonoClass->createInstance();
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
					componentHandle = static_object_cast<ManagedComponent>(component);
					break;
				}
			}
		}

		assert(componentHandle != nullptr);
		ScriptGameObjectManager::instance().createManagedScriptComponent(instance, componentHandle);
	}

	void ManagedComponent::onCreated()
	{
		MonoObject* instance = mOwner->getManagedInstance();

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
			MonoObject* instance = mOwner->getManagedInstance();

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
			MonoObject* instance = mOwner->getManagedInstance();

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnDestroyThunk, instance);
		}
	}

	void ManagedComponent::onEnabled()
	{
		if (mOnEnabledThunk != nullptr)
		{
			MonoObject* instance = mOwner->getManagedInstance();

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnEnabledThunk, instance);
		}
	}

	void ManagedComponent::onDisabled()
	{
		if (mOnDisabledThunk != nullptr)
		{
			MonoObject* instance = mOwner->getManagedInstance();

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnDisabledThunk, instance);
		}
	}

	void ManagedComponent::onTransformChanged(TransformChangedFlags flags)
	{
		if(mOnTransformChangedThunk != nullptr)
		{
			MonoObject* instance = mOwner->getManagedInstance();

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
