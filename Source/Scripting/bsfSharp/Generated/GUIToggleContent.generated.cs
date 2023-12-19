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

	/// <summary>Structure describing contents of a GUIToggle element.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct GUIToggleContent
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static GUIToggleContent Default()
		{
			GUIToggleContent value = new GUIToggleContent();
			value.GeneralContent = GUIContent.Default();
			value.ToggleGroup = null;

			return value;
		}

		public GUIToggleContent(GUIContent content, GUIToggleGroup toggleGroup = null)
		{
			this.GeneralContent = content;
			this.ToggleGroup = toggleGroup;
		}

		public GUIContent GeneralContent;
		public GUIToggleGroup ToggleGroup;
	}

	/** @} */
}
