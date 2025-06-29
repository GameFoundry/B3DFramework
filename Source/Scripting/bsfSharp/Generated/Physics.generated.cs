//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>Provides global physics settings, factory methods for physics objects and scene queries.</summary>
	[ShowInInspector]
	public partial class Physics : ScriptObject
	{
		private Physics(bool __dummy0) { }
		protected Physics() { }

		/// <summary>
		/// Enables or disables collision between two layers. Each physics object can be assigned a specific layer, and here you 
		/// can determine which layers can interact with each other.
		/// </summary>
		public static void ToggleCollision(ulong groupA, ulong groupB, bool enabled)
		{
			Internal_ToggleCollision(groupA, groupB, enabled);
		}

		/// <summary>Checks if two collision layers are allowed to interact.</summary>
		public static bool IsCollisionEnabled(ulong groupA, ulong groupB)
		{
			return Internal_IsCollisionEnabled(groupA, groupB);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ToggleCollision(ulong groupA, ulong groupB, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsCollisionEnabled(ulong groupA, ulong groupB);
	}

	/** @} */
}
