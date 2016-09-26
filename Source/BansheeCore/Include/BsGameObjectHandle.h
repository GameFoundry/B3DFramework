//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

namespace BansheeEngine
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
		GameObjectInstanceData()
			:object(nullptr), mInstanceId(0)
		{ }

		SPtr<GameObject> object;
		UINT64 mInstanceId;
	};

	typedef SPtr<GameObjectInstanceData> GameObjectInstanceDataPtr;

	/**	Internal data shared between GameObject handles. */
	struct BS_CORE_EXPORT GameObjectHandleData
	{
		GameObjectHandleData()
			:mPtr(nullptr)
		{ }

		GameObjectHandleData(const SPtr<GameObjectInstanceData>& ptr)
		{
			mPtr = ptr;
		}

		SPtr<GameObjectInstanceData> mPtr;
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
	class BS_CORE_EXPORT GameObjectHandleBase : public IReflectable
	{
	public:
		GameObjectHandleBase();

		/**
		 * Returns true if the object the handle is pointing to has been destroyed.
		 *
		 * @param[in] checkQueued	Game objects can be queued for destruction but not actually destroyed yet, and still
		 *							accessible. If this is false this method will return true only if the object is 
		 *							completely inaccessible (fully destroyed). If this is true this method will return true
		 *							if object is completely inaccessible or if it is just queued for destruction.
		 */
		bool isDestroyed(bool checkQueued = false) const;

		/**	Returns the instance ID of the object the handle is referencing. */
		UINT64 getInstanceId() const { return mData->mPtr != nullptr ? mData->mPtr->mInstanceId : 0; }

		/**
		 * Returns pointer to the referenced GameObject.
		 *
		 * @note	Throws exception if the GameObject was destroyed.
		 */
		GameObject* get() const 
		{ 
			throwIfDestroyed();

			return mData->mPtr->object.get(); 
		}

		/**
		 * Returns a smart pointer to the referenced GameObject.
		 *
		 * @note	Throws exception if the GameObject was destroyed.
		 */
		SPtr<GameObject> getInternalPtr() const
		{
			throwIfDestroyed();

			return mData->mPtr->object;
		}

		/**
		 * Returns pointer to the referenced GameObject.
		 *
		 * @note	Throws exception if the GameObject was destroyed.
		 */
		GameObject* operator->() const { return get(); }

		/**
		 * Returns reference to the referenced GameObject.
		 *
		 * @note	Throws exception if the GameObject was destroyed.
		 */
		GameObject& operator*() const { return *get(); }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Returns internal handle data. */
		SPtr<GameObjectHandleData> _getHandleData() const { return mData; }

		/** Resolves a handle to a proper GameObject in case it was created uninitialized. */
		void _resolve(const GameObjectHandleBase& object);

		/**	Changes the GameObject instance the handle is pointing to. */
		void _setHandleData(const SPtr<GameObject>& object);

		/** @} */

	protected:
		friend class GameObjectManager;

		template<class _Ty1, class _Ty2>
		friend bool operator==(const GameObjectHandle<_Ty1>& _Left, const GameObjectHandle<_Ty2>& _Right);

		GameObjectHandleBase(const SPtr<GameObject> ptr);
		GameObjectHandleBase(const SPtr<GameObjectHandleData>& data);
		GameObjectHandleBase(std::nullptr_t ptr);

		/**	Throws an exception if the referenced GameObject has been destroyed. */
		void throwIfDestroyed() const;
		
		/**	Invalidates the handle signifying the referenced object was destroyed. */
		void destroy()
		{
			// It's important not to clear mData->mPtr as some code might rely
			// on it. (for example for restoring lost handles)

			if (mData->mPtr != nullptr)
				mData->mPtr->object = nullptr;
		}

		SPtr<GameObjectHandleData> mData;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GameObjectHandleRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
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
			:GameObjectHandleBase()
		{	
			mData = bs_shared_ptr_new<GameObjectHandleData>();
		}

		/**	Copy constructor from another handle of the same type. */
		template <typename T1>
		GameObjectHandle(const GameObjectHandle<T1>& ptr)
			:GameObjectHandleBase()
		{ 	
			mData = ptr._getHandleData();
		}

		/**	Copy constructor from another handle of the base type. */
		GameObjectHandle(const GameObjectHandleBase& ptr)
			:GameObjectHandleBase()
		{ 	
			mData = ptr._getHandleData();
		}

		/**	Invalidates the handle. */
		GameObjectHandle<T>& operator=(std::nullptr_t ptr)
		{ 	
			mData = bs_shared_ptr_new<GameObjectHandleData>();

			return *this;
		}

		/**
		 * Returns a pointer to the referenced GameObject.
		 *
		 * @note	Throws exception if the GameObject was destroyed.
		 */
		T* get() const 
		{ 
			throwIfDestroyed();

			return reinterpret_cast<T*>(mData->mPtr->object.get()); 
		}

		/**
		 * Returns a smart pointer to the referenced GameObject.
		 *
		 * @note	Throws exception if the GameObject was destroyed.
		 */
		SPtr<T> getInternalPtr() const
		{
			throwIfDestroyed();

			return std::static_pointer_cast<T>(mData->mPtr->object);
		}

		/**
		 * Returns pointer to the referenced GameObject.
		 *
		 * @note	Throws exception if the GameObject was destroyed.
		 */
		T* operator->() const { return get(); }

		/**
		 * Returns reference to the referenced GameObject.
		 *
		 * @note	Throws exception if the GameObject was destroyed.
		 */
		T& operator*() const { return *get(); }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		template<class _Ty>
		struct Bool_struct
		{
			int _Member;
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
			return (((mData->mPtr != nullptr) && (mData->mPtr->object != nullptr)) ? &Bool_struct<T>::_Member : 0);
		}

		/** @} */
	};

	/**	Casts one GameObject handle type to another. */
	template<class _Ty1, class _Ty2>
		GameObjectHandle<_Ty1> static_object_cast(const GameObjectHandle<_Ty2>& other)
	{	
		return GameObjectHandle<_Ty1>(other);
	}

	/**	Compares if two handles point to the same GameObject. */
	template<class _Ty1, class _Ty2>
	bool operator==(const GameObjectHandle<_Ty1>& _Left, const GameObjectHandle<_Ty2>& _Right)
	{	
		return (_Left.mData == nullptr && _Right.mData == nullptr) || 
			(_Left.mData != nullptr && _Right.mData != nullptr && _Left.getInstanceId() == _Right.getInstanceId());
	}

	/**	Compares if two handles point to different GameObject%s. */
	template<class _Ty1, class _Ty2>
	bool operator!=(const GameObjectHandle<_Ty1>& _Left, const GameObjectHandle<_Ty2>& _Right)
	{	
		return (!(_Left == _Right));
	}

	/** @} */
}
