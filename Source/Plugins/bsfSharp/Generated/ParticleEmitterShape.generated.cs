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

	/// <summary>
	/// Base class from all emitter shapes. Emitter shapes determine the position and direction of newly created particles.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleEmitterShape : ScriptObject
	{
		private ParticleEmitterShape(bool __dummy0) { }
		protected ParticleEmitterShape() { }

	}

	/** @} */
}
