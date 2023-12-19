//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/// <summary>
	/// Object that allows you to group multiple GUI toggle buttons. Only one button among the grouped ones can be active.
	/// </summary>
	[ShowInInspector]
	public partial class GUIToggleGroup : ScriptObject
	{
		private GUIToggleGroup(bool __dummy0, bool __dummy1) { }
		protected GUIToggleGroup() { }

		/// <summary>
		/// Creates a toggle group that you may provide to GUIToggle upon construction. Toggles sharing the same group will only 
		/// have a single element active at a time.
		/// </summary>
		/// <param name="allowAllOff">
		/// If true all of the toggle buttons can be turned off, if false one will always be turned on.
		/// </param>
		public GUIToggleGroup(bool allowAllOff = false)
		{
			Internal_CreateToggleGroup(this, allowAllOff);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_CreateToggleGroup(GUIToggleGroup managedInstance, bool allowAllOff);
	}

	/** @} */
}
