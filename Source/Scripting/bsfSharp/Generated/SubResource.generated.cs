//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
#if !IS_B3D
	/** @addtogroup Importer
	 *  @{
	 */

	/// <summary>
	/// Contains a resource that was imported from a file that contains multiple resources (for example an animation from an 
	/// FBX file).
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct SubResource
	{
		/// <summary>Unique name of the sub-resource.</summary>
		public string Name;
		/// <summary>Contents of the sub-resource.</summary>
		public Resource Value;
	}

	/** @} */
#endif
}
