//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElementBase.h"
#include "Math/BsVector2I.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**
	 * Base class for layout GUI element. Layout element positions and sizes any child elements according to element styles
	 * and layout options.
	 */
	class B3D_EXPORT GUILayout : public GUIElementBase
	{
	public:
		GUILayout(const GUISizeConstraints& dimensions);
		GUILayout() = default;
		virtual ~GUILayout() = default;

		/**	Creates a new element and adds it to the layout after all existing elements. */
		template <class Type, class... Args>
		Type* AddNewElement(Args&&... args)
		{
			Type* elem = Type::Create(std::forward<Args>(args)...);
			AddElement(elem);
			return elem;
		}

		/**	Creates a new element and inserts it before the element at the specified index. */
		template <class Type, class... Args>
		Type* InsertNewElement(u32 idx, Args&&... args)
		{
			Type* elem = Type::Create(std::forward<Args>(args)...);
			InsertElement(idx, elem);
			return elem;
		}

		/**	Adds a new element to the layout after all existing elements. */
		void AddElement(GUIElementBase* element);

		/**	Removes the specified element from the layout. */
		void RemoveElement(GUIElementBase* element);

		/**	Removes a child element at the specified index. */
		void RemoveElementAt(u32 idx);

		/**	Inserts a GUI element before the element at the specified index. */
		void InsertElement(u32 idx, GUIElementBase* element);

		/** Removes all child elements and destroys them. */
		void Clear();

		/**	Returns number of child elements in the layout. */
		u32 GetNumChildren() const { return (u32)mChildren.size(); }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		GUIConstrainedSize GetConstrainedSize() const override { return GetCachedConstrainedSize(); }

		/** Returns a size range that was cached during the last GUIElementBase::_updateOptimalLayoutSizes call. */
		GUIConstrainedSize GetCachedConstrainedSize() const { return mConstrainedSize; }

		/**
		 * Returns a size ranges for all children that was cached during the last GUIElementBase::_updateOptimalLayoutSizes
		 * call.
		 */
		const Vector<GUIConstrainedSize>& GetChildrenConstrainedSizes() const { return mChildrenConstrainedSizes; }

		Vector2I CalculateUnconstrainedOptimalSize() const override { return mConstrainedSize.Optimal; }
		Type GetType() const override { return GUIElementBase::Type::Layout; }

		/** @} */

	protected:
		Vector<GUIConstrainedSize> mChildrenConstrainedSizes;
		GUIConstrainedSize mConstrainedSize;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUILayoutRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
} // namespace bs
