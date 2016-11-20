//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSceneObject.h"
#include "BsComponent.h"
#include "BsCoreSceneManager.h"
#include "BsException.h"
#include "BsDebug.h"
#include "BsSceneObjectRTTI.h"
#include "BsMemorySerializer.h"
#include "BsGameObjectManager.h"
#include "BsPrefabUtility.h"
#include "BsMatrix3.h"
#include "BsCoreApplication.h"

namespace bs
{
	SceneObject::SceneObject(const String& name, UINT32 flags)
		: GameObject(), mPrefabHash(0), mFlags(flags), mPosition(Vector3::ZERO), mRotation(Quaternion::IDENTITY)
		, mScale(Vector3::ONE), mWorldPosition(Vector3::ZERO), mWorldRotation(Quaternion::IDENTITY)
		, mWorldScale(Vector3::ONE), mCachedLocalTfrm(Matrix4::IDENTITY), mCachedWorldTfrm(Matrix4::IDENTITY)
		, mDirtyFlags(0xFFFFFFFF), mDirtyHash(0), mActiveSelf(true), mActiveHierarchy(true)
	{
		setName(name);
	}

	SceneObject::~SceneObject()
	{
		if(!mThisHandle.isDestroyed())
		{
			LOGWRN("Object is being deleted without being destroyed first? " + mName);
			destroyInternal(mThisHandle, true);
		}
	}

	HSceneObject SceneObject::create(const String& name, UINT32 flags)
	{
		HSceneObject newObject = createInternal(name, flags);

		if (newObject->isInstantiated())
			gCoreSceneManager().registerNewSO(newObject);

		return newObject;
	}

	HSceneObject SceneObject::createInternal(const String& name, UINT32 flags)
	{
		SPtr<SceneObject> sceneObjectPtr = SPtr<SceneObject>(new (bs_alloc<SceneObject>()) SceneObject(name, flags),
			&bs_delete<SceneObject>, StdAlloc<SceneObject>());
		
		HSceneObject sceneObject = GameObjectManager::instance().registerObject(sceneObjectPtr);
		sceneObject->mThisHandle = sceneObject;

		return sceneObject;
	}

	HSceneObject SceneObject::createInternal(const SPtr<SceneObject>& soPtr, UINT64 originalId)
	{
		HSceneObject sceneObject = GameObjectManager::instance().registerObject(soPtr, originalId);
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
				{
					if (getActive())
						component->onDisabled();

					component->onDestroyed();
				}

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

	String SceneObject::getPrefabLink(bool onlyDirect) const
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

		return "";
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
			rootObj->mPrefabLinkUUID = "";
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
				gCoreSceneManager().registerNewSO(obj->mThisHandle);

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
			{
				component->onInitialized();

				if (obj->getActive())
					component->onEnabled();
			}

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
		mPosition = position;
		notifyTransformChanged(TCF_Transform);
	}

	void SceneObject::setRotation(const Quaternion& rotation)
	{
		mRotation = rotation;
		notifyTransformChanged(TCF_Transform);
	}

	void SceneObject::setScale(const Vector3& scale)
	{
		mScale = scale;
		notifyTransformChanged(TCF_Transform);
	}

	void SceneObject::setWorldPosition(const Vector3& position)
	{
		if (mParent != nullptr)
		{
			Vector3 invScale = mParent->getWorldScale();
			if (invScale.x != 0) invScale.x = 1.0f / invScale.x;
			if (invScale.y != 0) invScale.y = 1.0f / invScale.y;
			if (invScale.z != 0) invScale.z = 1.0f / invScale.z;

			Quaternion invRotation = mParent->getWorldRotation().inverse();

			mPosition = invRotation.rotate(position - mParent->getWorldPosition()) *  invScale;
		}
		else
			mPosition = position;

		notifyTransformChanged(TCF_Transform);
	}

	void SceneObject::setWorldRotation(const Quaternion& rotation)
	{
		if (mParent != nullptr)
		{
			Quaternion invRotation = mParent->getWorldRotation().inverse();

			mRotation = invRotation * rotation;
		}
		else
			mRotation = rotation;

		notifyTransformChanged(TCF_Transform);
	}

	void SceneObject::setWorldScale(const Vector3& scale)
	{
		if (mParent != nullptr)
		{
			Matrix3 rotScale;
			mParent->getWorldTfrm().extract3x3Matrix(rotScale);
			rotScale.inverse();

			Matrix3 scaleMat = Matrix3(Quaternion::IDENTITY, scale);
			scaleMat = rotScale * scaleMat;

			Quaternion rotation;
			Vector3 localScale;
			scaleMat.decomposition(rotation, localScale);

			mScale = localScale;
		}
		else
			mScale = scale;

		notifyTransformChanged(TCF_Transform);
	}

	const Vector3& SceneObject::getWorldPosition() const
	{ 
		if (!isCachedWorldTfrmUpToDate())
			updateWorldTfrm();

		return mWorldPosition; 
	}

	const Quaternion& SceneObject::getWorldRotation() const 
	{ 
		if (!isCachedWorldTfrmUpToDate())
			updateWorldTfrm();

		return mWorldRotation; 
	}

	const Vector3& SceneObject::getWorldScale() const 
	{ 
		if (!isCachedWorldTfrmUpToDate())
			updateWorldTfrm();

		return mWorldScale; 
	}

	void SceneObject::lookAt(const Vector3& location, const Vector3& up)
	{
		Vector3 forward = location - getWorldPosition();
		
		Quaternion rotation = getWorldRotation();
		rotation.lookRotation(forward, up);
		setWorldRotation(rotation);
	}

	const Matrix4& SceneObject::getWorldTfrm() const
	{
		if (!isCachedWorldTfrmUpToDate())
			updateWorldTfrm();

		return mCachedWorldTfrm;
	}

	Matrix4 SceneObject::getInvWorldTfrm() const
	{
		if (!isCachedWorldTfrmUpToDate())
			updateWorldTfrm();

		Matrix4 worldToLocal = Matrix4::inverseTRS(mWorldPosition, mWorldRotation, mWorldScale);
		return worldToLocal;
	}

	const Matrix4& SceneObject::getLocalTfrm() const
	{
		if (!isCachedLocalTfrmUpToDate())
			updateLocalTfrm();

		return mCachedLocalTfrm;
	}

	void SceneObject::move(const Vector3& vec)
	{
		setPosition(mPosition + vec);
	}

	void SceneObject::moveRelative(const Vector3& vec)
	{
		// Transform the axes of the relative vector by camera's local axes
		Vector3 trans = mRotation.rotate(vec);

		setPosition(mPosition + trans);
	}

	void SceneObject::rotate(const Vector3& axis, const Radian& angle)
	{
		Quaternion q;
		q.fromAxisAngle(axis, angle);
		rotate(q);
	}

	void SceneObject::rotate(const Quaternion& q)
	{
		// Note the order of the mult, i.e. q comes after

		// Normalize the quat to avoid cumulative problems with precision
		Quaternion qnorm = q;
		qnorm.normalize();
		setRotation(qnorm * mRotation);
	}

	void SceneObject::roll(const Radian& angle)
	{
		// Rotate around local Z axis
		Vector3 zAxis = mRotation.rotate(Vector3::UNIT_Z);
		rotate(zAxis, angle);
	}

	void SceneObject::yaw(const Radian& angle)
	{
		Vector3 yAxis = mRotation.rotate(Vector3::UNIT_Y);
		rotate(yAxis, angle);
	}

	void SceneObject::pitch(const Radian& angle)
	{
		// Rotate around local X axis
		Vector3 xAxis = mRotation.rotate(Vector3::UNIT_X);
		rotate(xAxis, angle);
	}

	void SceneObject::setForward(const Vector3& forwardDir)
	{
		Quaternion currentRotation = getWorldRotation();
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
		mDirtyFlags |= DirtyFlags::LocalTfrmDirty | DirtyFlags::WorldTfrmDirty;
		mDirtyHash++;

		for(auto& entry : mComponents)
		{
			if (entry->supportsNotify(flags))
				entry->onTransformChanged(flags);
		}

		for (auto& entry : mChildren)
			entry->notifyTransformChanged(flags);
	}

	void SceneObject::updateWorldTfrm() const
	{
		if(mParent != nullptr)
		{
			// Update orientation
			const Quaternion& parentOrientation = mParent->getWorldRotation();
			mWorldRotation = parentOrientation * mRotation;

			// Update scale
			const Vector3& parentScale = mParent->getWorldScale();
			// Scale own position by parent scale, just combine
			// as equivalent axes, no shearing
			mWorldScale = parentScale * mScale;

			// Change position vector based on parent's orientation & scale
			mWorldPosition = parentOrientation.rotate(parentScale * mPosition);

			// Add altered position vector to parents
			mWorldPosition += mParent->getWorldPosition();

			mCachedWorldTfrm.setTRS(mWorldPosition, mWorldRotation, mWorldScale);
		}
		else
		{
			mWorldRotation = mRotation;
			mWorldPosition = mPosition;
			mWorldScale = mScale;

			mCachedWorldTfrm = getLocalTfrm();
		}

		mDirtyFlags &= ~DirtyFlags::WorldTfrmDirty;
	}

	void SceneObject::updateLocalTfrm() const
	{
		mCachedLocalTfrm.setTRS(mPosition, mRotation, mScale);

		mDirtyFlags &= ~DirtyFlags::LocalTfrmDirty;
	}

	/************************************************************************/
	/* 								Hierarchy	                     		*/
	/************************************************************************/

	void SceneObject::setParent(const HSceneObject& parent, bool keepWorldTransform)
	{
		if (parent.isDestroyed())
			return;

#if BS_EDITOR_BUILD
		String originalPrefab = getPrefabLink();
#endif

		_setParent(parent, keepWorldTransform);

#if BS_EDITOR_BUILD
		if (gCoreApplication().isEditor())
		{
			String newPrefab = getPrefabLink();
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
			Vector3 worldPos;
			Quaternion worldRot;
			Vector3 worldScale;

			if (keepWorldTransform)
			{
				// Make sure the object keeps its world coordinates
				worldPos = getWorldPosition();
				worldRot = getWorldRotation();
				worldScale = getWorldScale();
			}

			if (mParent != nullptr)
				mParent->removeChild(mThisHandle);

			if (parent != nullptr)
				parent->addChild(mThisHandle);

			mParent = parent;

			if (keepWorldTransform)
			{
				setWorldPosition(worldPos);
				setWorldRotation(worldRot);
				setWorldScale(worldScale);
			}

			notifyTransformChanged((TransformChangedFlags)(TCF_Parent | TCF_Transform));
		}
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
						component->onEnabled();
				}
				else
				{
					for (auto& component : mComponents)
						component->onDisabled();
				}
			}
		}
		
		for (auto child : mChildren)
		{
			child->setActiveHierarchy(mActiveHierarchy, triggerEvents);
		}
	}

	bool SceneObject::getActive(bool self)
	{
		if (self)
			return mActiveSelf;
		else
			return mActiveHierarchy;
	}

	HSceneObject SceneObject::clone(bool instantiate)
	{
		bool isInstantiated = !hasFlag(SOF_DontInstantiate);

		if (!instantiate)
			_setFlags(SOF_DontInstantiate);
		else
			_unsetFlags(SOF_DontInstantiate);

		UINT32 bufferSize = 0;

		MemorySerializer serializer;
		UINT8* buffer = serializer.encode(this, bufferSize, (void*(*)(UINT32))&bs_alloc);

		GameObjectManager::instance().setDeserializationMode(GODM_UseNewIds | GODM_RestoreExternal);
		SPtr<SceneObject> cloneObj = std::static_pointer_cast<SceneObject>(serializer.decode(buffer, bufferSize));
		bs_free(buffer);

		if(isInstantiated)
			_unsetFlags(SOF_DontInstantiate);
		else
			_setFlags(SOF_DontInstantiate);

		return cloneObj->mThisHandle;
	}

	HComponent SceneObject::getComponent(RTTITypeBase* type) const
	{
		for(auto& entry : mComponents)
		{
			if(entry->getRTTI()->isDerivedFrom(type))
				return entry;
		}

		return HComponent();
	}

	void SceneObject::destroyComponent(const HComponent component, bool immediate)
	{
		if(component == nullptr)
		{
			LOGDBG("Trying to remove a null component");
			return;
		}

		auto iter = std::find(mComponents.begin(), mComponents.end(), component);

		if(iter != mComponents.end())
		{
			(*iter)->_setIsDestroyed();

			if (isInstantiated())
			{
				if (getActive())
					component->onDisabled();

				(*iter)->onDestroyed();
			}
			
			(*iter)->destroyInternal(*iter, immediate);
			mComponents.erase(iter);
		}
		else
			LOGDBG("Trying to remove a component that doesn't exist on this SceneObject.");
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

	void SceneObject::addComponentInternal(const SPtr<Component> component)
	{
		GameObjectHandle<Component> newComponent = GameObjectManager::instance().getObject(component->getInstanceId());
		newComponent->mParent = mThisHandle;
		newComponent->mThisHandle = newComponent;

		mComponents.push_back(newComponent);
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