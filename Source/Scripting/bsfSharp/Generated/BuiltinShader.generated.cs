//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Types of builtin shaders that are always available.</summary>
	public enum BuiltinShader
	{
		Custom = 0,
		/// <summary>Physically based shader used for opaque 3D geometry.</summary>
		Standard = 1,
		/// <summary>Physically based shader used for transparent 3D geometry.</summary>
		Transparent = 2,
		/// <summary>Special shader used for rendering particles without any lighting, with support for transparency.</summary>
		ParticlesUnlit = 3,
		/// <summary>
		/// Special shader used for rendering particles with lighting using the forward rendering pipeline (supports  
		/// transparency).
		/// </summary>
		ParticlesLit = 4,
		/// <summary>
		/// Special shader used for rendering particles with lighting using the deferred rendering pipeline (no support for 
		/// transparency).
		/// </summary>
		ParticlesLitOpaque = 5,
		/// <summary>Special shader used for rendering decals that project onto other geometry.</summary>
		Decal = 6
	}
}
