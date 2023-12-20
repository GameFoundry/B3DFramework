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
	public partial struct GUITabButtonContent
	{
		public GUITabButtonContent(int index, GUIContent content, GUIToggleGroup toggleGroup)
		{
			this.Index = index;
			this.GeneralContent = GUIContent.Default();
			this.ToggleGroup = null;
		}

		///<summary>
		/// Returns a subset of this struct. This subset usually contains common fields shared with another struct.
		///</summary>
		public GUIToggleContent GetBase()
		{
			GUIToggleContent value;
			value.GeneralContent = GeneralContent;
			value.ToggleGroup = ToggleGroup;
			return value;
		}

		///<summary>
		/// Assigns values to a subset of fields of this struct. This subset usually contains common field shared with 
		/// another struct.
		///</summary>
		public void SetBase(GUIToggleContent value)
		{
			GeneralContent = value.GeneralContent;
			ToggleGroup = value.ToggleGroup;
		}

		public int Index;
		public GUIContent GeneralContent;
		public GUIToggleGroup ToggleGroup;
	}

	/** @} */
}
