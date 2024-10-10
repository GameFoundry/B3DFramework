//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Valid reference script types.</summary>
	public enum ManagedReferenceType
	{
		BuiltinResourceBase = 0,
		Count = 10,
		BuiltinResource = 1,
		ManagedResourceBase = 2,
		ManagedResource = 3,
		BuiltinComponentBase = 4,
		BuiltinComponent = 5,
		ManagedComponentBase = 6,
		ManagedComponent = 7,
		SceneObject = 8,
		ReflectableObject = 9
	}
}
