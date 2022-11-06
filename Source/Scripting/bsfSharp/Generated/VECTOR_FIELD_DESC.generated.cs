//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Descriptor structure used for initialization of a VectorField.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct VectorFieldOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static VectorFieldOptions Default()
		{
			VectorFieldOptions value = new VectorFieldOptions();
			value.CountX = 1;
			value.CountY = 1;
			value.CountZ = 1;
			value.Bounds = AABox.Default();

			return value;
		}

		/// <summary>Number of entries in the vector field along the X axis.</summary>
		public int CountX;
		/// <summary>Number of entries in the vector field along the Y axis.</summary>
		public int CountY;
		/// <summary>Number of entries in the vector field along the Z axis.</summary>
		public int CountZ;
		/// <summary>Spatial bounds of the vector field.</summary>
		public AABox Bounds;
	}

	/** @} */
}
