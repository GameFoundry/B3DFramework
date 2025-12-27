//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "Reflection/B3DRTTIType.h"

namespace b3d
{
	/** @addtogroup Implementation
	 *  @{
	 */

	class GameObjectManager;

	template <typename T>
	class GameObjectHandle;

	/**	Contains instance data that is held by all GameObject handles. */
	struct GameObjectInstanceData
	{
		SPtr<GameObject> Object;
	};

	/**	Internal data shared between GameObject handles. */
	struct B3D_EXPORT GameObjectHandleData
	{
		GameObjectHandleData() = default;

		GameObjectHandleData(SPtr<GameObjectInstanceData> instanceData, const UUID& id)
			: InstanceData(std::move(instanceData)), Id(id)
		{}

		SPtr<GameObjectInstanceData> InstanceData;
		UUID Id;
	};

	/**
	 * A handle that can point to various types of game objects. It primarily keeps track if the object is still alive,
	 * so anything still referencing it doesn't accidentally use it.
	 *
	 * @note
	 * This class exists because references between game objects should be quite loose. For example one game object should
	 * be able to reference another one without the other one knowing. But if that is the case I also need to handle the
	 * case when the other object we're referencing has been deleted, and that is the main purpose of this class.
	 */
	class B3D_EXPORT GameObjectHandleBase : public IReflectable
	{
	public:
		GameObjectHandleBase()
			: mSharedHandleData(B3DMakeShared<GameObjectHandleData>(nullptr, UUID::kEmpty))
		{}

		/**
		 * Returns true if the object the handle is pointing to has been destroyed.
		 *
		 * @param[in] checkQueued	Game objects can be queued for destruction but not actually destroyed yet, and still
		 *							accessible. If this is false this method will return true only if the object is
		 *							completely inaccessible (fully destroyed). If this is true this method will return true
		 *							if object is completely inaccessible or if it is just queued for destruction.
		 */
		bool IsDestroyed(bool checkQueued = false) const;

		/** Returns true if the handle points to a non-null object and the object is not queued for destruction. */
		bool IsValid() const { return !IsDestroyed(true);}

		/** Returns the globally unique ID of the object the handle is referencing. */
		const UUID& GetId() const { return mSharedHandleData->Id; }

		/**
		 * Returns pointer to the referenced GameObject.
		 *
		 * @note	Throws exception if the GameObject was destroyed.
		 */
		GameObject* Get() const
		{
			if(!B3D_ENSURE(!IsDestroyed()))
				return nullptr;

			return mSharedHandleData->InstanceData->Object.get();
		}

		/**
		 * Returns the shared pointer to the referenced GameObject.
		 *
		 * @note	Throws exception if the GameObject was destroyed.
		 */
		SPtr<GameObject> GetShared() const
		{
			if(!B3D_ENSURE(!IsDestroyed()))
				return nullptr;

			return mSharedHandleData->InstanceData->Object;
		}

		/** Returns pointer to the referenced GameObject. */
		GameObject* operator->() const { return Get(); }

		/** Returns reference to the referenced GameObject. */
		GameObject& operator*() const { return *Get(); }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Returns internal handle data. */
		const SPtr<GameObjectHandleData>& GetSharedHandleData() const { return mSharedHandleData; }

		/** Clears the handle so it doesn't point to any object. Note this will affect any other handles sharing the handle data. */
		void ClearObjectInstanceData()
		{
			B3D_ASSERT(mSharedHandleData != nullptr);

			mSharedHandleData->InstanceData = nullptr;
			mSharedHandleData->Id = UUID::kEmpty;
		}

		/** Updates the handle so it points to the provided object. Note this will affect any other handles sharing the handle data. */
		void SetObjectInstanceData(const SPtr<GameObject>& object);

		/**
		 * Updates the handle so it points to the same object as the provided object. Compared to the other overload of this method,
		 * this one has the advantage that its able to handle objects have been destroyed. The handle data will remain to be
		 * valid in case the object is later resurrected.
		 *
		 * Note this will affect any other handles sharing the handle data.
		 */
		void SetObjectInstanceData(const GameObjectHandleBase& other)
		{
			B3D_ASSERT(mSharedHandleData != nullptr);
			B3D_ASSERT(other.mSharedHandleData != nullptr);

			mSharedHandleData->InstanceData = other.mSharedHandleData->InstanceData;
			mSharedHandleData->Id = other.mSharedHandleData->Id;
		}

		/** @} */

	protected:
		friend class GameObjectManager;
		friend class GameObjectCollection;

		template <class _Ty1, class _Ty2>
		friend bool operator==(const GameObjectHandle<_Ty1>& lhs, const GameObjectHandle<_Ty2>& rhs);

		GameObjectHandleBase(const SPtr<GameObject>& object);

		GameObjectHandleBase(SPtr<GameObjectHandleData> sharedHandleData)
			: mSharedHandleData(std::move(sharedHandleData))
		{}

		GameObjectHandleBase(std::nullptr_t)
			: mSharedHandleData(B3DMakeShared<GameObjectHandleData>(nullptr, UUID::kEmpty))
		{}

		/** Data shared between a set of handles pointing the referenced object. */
		SPtr<GameObjectHandleData> mSharedHandleData;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GameObjectHandleRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	/** @addtogroup Scene
	 *  @{
	 */

	/**
	 * @copydoc	GameObjectHandleBase
	 *
	 * @note	It is important this class contains no data since we often value cast it to its base.
	 */
	template <typename T>
	class GameObjectHandle : public GameObjectHandleBase
	{
	public:
		/**	Constructs a new empty handle. */
		GameObjectHandle()
		{
			mSharedHandleData = B3DMakeShared<GameObjectHandleData>();
		}

		/**	Constructs a new empty handle. */
		GameObjectHandle(std::nullptr_t)
		{
			mSharedHandleData = B3DMakeShared<GameObjectHandleData>();
		}

		/**	Copy constructor from another handle of the same type. */
		GameObjectHandle(const GameObjectHandle<T>& other) = default;

		/**	Move constructor from another handle of the same type. */
		GameObjectHandle(GameObjectHandle<T>&& other) = default;

		/** Casting of derived type to base. */
		template <typename DerivedType, typename = std::enable_if_t<std::is_base_of_v<T, DerivedType>>>
		GameObjectHandle(const GameObjectHandle<DerivedType>& other)
			: GameObjectHandle(other.GetSharedHandleData())
		{ }

		/** Casting of derived type to base. */
		template <typename DerivedType, typename = std::enable_if_t<std::is_base_of_v<T, DerivedType>>>
		GameObjectHandle(GameObjectHandle<DerivedType>&& other)
			: GameObjectHandle(std::move(other.GetSharedHandleData()))
		{ }

		/**	Invalidates the handle. */
		GameObjectHandle<T>& operator=(std::nullptr_t)
		{
			mSharedHandleData = B3DMakeShared<GameObjectHandleData>();

			return *this;
		}

		/** Copy assignment */
		GameObjectHandle<T>& operator=(const GameObjectHandle<T>& other) = default;

		/** Move assignment */
		GameObjectHandle<T>& operator=(GameObjectHandle<T>&& other) = default;

		/** Returns a pointer to the referenced GameObject. */
		T* Get() const
		{
			if(!B3D_ENSURE(!IsDestroyed()))
				return nullptr;

			return reinterpret_cast<T*>(mSharedHandleData->InstanceData->Object.get());
		}

		/** Returns a smart pointer to the referenced GameObject. */
		SPtr<T> GetShared() const
		{
			if(!B3D_ENSURE(!IsDestroyed()))
				return nullptr;

			return std::static_pointer_cast<T>(mSharedHandleData->InstanceData->Object);
		}

		/** Attempts to cast the handle to another type. Returns nullptr if cast is not valid. */
		template<class CastType>
		GameObjectHandle<CastType> As() const
		{
			RTTIType* const CastTypeRTTI = CastType::GetRttiStatic();
			RTTIType* const MyTypeRTTI = T::GetRttiStatic();

			if(CastTypeRTTI->IsDerivedFrom(MyTypeRTTI))
				return GameObjectHandle<CastType>(GetSharedHandleData());

			return nullptr;
		}

		/** Returns pointer to the referenced GameObject.  */
		T* operator->() const { return Get(); }

		/** Returns reference to the referenced GameObject. */
		T& operator*() const { return *Get(); }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		template <class _Ty>
		struct Bool_struct
		{
			int Member;
		};

		/**
		 * Allows direct conversion of handle to bool.
		 *
		 * @note
		 * This is needed because we can't directly convert to bool since then we can assign pointer to bool and that's
		 * weird.
		 */
		operator int Bool_struct<T>::*() const
		{
			return (((mSharedHandleData->InstanceData != nullptr) && (mSharedHandleData->InstanceData->Object != nullptr)) ? &Bool_struct<T>::Member : 0);
		}

		/** @} */

	protected:
		template <class _Ty1, class _Ty2>
		friend GameObjectHandle<_Ty1> B3DStaticGameObjectCast(const GameObjectHandle<_Ty2>& other);

		template <class _Ty1>
		friend GameObjectHandle<_Ty1> B3DStaticGameObjectCast(const GameObjectHandleBase& other);

		GameObjectHandle(SPtr<GameObjectHandleData> data)
			: GameObjectHandleBase(std::move(data))
		{}
	};

	/**	Casts one GameObject handle type to another. */
	template <class _Ty1, class _Ty2>
	GameObjectHandle<_Ty1> B3DStaticGameObjectCast(const GameObjectHandle<_Ty2>& other)
	{
		return GameObjectHandle<_Ty1>(other.GetSharedHandleData());
	}

	/**	Casts a generic GameObject handle to a specific one . */
	template <class T>
	GameObjectHandle<T> B3DStaticGameObjectCast(const GameObjectHandleBase& other)
	{
		return GameObjectHandle<T>(other.GetSharedHandleData());
	}

	/**	Compares if two handles point to the same GameObject. */
	template <class _Ty1, class _Ty2>
	bool operator==(const GameObjectHandle<_Ty1>& lhs, const GameObjectHandle<_Ty2>& rhs)
	{
		if(lhs.GetId() != rhs.GetId())
			return false;

		if(lhs == nullptr && rhs == nullptr)
			return true;

		if((lhs != nullptr && rhs == nullptr) || (lhs == nullptr && rhs != nullptr))
			return false;

		const SPtr<GameObjectCollection>& lhsCollection = lhs->GetOwnerCollection().lock();
		const SPtr<GameObjectCollection>& rhsCollection = rhs->GetOwnerCollection().lock();

		return lhsCollection == rhsCollection;
	}

	/**	Compares if two handles point to different GameObject%s. */
	template <class _Ty1, class _Ty2>
	bool operator!=(const GameObjectHandle<_Ty1>& lhs, const GameObjectHandle<_Ty2>& rhs)
	{
		return (!(lhs == rhs));
	}

	/** @} */
} // namespace b3d
