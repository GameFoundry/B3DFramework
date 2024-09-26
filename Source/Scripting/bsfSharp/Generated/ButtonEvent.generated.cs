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

	/// <summary>Contains data about a button input event.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ButtonEvent
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ButtonEvent Default()
		{
			ButtonEvent value = new ButtonEvent();
			value.ButtonCode = (ButtonCode)0;
			value.Timestamp = 0;
			value.DeviceIndex = 0;
			value.IsUsed = false;

			return value;
		}

		/// <summary>Button code this event is referring to.</summary>
		public ButtonCode ButtonCode;
		/// <summary>Timestamp in ticks when the event happened.</summary>
		public ulong Timestamp;
		/// <summary>Index of the device that the event originated from.</summary>
		public int DeviceIndex;
		/// <summary>This will be set to true if some previous event receiver has marked the event as used.</summary>
		public bool IsUsed;
	}

	/** @} */
}
