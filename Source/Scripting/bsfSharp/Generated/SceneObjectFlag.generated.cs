//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Possible modifiers that can be applied to a SceneObject.</summary>
	public enum SceneObjectFlag
	{
		/// <summary>Object will be skipped when saving the scene hierarchy or a prefab.</summary>
		DontSave = 1,
		/// <summary>
		/// Provides a hint to external systems that his object is used by engine internals. For example, those systems might not 
		/// want to display those objects together with the user created ones.
		/// </summary>
		Internal = 4,
		/// <summary>
		/// Object will remain in the scene even after scene clear, unless destroyed directly. This only works with top-level 
		/// objects.
		/// </summary>
		Persistent = 2
	}
}
