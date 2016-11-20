//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsIReflectable.h"
#include "BsRect2I.h"

namespace bs
{
	/** @addtogroup EditorWindow-Internal
	 *  @{
	 */

	/**
	 * Used for storing a layout of widgets in a dock manager. 
	 *
	 * @see		DockManager
	 */
	class DockManagerLayout : public IReflectable
	{
	public:
		/**
		 * A single entry in the dock layout which may contain references to two other entries (non-leaf) or may contain
		 * multiple widgets (leaf).
		 */
		struct Entry
		{
		public:
			Entry();
			~Entry();

			/**
			 * Creates a new leaf entry with the specified widgets.
			 *
			 * @param[in]	parent		Parent of this leaf entry. Can be null for root.
			 * @param[in]	childIdx	Index of this entry in the parents child list. Can be 0 or 1.
			 * @param[in]	widgetNames	A list of all widgets opened in this entry, listed by name.
			 */
			static Entry* createLeaf(Entry* parent, UINT32 childIdx, 
				const Vector<String>& widgetNames);

			/**
			 * Creates a new container entry with the specified split data.
			 *
			 * @param[in]	parent				Parent of this leaf entry. Can be null for root.
			 * @param[in]	childIdx			Index of this entry in the parents child list. Can be 0 or 1.
			 * @param[in]	splitPosition		Determines at what position(in percent) should this container be split. 
			 * @param[in]	horizontalSplit		Whether the split is horizontal (true) or vertical (false).
			 */
			static Entry* createContainer(Entry* parent, UINT32 childIdx, float splitPosition, 
				bool horizontalSplit);

			Vector<String> widgetNames;
			bool isLeaf;
			float splitPosition;
			bool horizontalSplit;

			Entry* children[2];
			Entry* parent;
		};

	public:
		DockManagerLayout();
		~DockManagerLayout();

		/**	Returns the root entry in the saved dock manager hierarchy. */
		Entry& getRootEntry() { return mRootEntry; }

		/**
		 * Signals whether there is a maximized dock container in the layout.
		 *
		 * @param[in]	maximized 	True if maximized.
		 * @param[in]	widgetNames	Names of the widgets on the maximized container.
		 */
		void setIsMaximized(bool maximized, const Vector<String>& widgetNames);

		/**	Check if the layout has a maximized container. */
		bool isMaximized() const { return mIsMaximized; }

		/**	Returns widget names that are in the maximized container, if there is one. */
		const Vector<String>& getMaximizedWidgetNames() const { return mMaximizedWidgetNames; }

		/**
		 * Removes widgets that can no longer be found (their names no longer reference a widget), and removes containers
		 * with no widgets.
		 */
		void pruneInvalidLeaves();

		/**	Makes a deep copy of this object. */
		SPtr<DockManagerLayout> clone();

	private:
		Entry mRootEntry;
		bool mIsMaximized;
		Vector<String> mMaximizedWidgetNames;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class DockManagerLayoutRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;	
	};

	/** @} */
}