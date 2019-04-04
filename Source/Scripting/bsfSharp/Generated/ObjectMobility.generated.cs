//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>
	/// Controls what kind of mobility restrictions a scene object has. This is used primarily as a performance hint to other 
	/// systems. Generally the more restricted the mobility the higher performance can be achieved.
	/// </summary>
	public enum ObjectMobility
	{
		/// <summary>Scene object can be moved and has no mobility restrictions.</summary>
		Movable = 0,
		/// <summary>
		/// Scene object isn&apos;t allowed to be moved but is allowed to be visually changed in other ways (e.g. changing the 
		/// displayed mesh or light intensity (depends on attached components).
		/// </summary>
		Immovable = 1,
		/// <summary>
		/// Scene object isn&apos;t allowed to be moved nor is it allowed to be visually changed. Object must be fully static.
		/// </summary>
		Static = 2
	}
}
