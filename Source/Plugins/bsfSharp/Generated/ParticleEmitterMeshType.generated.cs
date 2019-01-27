//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Determines the emission type for the mesh particle emitter shape.</summary>
	public enum ParticleEmitterMeshType
	{
		/// <summary>Particles will be emitted from mesh vertices.</summary>
		Vertex = 0,
		/// <summary>Particles will be emitted from mesh edges.</summary>
		Edge = 1,
		/// <summary>Particles will be emitted from mesh triangles.</summary>
		Triangle = 2
	}

	/** @} */
}
