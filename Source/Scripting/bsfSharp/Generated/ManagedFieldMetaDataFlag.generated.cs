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
		/// <summary>Integer or floating point field with min/max range.</summary>
		Range = 4,
		/// <summary>Field will be automatically serialized.</summary>
		Serializable = 1,
		/// <summary>Field containing a reference type that should never be null.</summary>
		NotNull = 128,
		/// <summary>Field will be visible in the default inspector.</summary>
		Inspectable = 2,
		/// <summary>Integer or floating point field with a minimum increment/decrement step.</summary>
		Step = 8,
		/// <summary>Field can be animated through the animation window.</summary>
		Animable = 16,
		/// <summary>Integer field rendered as a layer selection dropdown.</summary>
		AsLayerMask = 32,
		/// <summary>
		/// Signifies that the field containing a class/struct should display the child fields of that objects as if they were 
		/// part of the parent class in the inspector.
		/// </summary>
		Inline = 8192,
		/// <summary>Field containing a reference type being passed by copy instead of by reference.</summary>
		PassByCopy = 64,
		/// <summary>
		/// Field represents a property that wraps a native object. Getters and setters of such a property issue calls into 
		/// native code to update the native object.
		/// </summary>
		NativeWrapper = 256,
		/// <summary>
		/// When a field changes those changes need to be applied to the parent object by calling the field setter. Only 
		/// applicable to properties containing reference types.
		/// </summary>
		ApplyOnDirty = 512,
		/// <summary>Signifies that a resource reference should be loaded when assigned to field through the inspector.</summary>
		LoadOnAssign = 16384,
		/// <summary>
		/// When a quaternion is displayed in the inspector, by default it will be displayed as converted into euler angles. Use 
		/// this flag to force it to be displayed as a quaternion (4D value) with no conversion instead.
		/// </summary>
		AsQuaternion = 1024,
		/// <summary>
		/// Fields contains information about a category, which is used for grouping fields under a foldout in the inspector. 
		/// Retrieve the category field style for information about the category.
		/// </summary>
		Category = 2048,
		/// <summary>
		/// Field contains information about its order relative to other fields. Retrieve the order field style for information 
		/// about the order.
		/// </summary>
		Order = 4096,
		/// <summary>Field containing a color that supports high dynamic range.</summary>
		HDR = 32768
	}
}
