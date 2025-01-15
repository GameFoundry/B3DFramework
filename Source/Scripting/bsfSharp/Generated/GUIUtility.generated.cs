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

	/// <summary>Helper class that performs various operations related to GUI.</summary>
	[ShowInInspector]
	public partial class GUIUtility : ScriptObject
	{
		private GUIUtility(bool __dummy0) { }
		protected GUIUtility() { }

		/// <summary>
		/// Calculates optimal content size for the provided text using the provided font and size. Size is calculated without 
		/// word wrap.
		/// </summary>
		/// <param name="text">Text to calculate the size for.</param>
		/// <param name="font">Font to use for rendering the text.</param>
		/// <param name="fontSize">Size of individual characters in the font, in points.</param>
		/// <returns>Width/height required to display the text, in pixels.</returns>
		public static Vector2I CalculateTextBounds(string text, RRef<Font> font, float fontSize)
		{
			Vector2I temp;
			Internal_CalculateTextBounds(text, font, fontSize, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_CalculateTextBounds(string text, RRef<Font> font, float fontSize, out Vector2I __output);
	}

	/** @} */
}
