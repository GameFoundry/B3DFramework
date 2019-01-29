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
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>Physics joint that will maintain a fixed distance and orientation between its two attached bodies.</summary>
	[ShowInInspector]
	public partial class FixedJoint : Joint
	{
		private FixedJoint(bool __dummy0) { }
		protected FixedJoint() { }

	}

	/** @} */
}
