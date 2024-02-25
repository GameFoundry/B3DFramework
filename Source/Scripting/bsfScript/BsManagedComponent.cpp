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

using namespace bs;
ManagedComponent::ManagedComponent(const HSceneObject& parent, MonoReflectionType* runtimeType)
	: Component(parent), mRuntimeType(runtimeType)
{
	MonoUtil::GetClassName(mRuntimeType, mNamespace, mTypeName);
	SetName(mTypeName);
}

MonoObject* ManagedComponent::GetManagedInstance() const
{
	if(mOwner)
		return mOwner->GetManagedInstance();

	return nullptr;
}

RawBackupData ManagedComponent::Backup(bool clearExisting)
{
	RawBackupData backupData;

	// If type is not missing read data from actual managed instance, instead just
	// return the data we backed up before the type was lost
	if(!mMissingType)
	{
		MonoObject* instance = mOwner->GetManagedInstance();
		SPtr<ManagedSerializableObject> serializableObject = ManagedSerializableObject::CreateFromExisting(instance);

		// Serialize the object information and its fields. We cannot just serialize the entire object because
		// the managed instance had to be created in a previous step. So we handle creation of the top level object manually.

		if(serializableObject != nullptr)
		{
			SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
			BinarySerializer bs;

			bs.Encode(serializableObject.get(), stream);

			backupData.Size = (u32)stream->Size();
			backupData.Data = stream->DisownMemory();
		}
		else
		{
			backupData.Size = 0;
			backupData.Data = nullptr;
		}
	}
	else
	{
		SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();

		if(mSerializedObjectData != nullptr)
		{
			BinarySerializer bs;
			bs.Encode(mSerializedObjectData.get(), stream);
		}

		backupData.Size = (u32)stream->Size();
		backupData.Data = stream->DisownMemory();
	}

	if(clearExisting)
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

void ManagedComponent::Restore(const RawBackupData& data, bool missingType)
{
	Initialize(mOwner);
	mObjInfo = nullptr;

	MonoObject* instance = mOwner->GetManagedInstance();
	if(instance != nullptr && data.Data != nullptr)
	{
		BinarySerializer bs;

		CoreSerializationContext serzContext;
		serzContext.GoState = B3DMakeShared<GameObjectDeserializationState>();
		serzContext.GameObjectCollection = SO()->GetOwnerCollection().lock();

		auto serializableObject = std::static_pointer_cast<ManagedSerializableObject>(
			bs.Decode(B3DMakeShared<MemoryDataStream>(data.Data, data.Size), data.Size, BinarySerializerFlag::None, &serzContext));

		serzContext.GoState->Resolve();

		if(!missingType)
		{
			ScriptAssemblyManager::Instance().GetSerializableObjectInfo(mNamespace, mTypeName, mObjInfo);

			serializableObject->Deserialize(instance, mObjInfo);
		}
		else
			mSerializedObjectData = serializableObject;
	}

	if(!missingType)
		mSerializedObjectData = nullptr;

	mMissingType = missingType;
	mRequiresReset = true;
}

void ManagedComponent::Initialize(ScriptManagedComponent* owner)
{
	mOwner = owner;
	mFullTypeName = mNamespace + "." + mTypeName;

	MonoObject* instance = owner->GetManagedInstance();
	mManagedClass = nullptr;
	if(instance != nullptr)
	{
		::MonoClass* monoClass = MonoUtil::GetClass(instance);
		mRuntimeType = MonoUtil::GetType(monoClass);

		mManagedClass = MonoManager::Instance().FindClass(monoClass);
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
		if(mOnCreatedThunk == nullptr)
		{
			MonoMethod* onCreatedMethod = mManagedClass->GetMethod("OnCreate", 0);
			if(onCreatedMethod != nullptr)
				mOnCreatedThunk = (OnInitializedThunkDef)onCreatedMethod->GetThunk();
		}

		if(mOnInitializedThunk == nullptr)
		{
			MonoMethod* onInitializedMethod = mManagedClass->GetMethod("OnInitialize", 0);
			if(onInitializedMethod != nullptr)
				mOnInitializedThunk = (OnInitializedThunkDef)onInitializedMethod->GetThunk();
		}

		if(mOnUpdateThunk == nullptr)
		{
			MonoMethod* onUpdateMethod = mManagedClass->GetMethod("OnUpdate", 0);
			if(onUpdateMethod != nullptr)
				mOnUpdateThunk = (OnUpdateThunkDef)onUpdateMethod->GetThunk();
		}

		if(mOnResetThunk == nullptr)
		{
			MonoMethod* onResetMethod = mManagedClass->GetMethod("OnReset", 0);
			if(onResetMethod != nullptr)
				mOnResetThunk = (OnResetThunkDef)onResetMethod->GetThunk();
		}

		if(mOnDestroyThunk == nullptr)
		{
			MonoMethod* onDestroyMethod = mManagedClass->GetMethod("OnDestroy", 0);
			if(onDestroyMethod != nullptr)
				mOnDestroyThunk = (OnDestroyedThunkDef)onDestroyMethod->GetThunk();
		}

		if(mOnDisabledThunk == nullptr)
		{
			MonoMethod* onDisableMethod = mManagedClass->GetMethod("OnDisable", 0);
			if(onDisableMethod != nullptr)
				mOnDisabledThunk = (OnDisabledThunkDef)onDisableMethod->GetThunk();
		}

		if(mOnEnabledThunk == nullptr)
		{
			MonoMethod* onEnableMethod = mManagedClass->GetMethod("OnEnable", 0);
			if(onEnableMethod != nullptr)
				mOnEnabledThunk = (OnInitializedThunkDef)onEnableMethod->GetThunk();
		}

		if(mOnTransformChangedThunk == nullptr)
		{
			MonoMethod* onTransformChangedMethod = mManagedClass->GetMethod("OnTransformChanged", 1);
			if(onTransformChangedMethod != nullptr)
				mOnTransformChangedThunk = (OnTransformChangedThunkDef)onTransformChangedMethod->GetThunk();
		}

		if(mCalculateBoundsMethod == nullptr)
			mCalculateBoundsMethod = mManagedClass->GetMethod("CalculateBounds", 2);

		// Search for methods on base class if there is one
		MonoClass* baseClass = mManagedClass->GetBaseClass();
		if(baseClass != ScriptManagedComponent::GetMetaData()->ScriptClass)
			mManagedClass = baseClass;
		else
			break;
	}

	if(mManagedClass != nullptr)
	{
		MonoAssembly* engineAssembly = MonoManager::Instance().GetAssembly(kEngineAssembly);
		if(engineAssembly == nullptr)
			B3D_EXCEPT(InvalidStateException, String(kEngineAssembly) + " assembly is not loaded.");

		MonoClass* runInEditorAttrib = engineAssembly->GetClass(kEngineNs, "RunInEditor");
		if(runInEditorAttrib == nullptr)
			B3D_EXCEPT(InvalidStateException, "Cannot find RunInEditor managed class.");

		bool runInEditor = mManagedClass->GetAttribute(runInEditorAttrib) != nullptr;
		if(runInEditor)
			SetFlag(ComponentFlag::AlwaysRun, true);
	}
}

bool ManagedComponent::TypeEquals(const Component& other)
{
	if(Component::TypeEquals(other))
	{
		const ManagedComponent& otherMC = static_cast<const ManagedComponent&>(other);

		// Not comparing MonoReflectionType directly because this needs to be able to work before instantiation
		return mNamespace == otherMC.GetManagedNamespace() && mTypeName == otherMC.GetManagedTypeName();
	}

	return false;
}

bool ManagedComponent::CalculateBounds(Bounds& bounds)
{
	MonoObject* instance = nullptr;

	if(mOwner)
		instance = mOwner->GetManagedInstance();

	if(instance != nullptr && mCalculateBoundsMethod != nullptr)
	{
		AABox box;
		Sphere sphere;

		void* params[2];
		params[0] = &box;
		params[1] = &sphere;

		MonoObject* areBoundsValidObj = mCalculateBoundsMethod->InvokeVirtual(instance, params);

		bool areBoundsValid;
		areBoundsValid = *(bool*)MonoUtil::Unbox(areBoundsValidObj);

		bounds = Bounds(box, sphere);
		return areBoundsValid;
	}

	return Component::CalculateBounds(bounds);
}

void ManagedComponent::Update()
{
	if(mOnUpdateThunk != nullptr)
	{
		MonoObject* instance = mOwner->GetManagedInstance();

		// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
		// for some extra speed.
		MonoUtil::InvokeThunk(mOnUpdateThunk, instance);
	}
}

void ManagedComponent::TriggerOnReset()
{
	if(mRequiresReset && mOnResetThunk != nullptr)
	{
		MonoObject* instance = mOwner->GetManagedInstance();

		// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
		// for some extra speed.
		MonoUtil::InvokeThunk(mOnResetThunk, instance);
	}

	mRequiresReset = false;
}

void ManagedComponent::InstantiateInternal()
{
	mObjInfo = nullptr;

	MonoObject* instance;
	if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(mNamespace, mTypeName, mObjInfo))
	{
		instance = ScriptAssemblyManager::Instance().GetBuiltinClasses().MissingComponentClass->CreateInstance(true);
		mMissingType = true;
	}
	else
	{
		instance = mObjInfo->MMonoClass->CreateInstance();
		mMissingType = false;
	}

	// Find handle to self
	HManagedComponent componentHandle;
	if(SO() != nullptr)
	{
		const Vector<HComponent>& components = SO()->GetComponents();
		for(auto& component : components)
		{
			if(component.Get() == this)
			{
				componentHandle = B3DStaticGameObjectCast<ManagedComponent>(component);
				break;
			}
		}
	}

	B3D_ASSERT(componentHandle != nullptr);
	ScriptGameObjectManager::Instance().CreateManagedScriptComponent(instance, componentHandle);
}

void ManagedComponent::OnCreated()
{
	MonoObject* instance = mOwner->GetManagedInstance();

	if(mSerializedObjectData != nullptr && !mMissingType)
	{
		mSerializedObjectData->Deserialize(instance, mObjInfo);
		mSerializedObjectData = nullptr;
	}

	if(mOnCreatedThunk != nullptr)
	{
		// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
		// for some extra speed.
		MonoUtil::InvokeThunk(mOnCreatedThunk, instance);
	}

	TriggerOnReset();
}

void ManagedComponent::OnInitialized()
{
	if(mOnInitializedThunk != nullptr)
	{
		MonoObject* instance = mOwner->GetManagedInstance();

		// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
		// for some extra speed.
		MonoUtil::InvokeThunk(mOnInitializedThunk, instance);
	}

	TriggerOnReset();
}

void ManagedComponent::OnDestroyed()
{
	if(mOnDestroyThunk != nullptr)
	{
		MonoObject* instance = mOwner->GetManagedInstance();

		// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
		// for some extra speed.
		MonoUtil::InvokeThunk(mOnDestroyThunk, instance);
	}
}

void ManagedComponent::OnEnabled()
{
	if(mOnEnabledThunk != nullptr)
	{
		MonoObject* instance = mOwner->GetManagedInstance();

		// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
		// for some extra speed.
		MonoUtil::InvokeThunk(mOnEnabledThunk, instance);
	}
}

void ManagedComponent::OnDisabled()
{
	if(mOnDisabledThunk != nullptr)
	{
		MonoObject* instance = mOwner->GetManagedInstance();

		// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
		// for some extra speed.
		MonoUtil::InvokeThunk(mOnDisabledThunk, instance);
	}
}

void ManagedComponent::OnTransformChanged(TransformChangedFlags flags)
{
	if(mOnTransformChangedThunk != nullptr)
	{
		MonoObject* instance = mOwner->GetManagedInstance();

		// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
		// for some extra speed.
		MonoUtil::InvokeThunk(mOnTransformChangedThunk, instance, flags);
	}
}

RTTITypeBase* ManagedComponent::GetRttiStatic()
{
	return ManagedComponentRTTI::Instance();
}

RTTITypeBase* ManagedComponent::GetRtti() const
{
	return ManagedComponent::GetRttiStatic();
}
