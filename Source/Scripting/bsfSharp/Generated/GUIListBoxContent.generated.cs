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

	/// <summary>Structure describing contents of a GUIListBox element.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct GUIListBoxContent
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static GUIListBoxContent Default()
		{
			GUIListBoxContent value = new GUIListBoxContent();
			value.Elements = null;
			value.AllowMultiselect = false;

			return value;
		}

		public GUIListBoxContent(LocString[] elements, bool allowMultiselect = false)
		{
			this.Elements = elements;
			this.AllowMultiselect = allowMultiselect;
		}

		public LocString[] Elements;
		public bool AllowMultiselect;
	}

	/** @} */
}
