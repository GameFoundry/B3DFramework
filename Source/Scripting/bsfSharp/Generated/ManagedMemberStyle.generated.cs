//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Contains information about a style of a serializable field.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ManagedMemberStyle
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ManagedMemberStyle Default()
		{
			ManagedMemberStyle value = new ManagedMemberStyle();
			value.RangeMin = 0f;
			value.RangeMax = 0f;
			value.StepIncrement = 0f;
			value.DisplayAsSlider = false;
			value.CategoryName = "";
			value.Order = 0;

			return value;
		}

		/// <summary>Returns the lower bound of the range. Only relevant if</summary>
		public float RangeMin;
		/// <summary>Returns the upper bound of the range. Only relevant if</summary>
		public float RangeMax;
		/// <summary>Minimum increment the field value can be increment/decremented by. Only relevant if</summary>
		public float StepIncrement;
		/// <summary>If true, number fields will be displayed as sliders instead of regular input boxes.</summary>
		public bool DisplayAsSlider;
		/// <summary>Name of the category to display in inspector, if the member is part of one.</summary>
		public string CategoryName;
		/// <summary>Determines ordering in inspector relative to other members.</summary>
		public int Order;
	}
}
