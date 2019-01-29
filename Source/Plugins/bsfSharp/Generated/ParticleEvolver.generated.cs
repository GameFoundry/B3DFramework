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

	/// <summary>Updates properties of all active particles in a particle system in some way.</summary>
	[ShowInInspector]
	public partial class ParticleEvolver : ScriptObject
	{
		private ParticleEvolver(bool __dummy0) { }
		protected ParticleEvolver() { }

	}

	/** @} */
}
