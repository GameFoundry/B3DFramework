//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
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
			value.countX = 1;
			value.countY = 1;
			value.countZ = 1;
			value.bounds = new AABox();

			return value;
		}

		/// <summary>Number of entries in the vector field along the X axis.</summary>
		public uint countX;
		/// <summary>Number of entries in the vector field along the Y axis.</summary>
		public uint countY;
		/// <summary>Number of entries in the vector field along the Z axis.</summary>
		public uint countZ;
		/// <summary>Spatial bounds of the vector field.</summary>
		public AABox bounds;
	}

	/** @} */
}
