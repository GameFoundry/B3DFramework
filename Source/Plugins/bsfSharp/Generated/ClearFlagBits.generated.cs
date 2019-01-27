//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Flags that determine which portion of the viewport to clear.</summary>
	public enum ClearFlags
	{
		Empty = 0,
		Color = 1,
		Depth = 2,
		Stencil = 4
	}
}
