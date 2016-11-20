//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsModule.h"

namespace bs
{
	/** @addtogroup EditorWindow-Internal
	 *  @{
	 */

	/** Handles opening and closing of a drop down window. Only a single drop down window can be open at a time. */
	class BS_ED_EXPORT DropDownWindowManager : public Module <DropDownWindowManager>
	{
	public:
		DropDownWindowManager();
		~DropDownWindowManager();

		/**
		 * Opens a new drop down window with the specified type and arguments.
		 *
		 * @param[in]	parent		Render window parent in which to open the drop down window.
		 * @param[in]	camera		Camera in which to open the drop down window.
		 * @param[in]	position	Position relative to the viewport at which to open the drop down window.
		 * @param[in]	...args		A set of arguments to be passed along to the drop down window constructor.
		 * @return					An instance of the newly created drop down window.
		 *
		 * @note	This method will automatically close any existing drop down windows before opening a new one.
		 */
		template<class T, class... Args>
		T* open(const SPtr<RenderWindow>& parent, const SPtr<Camera>& camera,
			const Vector2I& position, Args &&...args)
		{
			close();

			mOpenWindow = bs_new<T>(parent, camera, position, std::forward<Args>(args)...);
			return static_cast<T*>(mOpenWindow);
		}

		/**	Closes a drop down window if one is open. */
		void close();

		/** To be called once per frame. Internal method. */
		void update();
	protected:
		DropDownWindow* mOpenWindow;
	};

	/** @} */
}