//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsSceneObject.h"
#include "Scene/BsComponent.h"
#include "Scene/BsSceneManager.h"
#include "Error/BsException.h"
#include "Debug/BsDebug.h"
#include "Private/RTTI/BsSceneObjectRTTI.h"
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsDataStream.h"
#include "Scene/BsGameObjectManager.h"
#include "Scene/BsPrefabUtility.h"
#include "Math/BsMatrix3.h"
#include "BsCoreApplication.h"

namespace bs
{
	SceneObject::SceneObject(const String& name, UINT32 flags)
		: GameObject(), mFlags(flags)
	{
		setName(name);
	}

	SceneObject::~SceneObject()
	{
		if(!mThisHandle.isDestroyed())
		{
			BS_LOG(Warning, Scene, "Object is being deleted without being destroyed first? {0}", mName);
			destroyInternal(mThisHandle, true);
		}
	}

	HSceneObject SceneObject::create(const String& name, UINT32 flags)
	{
		HSceneObject newObject = createInternal(name, flags);

		if (newObject->isInstantiated())
			gSceneManager().registerNewSO(newObject);

		return newObject;
	}

	HSceneObject SceneObject::createInternal(const String& name, UINT32 flags)
	{
		SPtr<SceneObject> sceneObjectPtr = SPtr<SceneObject>(new (bs_alloc<SceneObject>()) SceneObject(name, flags),
			&bs_delete<SceneObject>, StdAlloc<SceneObject>());
		sceneObjectPtr->mUUID = UUIDGenerator::generateRandom();

		HSceneObject sceneObject = static_object_cast<SceneObject>(
			GameObjectManager::instance().registerObject(sceneObjectPtr));
		sceneObject->mThisHandle = sceneObject;

		return sceneObject;
	}

	HSceneObject SceneObject::createInternal(const SPtr<SceneObject>& soPtr)
	{
		HSceneObject sceneObject = static_object_cast<SceneObject>(
			GameObjectManager::instance().registerObject(soPtr));
		sceneObject->mThisHandle = sceneObject;

		return sceneObject;
	}

	void SceneObject::destroy(bool immediate)
	{
		// Parent is our owner, so when his reference to us is removed, delete might be called.
		// So make sure this is the last thing we do.
		if(mParent != nullptr)
		{
			if(!mParent.isDestroyed())
				mParent->removeChild(mThisHandle);

			mParent = nullptr;
		}

		destroyInternal(mThisHandle, immediate);
	}

	void SceneObject::destroyInternal(GameObjectHandleBase& handle, bool immediate)
	{
		if (immediate)
		{
			for (auto iter = mChildren.begin(); iter != mChildren.end(); ++iter)
				(*iter)->destroyInternal(*iter, true);

			mChildren.clear();

			// It's important to remove the elements from the array as soon as they're destroyed, as OnDestroy callbacks
			// for components might query the SO's components, and we want to only return live ones
			while (!mComponents.empty())
			{
				HComponent component = mComponents.back();
				component->_setIsDestroyed();

				if (isInstantiated())
					gSceneManager()._notifyComponentDestroyed(component, immediate);

				component->destroyInternal(component, true);
				mComponents.erase(mComponents.end() - 1);
			}

			GameObjectManager::instance().unregisterObject(handle);
		}
		else
			GameObjectManager::instance().queueForDestroy(handle);
	}

	void SceneObject::_setInstanceData(GameObjectInstanceDataPtr& other)
	{
		GameObject::_setInstanceData(other);

		// Instance data changed, so make sure to refresh the handles to reflect that
		SPtr<SceneObject> thisPtr = mThisHandle.getInternalPtr();
		mThisHandle._setHandleData(thisPtr);
	}

	UUID SceneObject::getPrefabLink(bool onlyDirect) const
	{
		const SceneObject* curObj = this;

		while (curObj != nullptr)
		{
			if (!curObj->mPrefabLinkUUID.empty())
				return curObj->mPrefabLinkUUID;

			if (curObj->mParent != nullptr && !onlyDirect)
				curObj = curObj->mParent.get();
			else
				curObj = nullptr;
		}

		return UUID::EMPTY;
	}

	HSceneObject SceneObject::getPrefabParent() const
	{
		HSceneObject curObj = mThisHandle;

		while (curObj != nullptr)
		{
			if (!curObj->mPrefabLinkUUID.empty())
				return curObj;

			if (curObj->mParent != nullptr)
				curObj = curObj->mParent;
			else
				curObj = nullptr;
		}

		return curObj;
	}

	void SceneObject::breakPrefabLink()
	{
		SceneObject* rootObj = this;

		while (rootObj != nullptr)
		{
			if (!rootObj->mPrefabLinkUUID.empty())
				break;

			if (rootObj->mParent != nullptr)
				rootObj = rootObj->mParent.get();
			else
				rootObj = nullptr;
		}

		if (rootObj != nullptr)
		{
			rootObj->mPrefabLinkUUID = UUID::EMPTY;
			rootObj->mPrefabDiff = nullptr;
			PrefabUtility::clearPrefabIds(rootObj->getHandle(), true, false);
		}
	}

	bool SceneObject::hasFlag(UINT32 flag) const
	{
		return (mFlags & flag) != 0;
	}

	void SceneObject::_setFlags(UINT32 flags)
	{
		mFlags |= flags;

		for (auto& child : mChildren)
			child->_setFlags(flags);
	}

	void SceneObject::_unsetFlags(UINT32 flags)
	{
		mFlags &= ~flags;

		for (auto& child : mChildren)
			child->_unsetFlags(flags);
	}

	void SceneObject::_instantiate(bool prefabOnly)
	{
		std::function<void(SceneObject*)> instantiateRecursive = [&](SceneObject* obj)
		{
			obj->mFlags &= ~SOF_DontInstantiate;

			if (obj->mParent == nullptr)
				gSceneManager().registerNewSO(obj->mThisHandle);

			for (auto& component : obj->mComponents)
				component->_instantiate();

			for (auto& child : obj->mChildren)
			{
				if(!prefabOnly || child->mPrefabLinkUUID.empty())
					instantiateRecursive(child.get());
			}
		};

		std::function<void(SceneObject*)> triggerEventsRecursive = [&](SceneObject* obj)
		{
			for (auto& component : obj->mComponents)
				gSceneManager()._notifyComponentCreated(component, obj->getActive());

			for (auto& child : obj->mChildren)
			{
				if (!prefabOnly || child->mPrefabLinkUUID.empty())
					triggerEventsRecursive(child.get());
			}
		};

		instantiateRecursive(this);
		triggerEventsRecursive(this);
	}

	/************************************************************************/
	/* 								Transform	                     		*/
	/************************************************************************/

	void SceneObject::setPosition(const Vector3& position)
	{
		if (mMobility == ObjectMobility::Movable)
		{
			mLocalTfrm.setPosition(position);
			notifyTransformChanged(TCF_Transform);
		}
	}

	void SceneObject::setRotation(const Quaternion& rotation)
	{
		if (mMobility == ObjectMobility::Movable)
		{
			mLocalTfrm.setRotation(rotation);
			notifyTransformChanged(TCF_Transform);
		}
	}

	void SceneObject::setScale(const Vector3& scale)
	{
		if (mMobility == ObjectMobility::Movable)
		{
			mLocalTfrm.setScale(scale);
			notifyTransformChanged(TCF_Transform);
		}
	}

	void SceneObject::setWorldPosition(const Vector3& position)
	{
		if(mMobility != ObjectMobility::Movable)
			return;

		if (mParent != nullptr)
			mLocalTfrm.setWorldPosition(position, mParent->getTransform());
		else
			mLocalTfrm.setPosition(position);

		notifyTransformChanged(TCF_Transform);
	}

	void SceneObject::setWorldRotation(const Quaternion& rotation)
	{
		if (mMobility != ObjectMobility::Movable)
			return;

		if (mParent != nullptr)
			mLocalTfrm.setWorldRotation(rotation, mParent->getTransform());
		else
			mLocalTfrm.setRotation(rotation);

		notifyTransformChanged(TCF_Transform);
	}

	void SceneObject::setWorldScale(const Vector3& scale)
	{
		if (mMobility != ObjectMobility::Movable)
			return;

		if (mParent != nullptr)
			mLocalTfrm.setWorldScale(scale, mParent->getTransform());
		else
			mLocalTfrm.setScale(scale);

		notifyTransformChanged(TCF_Transform);
	}

	const Transform& SceneObject::getTransform() const
	{
		if (!isCachedWorldTfrmUpToDate())
			updateWorldTfrm();

		return mWorldTfrm;
	}

	void SceneObject::lookAt(const Vector3& location, const Vector3& up)
	{
		const Transform& worldTfrm = getTransform();

		Vector3 forward = location - worldTfrm.getPosition();
		
		Quaternion rotation = worldTfrm.getRotation();
		rotation.lookRotation(forward, up);
		setWorldRotation(rotation);
	}

	const Matrix4& SceneObject::getWorldMatrix() const
	{
		if (!isCachedWorldTfrmUpToDate())
			updateWorldTfrm();

		return mCachedWorldTfrm;
	}

	Matrix4 SceneObject::getInvWorldMatrix() const
	{
		if (!isCachedWorldTfrmUpToDate())
			updateWorldTfrm();

		Matrix4 worldToLocal = mWorldTfrm.getInvMatrix();
		return worldToLocal;
	}

	const Matrix4& SceneObject::getLocalMatrix() const
	{
		if (!isCachedLocalTfrmUpToDate())
			updateLocalTfrm();

		return mCachedLocalTfrm;
	}

	void SceneObject::move(const Vector3& vec)
	{
		if (mMobility == ObjectMobility::Movable)
		{
			mLocalTfrm.move(vec);
			notifyTransformChanged(TCF_Transform);
		}
	}

	void SceneObject::moveRelative(const Vector3& vec)
	{
		if (mMobility == ObjectMobility::Movable)
		{
			mLocalTfrm.moveRelative(vec);
			notifyTransformChanged(TCF_Transform);
		}
	}

	void SceneObject::rotate(const Vector3& axis, const Radian& angle)
	{
		if (mMobility == ObjectMobility::Movable)
		{
			mLocalTfrm.rotate(axis, angle);
			notifyTransformChanged(TCF_Transform);
		}
	}

	void SceneObject::rotate(const Quaternion& q)
	{
		if (mMobility == ObjectMobility::Movable)
		{
			mLocalTfrm.rotate(q);
			notifyTransformChanged(TCF_Transform);
		}
	}

	void SceneObject::roll(const Radian& angle)
	{
		if (mMobility == ObjectMobility::Movable)
		{
			mLocalTfrm.roll(angle);
			notifyTransformChanged(TCF_Transform);
		}
	}

	void SceneObject::yaw(const Radian& angle)
	{
		if (mMobility == ObjectMobility::Movable)
		{
			mLocalTfrm.yaw(angle);
			notifyTransformChanged(TCF_Transform);
		}
	}

	void SceneObject::pitch(const Radian& angle)
	{
		if (mMobility == ObjectMobility::Movable)
		{
			mLocalTfrm.pitch(angle);
			notifyTransformChanged(TCF_Transform);
		}
	}

	void SceneObject::setForward(const Vector3& forwardDir)
	{
		const Transform& worldTfrm = getTransform();

		Quaternion currentRotation = worldTfrm.getRotation();
		currentRotation.lookRotation(forwardDir);
		setWorldRotation(currentRotation);
	}

	void SceneObject::updateTransformsIfDirty()
	{
		if (!isCachedLocalTfrmUpToDate())
			updateLocalTfrm();

		if (!isCachedWorldTfrmUpToDate())
			updateWorldTfrm();
	}

	void SceneObject::notifyTransformChanged(TransformChangedFlags flags) const
	{
		// If object is immovable, don't send transform changed events nor mark the transform dirty
		TransformChangedFlags componentFlags = flags;
		if (mMobility != ObjectMobility::Movable)
			componentFlags = (TransformChangedFlags)(componentFlags & ~TCF_Transform);
		else
		{
			mDirtyFlags |= DirtyFlags::LocalTfrmDirty | DirtyFlags::WorldTfrmDirty;
			mDirtyHash++;
		}

		// Only send component flags if we haven't removed them all
		if (componentFlags != 0)
		{
			for (auto& entry : mComponents)
			{
				if (entry->supportsNotify(flags))
				{
					bool alwaysRun = entry->hasFlag(ComponentFlag::AlwaysRun);
					if (alwaysRun || gSceneManager().isRunning())
						entry->onTransformChanged(componentFlags);
				}
			}
		}

		// Mobility flag is only relevant for this scene object
		flags = (TransformChangedFlags)(flags & ~TCF_Mobility);
		if (flags != 0)
		{
			for (auto& entry : mChildren)
				entry->notifyTransformChanged(flags);
		}
	}

	void SceneObject::updateWorldTfrm() const
	{
		mWorldTfrm = mLocalTfrm;

		// Don't allow movement from parent when not movable
		if (mParent != nullptr && mMobility == ObjectMobility::Movable)
		{
			mWorldTfrm.makeWorld(mParent->getTransform());

			mCachedWorldTfrm = mWorldTfrm.getMatrix();
		}
		else
		{
			mCachedWorldTfrm = getLocalMatrix();
		}

		mDirtyFlags &= ~DirtyFlags::WorldTfrmDirty;
	}

	void SceneObject::updateLocalTfrm() const
	{
		mCachedLocalTfrm = mLocalTfrm.getMatrix();
		mDirtyFlags &= ~DirtyFlags::LocalTfrmDirty;
	}

	/************************************************************************/
	/* 								Hierarchy	                     		*/
	/************************************************************************/

	void SceneObject::setParent(const HSceneObject& parent, bool keepWorldTransform)
	{
		if (parent.isDestroyed())
			return;

#if BS_IS_BANSHEE3D
		UUID originalPrefab = getPrefabLink();
#endif

		if (mMobility != ObjectMobility::Movable)
			keepWorldTransform = true;

		_setParent(parent, keepWorldTransform);

#if BS_IS_BANSHEE3D
		if (gCoreApplication().isEditor())
		{
			UUID newPrefab = getPrefabLink();
			if (originalPrefab != newPrefab)
				PrefabUtility::clearPrefabIds(mThisHandle);
		}
#endif
	}

	void SceneObject::_setParent(const HSceneObject& parent, bool keepWorldTransform)
	{
		if (mThisHandle == parent)
			return;

		if (mParent == nullptr || mParent != parent)
		{
			Transform worldTfrm;

			// Make sure the object keeps its world coordinates
			if (keepWorldTransform)
				worldTfrm = getTransform();

			if (mParent != nullptr)
				mParent->removeChild(mThisHandle);

			if (parent != nullptr)
			{
				parent->addChild(mThisHandle);
				setScene(parent->mParentScene);
			}
			else
				setScene(nullptr);

			mParent = parent;

			if (keepWorldTransform)
			{
				mLocalTfrm = worldTfrm;

				if (mParent != nullptr)
					mLocalTfrm.makeLocal(mParent->getTransform());
			}

			bool isInstantiated = (mFlags & SOF_DontInstantiate) == 0;
			if(isInstantiated)
				notifyTransformChanged((TransformChangedFlags)(TCF_Parent | TCF_Transform));
		}
	}

	const SPtr<SceneInstance>& SceneObject::getScene() const
	{
		if(mParentScene)
			return mParentScene;

		BS_LOG(Warning, Scene,
			"Attempting to access a scene of a SceneObject with no scene, returning main scene instead.");
		return gSceneManager().getMainScene();
		
	}

	void SceneObject::setScene(const SPtr<SceneInstance>& scene)
	{
		if(mParentScene == scene)
			return;

		mParentScene = scene;

		for(auto& child : mChildren)
			child->setScene(scene);
	}

	HSceneObject SceneObject::getChild(UINT32 idx) const
	{
		if(idx >= mChildren.size())
		{
			BS_EXCEPT(InternalErrorException, "Child index out of range.");
		}

		return mChildren[idx];
	}

	int SceneObject::indexOfChild(const HSceneObject& child) const
	{
		for(int i = 0; i < (int)mChildren.size(); i++)
		{
			if(mChildren[i] == child)
				return i;
		}

		return -1;
	}

	void SceneObject::addChild(const HSceneObject& object)
	{
		mChildren.push_back(object);

		object->_setFlags(mFlags);
	}

	void SceneObject::removeChild(const HSceneObject& object)
	{
		auto result = find(mChildren.begin(), mChildren.end(), object);

		if(result != mChildren.end())
			mChildren.erase(result);
		else
		{
			BS_EXCEPT(InternalErrorException,
				"Trying to remove a child but it's not a child of the transform.");
		}
	}

	HSceneObject SceneObject::findPath(const String& path) const
	{
		if (path.empty())
			return HSceneObject();
		
		String trimmedPath = path;
		StringUtil::trim(trimmedPath, "/");

		Vector<String> entries = StringUtil::split(trimmedPath, "/");

		// Find scene object referenced by the path
		HSceneObject so = getHandle();
		UINT32 pathIdx = 0;
		for (; pathIdx < (UINT32)entries.size(); pathIdx++)
		{
			String entry = entries[pathIdx];

			if (entry.empty())
				continue;

			// This character signifies not-a-scene-object. This is allowed to support
			// paths used by the scripting system (which can point to properties of
			// components on scene objects).
			if (entry[0] != '!')
				break;

			String childName = entry.substr(1, entry.size() - 1);
			so = so->findChild(childName);

			if (so == nullptr)
				break;
		}

		return so;
	}

	HSceneObject SceneObject::findChild(const String& name, bool recursive)
	{
		for (auto& child : mChildren)
		{
			if (child->getName() == name)
				return child;
		}

		if (recursive)
		{
			for (auto& child : mChildren)
			{
				HSceneObject foundObject = child->findChild(name, true);
				if (foundObject != nullptr)
					return foundObject;
			}
		}

		return HSceneObject();
	}

	Vector<HSceneObject> SceneObject::findChildren(const String& name, bool recursive)
	{
		std::function<void(const HSceneObject&, Vector<HSceneObject>&)> findChildrenInternal =
			[&](const HSceneObject& so, Vector<HSceneObject>& output)
		{
			for (auto& child : so->mChildren)
			{
				if (child->getName() == name)
					output.push_back(child);
			}

			if (recursive)
			{
				for (auto& child : so->mChildren)
					findChildrenInternal(child, output);
			}
		};

		Vector<HSceneObject> output;
		findChildrenInternal(mThisHandle, output);

		return output;
	}

	void SceneObject::setActive(bool active)
	{
		mActiveSelf = active;
		setActiveHierarchy(active);
	}

	void SceneObject::setActiveHierarchy(bool active, bool triggerEvents)
	{
		bool activeHierarchy = active && mActiveSelf;

		if (mActiveHierarchy != activeHierarchy)
		{
			mActiveHierarchy = activeHierarchy;

			if (triggerEvents)
			{
				if (activeHierarchy)
				{
					for (auto& component : mComponents)
						gSceneManager()._notifyComponentActivated(component, triggerEvents);
				}
				else
				{
					for (auto& component : mComponents)
						gSceneManager()._notifyComponentDeactivated(component, triggerEvents);
				}
			}
		}
		
		for (auto child : mChildren)
		{
			child->setActiveHierarchy(mActiveHierarchy, triggerEvents);
		}
	}

	bool SceneObject::getActive(bool self) const
	{
		if (self)
			return mActiveSelf;
		else
			return mActiveHierarchy;
	}

	void SceneObject::setMobility(ObjectMobility mobility)
	{
		if(mMobility != mobility)
		{
			mMobility = mobility;

			// If mobility changed to movable, update both the mobility flag and transform, otherwise just mobility
			if (mMobility == ObjectMobility::Movable)
				notifyTransformChanged((TransformChangedFlags)(TCF_Transform | TCF_Mobility));
			else
				notifyTransformChanged(TCF_Mobility);
		}
	}

	HSceneObject SceneObject::clone(bool instantiate, bool preserveUUIDs)
	{
		const bool isInstantiated = !hasFlag(SOF_DontInstantiate);

		if (!instantiate)
			_setFlags(SOF_DontInstantiate);
		else
			_unsetFlags(SOF_DontInstantiate);

		SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>();
		BinarySerializer serializer;
		serializer.encode(this, stream);

		int flags = GODM_RestoreExternal | GODM_UseNewIds;
		if(!preserveUUIDs)
			flags |= GODM_UseNewUUID;

		CoreSerializationContext serzContext;
		serzContext.goState = bs_shared_ptr_new<GameObjectDeserializationState>(flags);

		stream->seek(0);
		SPtr<SceneObject> cloneObj = std::static_pointer_cast<SceneObject>(
			serializer.decode(stream, (UINT32)stream->size(), BinarySerializerFlag::None, &serzContext));

		if(isInstantiated)
			_unsetFlags(SOF_DontInstantiate);
		else
			_setFlags(SOF_DontInstantiate);

		return cloneObj->mThisHandle;
	}

	HComponent SceneObject::getComponent(RTTITypeBase* type) const
	{
		if(type != Component::getRTTIStatic())
		{
			for (auto& entry : mComponents)
			{
				if (entry->getRTTI()->isDerivedFrom(type))
					return entry;
			}
		}

		return HComponent();
	}

	void SceneObject::destroyComponent(const HComponent component, bool immediate)
	{
		if(component == nullptr)
		{
			BS_LOG(Warning, Scene, "Trying to remove a null component");
			return;
		}

		auto iter = std::find(mComponents.begin(), mComponents.end(), component);

		if(iter != mComponents.end())
		{
			(*iter)->_setIsDestroyed();

			if (isInstantiated())
				gSceneManager()._notifyComponentDestroyed(*iter, immediate);
			
			(*iter)->destroyInternal(*iter, immediate);
			mComponents.erase(iter);
		}
		else
			BS_LOG(Warning, Scene, "Trying to remove a component that doesn't exist on this SceneObject.");
	}

	void SceneObject::destroyComponent(Component* component, bool immediate)
	{
		auto iterFind = std::find_if(mComponents.begin(), mComponents.end(),
			[component](const HComponent& x)
		{
			if(x.isDestroyed())
				return false;

			return x._getHandleData()->mPtr->object.get() == component; }
		);

		if(iterFind != mComponents.end())
		{
			destroyComponent(*iterFind, immediate);
		}
	}

	HComponent SceneObject::addComponent(UINT32 typeId)
	{
		SPtr<IReflectable> newObj = rtti_create(typeId);

		if(!rtti_is_subclass<Component>(newObj.get()))
		{
			BS_LOG(Error, Scene, "Specified type is not a valid Component.");
			return HComponent();
		}

		SPtr<Component> componentPtr = std::static_pointer_cast<Component>(newObj);

		// Clean up the self-reference assigned by the RTTI system
		componentPtr->mRTTIData = nullptr;

		HComponent newComponent = static_object_cast<Component>(GameObjectManager::instance().registerObject(componentPtr));
		newComponent->mParent = mThisHandle;

		addAndInitializeComponent(newComponent);
		return newComponent;
	}

	void SceneObject::addComponentInternal(const SPtr<Component>& component)
	{
		HComponent newComponent = static_object_cast<Component>(
			GameObjectManager::instance().getObject(component->getInstanceId()));
		newComponent->mParent = mThisHandle;
		newComponent->mThisHandle = newComponent;

		mComponents.push_back(newComponent);
	}

	void SceneObject::addAndInitializeComponent(const HComponent& component)
	{
		component->mThisHandle = component;

		if(component->mUUID.empty())
			component->mUUID = UUIDGenerator::generateRandom();

		mComponents.push_back(component);

		if (isInstantiated())
		{
			component->_instantiate();

			gSceneManager()._notifyComponentCreated(component, getActive());
		}
	}

	void SceneObject::addAndInitializeComponent(const SPtr<Component>& component)
	{
		HComponent newComponent = static_object_cast<Component>(
			GameObjectManager::instance().getObject(component->getInstanceId()));
		newComponent->mParent = mThisHandle;

		addAndInitializeComponent(newComponent);
	}

	RTTITypeBase* SceneObject::getRTTIStatic()
	{
		return SceneObjectRTTI::instance();
	}

	RTTITypeBase* SceneObject::getRTTI() const
	{
		return SceneObject::getRTTIStatic();
	}
}
