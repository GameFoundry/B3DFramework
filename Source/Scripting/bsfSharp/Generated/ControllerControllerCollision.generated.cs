//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>Contains data about a collision between two character controllers.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ControllerControllerCollision
	{
		///<summary>
		/// Returns a subset of this struct. This subset usually contains common fields shared with another struct.
		///</summary>
		public ControllerCollision GetBase()
		{
			ControllerCollision value;
			value.Position = Position;
			value.Normal = Normal;
			value.MotionDir = MotionDir;
			value.MotionAmount = MotionAmount;
			return value;
		}

		///<summary>
		/// Assigns values to a subset of fields of this struct. This subset usually contains common field shared with 
		/// another struct.
		///</summary>
		public void SetBase(ControllerCollision value)
		{
			Position = value.Position;
			Normal = value.Normal;
			MotionDir = value.MotionDir;
			MotionAmount = value.MotionAmount;
		}

		/// <summary>
		/// Component of the controller that was touched. Can be null if the controller has no component parent, in which case 
		/// check #controllerRaw.
		/// </summary>
		public CharacterController Controller;
		/// <summary>Contact position.</summary>
		public Vector3 Position;
		/// <summary>Contact normal.</summary>
		public Vector3 Normal;
		/// <summary>Direction of motion after the hit.</summary>
		public Vector3 MotionDir;
		/// <summary>Magnitude of motion after the hit.</summary>
		public float MotionAmount;
	}

	/** @} */
}
