//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Valid serializable script types.</summary>
	public enum ManagedPrimitiveType
	{
		I32 = 6,
		Bool = 0,
		Char = 1,
		I8 = 2,
		String = 12,
		U8 = 3,
		I16 = 4,
		U16 = 5,
		U32 = 7,
		I64 = 8,
		U64 = 9,
		Float = 10,
		Double = 11,
		Count = 13
	}
}
