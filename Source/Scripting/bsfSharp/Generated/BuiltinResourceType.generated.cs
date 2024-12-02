//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Types of resources accessible from script code.</summary>
	public enum BuiltinResourceType
	{
		SpriteVectorPath = 1239,
		StringTable = 1083,
		ShaderInclude = 1072,
		Mesh = 1002,
		Texture = 1001,
		PhysicsMaterial = 1092,
		SpriteTexture = 30002,
		Material = 1017,
		Shader = 1016,
		Font = 1051,
		Undefined = 0,
		Prefab = 1077,
		PlainText = 30005,
		ScriptCode = 30006,
		PhysicsMesh = 1099,
		AudioClip = 1111,
		AnimationClip = 1115,
		VectorField = 1180,
		SpriteImage = 1237,
		SpriteGlyph = 1238,
		VectorPath = 1232
	}
}
