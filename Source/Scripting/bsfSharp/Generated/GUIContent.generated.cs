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
	/// Holds data used for displaying content in a GUIElement. Content can consist of a string, image, a tooltip or none of 
	/// those.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct GUIContent
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static GUIContent Default()
		{
			GUIContent value = new GUIContent();
			value.Text = null;
			value.Images = GUIContentImages.Default();
			value.Tooltip = null;

			return value;
		}

		/// <summary>Constructs content with just a string.</summary>
		public GUIContent(LocString text)
		{
			this.Text = text;
			this.Images = GUIContentImages.Default();
			this.Tooltip = null;
		}

		/// <summary>Constructs content with a string and a tooltip.</summary>
		public GUIContent(LocString text, LocString tooltip)
		{
			this.Text = text;
			this.Images = GUIContentImages.Default();
			this.Tooltip = tooltip;
		}

		/// <summary>Constructs content with just an image.</summary>
		public GUIContent(GUIContentImages image)
		{
			this.Text = null;
			this.Images = image;
			this.Tooltip = null;
		}

		/// <summary>Constructs content with an image and a tooltip.</summary>
		public GUIContent(GUIContentImages image, LocString tooltip)
		{
			this.Text = null;
			this.Images = image;
			this.Tooltip = tooltip;
		}

		/// <summary>Constructs content with a string and an image.</summary>
		public GUIContent(LocString text, GUIContentImages image)
		{
			this.Text = text;
			this.Images = image;
			this.Tooltip = null;
		}

		/// <summary>Constructs content with a string, an image and a tooltip.</summary>
		public GUIContent(LocString text, GUIContentImages image, LocString tooltip)
		{
			this.Text = text;
			this.Images = image;
			this.Tooltip = tooltip;
		}

		public LocString Text;
		public GUIContentImages Images;
		public LocString Tooltip;
	}

	/** @} */
}
