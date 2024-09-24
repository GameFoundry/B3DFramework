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

	/// <summary>Information used for initializing a virtual axis.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct VirtualAxisCreateInformation
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static VirtualAxisCreateInformation Default()
		{
			VirtualAxisCreateInformation value = new VirtualAxisCreateInformation();
			value.Type = 0;
			value.DeadZone = 9.99999974E-5f;
			value.Sensitivity = 1f;
			value.Invert = false;
			value.Normalize = false;

			return value;
		}

		/// <summary>
		/// Type of physical axis to map to. See InputAxis type for common types, but you are not limited to those values.
		/// </summary>
		public int Type;
		/// <summary>Value below which to ignore axis value and consider it 0.</summary>
		public float DeadZone;
		/// <summary>Higher sensitivity means the axis will more easily reach its maximum values.</summary>
		public float Sensitivity;
		/// <summary>Should the axis be inverted.</summary>
		public bool Invert;
		/// <summary>
		/// If enabled, axis values will be normalized to [-1, 1] range. Most axes already come in normalized form and this value 
		/// will not affect such axes. Some axes, like mouse movement are not normalized by default and will instead report 
		/// relative movement. By enabling this you will normalize such axes to [-1, 1] range.
		/// </summary>
		public bool Normalize;
	}

	/** @} */
}
