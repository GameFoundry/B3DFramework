//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Flags that are used to further define a field in a managed serializable object.</summary>
	public enum ManagedFieldMetaDataFlag
	{
		Range = 4,
		Serializable = 1,
		NotNull = 128,
		Inspectable = 2,
		Step = 8,
		Animable = 16,
		AsLayerMask = 32,
		Inline = 8192,
		PassByCopy = 64,
		NativeWrapper = 256,
		ApplyOnDirty = 512,
		LoadOnAssign = 16384,
		AsQuaternion = 1024,
		Category = 2048,
		Order = 4096,
		HDR = 32768
	}
}
