//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Input
	 *  @{
	 */

	/// <summary>
	/// Event that gets sent out when user interacts with the screen in some way, usually by moving the mouse cursor or using 
	/// touch input.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct PointerEvent
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static PointerEvent Default()
		{
			PointerEvent value = new PointerEvent();
			value.ScreenPos = Vector2I.Default();
			value.Delta = Vector2I.Default();
			value.Button = PointerEventButton.Left;
			value.Type = PointerEventType.CursorMoved;
			value.Shift = false;
			value.Control = false;
			value.Alt = false;
			value.MouseWheelScrollAmount = 0f;
			value.IsUsed = false;

			return value;
		}

		/// <summary>Screen position where the input event occurred.</summary>
		public Vector2I ScreenPos;
		/// <summary>Change in movement since last sent event.</summary>
		public Vector2I Delta;
		/// <summary>
		/// Button that triggered the pointer event. Might be irrelevant depending on event type. (for example move events 
		/// don&apos;t correspond to a button.
		/// </summary>
		public PointerEventButton Button;
		/// <summary>Type of the pointer event.</summary>
		public PointerEventType Type;
		/// <summary>Is shift button on the keyboard being held down.</summary>
		public bool Shift;
		/// <summary>Is control button on the keyboard being held down.</summary>
		public bool Control;
		/// <summary>Is alt button on the keyboard being held down.</summary>
		public bool Alt;
		/// <summary>If mouse wheel is being scrolled, what is the amount. Only relevant for move events.</summary>
		public float MouseWheelScrollAmount;
		/// <summary>This will be set to true if some previous event receiver has marked the event as used.</summary>
		public bool IsUsed;
	}

	/** @} */
}
