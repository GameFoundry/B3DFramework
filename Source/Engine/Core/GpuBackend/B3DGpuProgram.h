//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "Utility/B3DDataBlob.h"
#include "Utility/B3DTArrayView.h"
#include "GpuBackend/B3DVertexDescription.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	struct GpuProgramBytecode;

	/** Identifies what a GpuDescriptorTableEntry describes: a pointer to a nested table, or a leaf resource binding. */
	enum class GpuDescriptorEntryKind
	{
		SubTable, /**< Entry points at another table in GpuResourceTableLayout::Tables (via TableIndex). */
		Resource, /**< Entry is a leaf resource binding (constant buffer / texture / buffer / sampler). */
	};

	/**
	 * One node in a GpuResourceTableLayout: a contiguous group of descriptors and/or pointers to nested tables that a
	 * backend packs together. Its entries live in GpuResourceTableLayout::Entries as the half-open range [FirstEntry, FirstEntry + EntryCount).
	 */
	struct GpuDescriptorTable
	{
		u32 Set = 0;           /**< Engine set (== register space) this table backs. Unused for the root table (index 0). */
		u32 OffsetInBytes = 0; /**< Byte offset of this table's data within its parent table. 0 for the root table. */
		u32 SizeInBytes = 0;   /**< Total byte size of this table's contents. */
		u32 FirstEntry = 0;    /**< Index of this table's first entry in GpuResourceTableLayout::Entries. */
		u32 EntryCount = 0;    /**< Number of entries belonging to this table. */
	};

	/**
	 * A single member of a GpuDescriptorTable: either a pointer to a nested table (Kind == SubTable) or a leaf resource
	 * binding (Kind == Resource). References to other tables are by array index into GpuResourceTableLayout::Tables,
	 * never by pointer, so the whole layout is trivially serializable and relocatable.
	 */
	struct GpuDescriptorTableEntry
	{
		GpuDescriptorEntryKind Kind = GpuDescriptorEntryKind::Resource; /**< Selects which of the fields below apply. */
		u32 OffsetInBytes = 0; /**< Byte offset of this entry within its parent table. */

		// Kind == SubTable:
		u32 TableIndex = 0; /**< Index into GpuResourceTableLayout::Tables of the nested table this entry points at. */

		// Kind == Resource:
		GpuParameterType Type = GpuParameterType::Unknown; /**< Resource class of the leaf binding. */
		u32 Slot = 0;                  /**< Engine slot the resource was declared at. */
		u32 DescriptorCount = 1;       /**< Number of descriptors (array size); 1 for a non-array binding. */
		u32 DescriptorSizeInBytes = 0; /**< Size of a single descriptor; 0 when driver-managed. */
	};

	/**
	 * Description of how a GPU program's resources are packed into descriptor tables.
	 *
	 * The layout is a flat pool of tables that reference each other by array index. Tables[0] is the root, 
	 * the traversal helpers walk the hierarchy from there.
	 */
	struct B3D_EXPORT GpuResourceTableLayout : IReflectable
	{
		/** All descriptor tables, flattened. Tables[0] is the root table. Empty when the program binds no resources. */
		Vector<GpuDescriptorTable> Tables;

		/** Shared entry pool. Each table owns the sub-range [FirstEntry, FirstEntry + EntryCount) of this vector. */
		Vector<GpuDescriptorTableEntry> Entries;

		/** True when the program declares no resource tables (a resource-less shader). */
		bool IsEmpty() const { return Tables.empty(); }

		/** Returns the root (top-level) table. Only valid when !IsEmpty(). */
		const GpuDescriptorTable& GetRootTable() const { return Tables.front(); }

		/** Returns the entries owned by @p table as a contiguous read-only view into Entries. */
		TArrayView<const GpuDescriptorTableEntry> GetEntries(const GpuDescriptorTable& table) const;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
		friend class GpuResourceTableLayoutRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Descriptor structure used for initialization of a GpuProgram. */
	struct B3D_EXPORT GpuProgramCreateInformation : public IReflectable
	{
		String Name; /**< Name of the program. Used primarily for debugging. */
		String Source; /**< Source code to compile the program from. */
		String EntryPoint; /**< Name of the entry point function, for example "main". */
		String Language; /**< Language the source is written in, for example "hlsl" or "glsl". */
		GpuProgramType Type = GPT_VERTEX_PROGRAM; /**< Type of the program, for example vertex or fragment. */
		bool RequiresAdjacency = false; /**< If true then adjacency information will be provided when rendering. */

		/**
		 * Optional intermediate version of the GPU program. Can significantly speed up GPU program compilation/creation
		 * when supported by the render backend. Call render::GpuProgram::CompileBytecode to generate it.
		 */
		TShared<GpuProgramBytecode> Bytecode;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
		friend class GpuProgramCreateInformationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * A GPU program compiled to an intermediate bytecode format, as well as any relevant meta-data that could be
	 * extracted from that format.
	 */
	struct B3D_EXPORT GpuProgramBytecode : IReflectable
	{
		~GpuProgramBytecode();

		/** Instructions (compiled code) for the GPU program. Contains no data if compilation was not succesful. */
		DataBlob Instructions;

		/** Reflected information about GPU program parameters. */
		TShared<GpuProgramParameterDescription> ParameterDescription;

		/** Reflected description of how the program's resources pack into descriptor tables (SRT / root signature). */
		TShared<GpuResourceTableLayout> ResourceTableLayout;

		/** Input parameters for a vertex GPU program. */
		Vector<VertexElement> VertexInput;

		/** Messages output during the compilation process. Includes errors in case compilation failed. */
		String Messages;

		/** Identifier of the compiler that compiled the bytecode. */
		String CompilerId;

		/** Version of the compiler that compiled the bytecode. */
		u32 CompilerVersion = 0;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GpuProgramBytecodeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains a GPU program such as vertex or fragment program which gets compiled from the provided source code.
	 *
	 * @note	Thread safe (Immutable).
	 */
	class B3D_EXPORT GpuProgram : public IReflectable
	{
	public:
		virtual ~GpuProgram();

		/** Initializes the object. The object should not be used before this is called. */
		virtual void Initialize() { }

		/** Returns whether this program can be supported on the current renderer and hardware. */
		virtual bool IsSupported() const;

		/** Returns true if program was successfully compiled. */
		virtual bool IsCompiled() const { return mIsCompiled; }

		/**	Returns an error message returned by the compiler, if the compilation failed. */
		virtual String GetCompileErrorMessage() const { return mCompileMessages; }

		/**
		 * Sets whether this geometry program requires adjacency information from the input primitives.
		 *
		 * @note	Only relevant for geometry programs.
		 */
		virtual void SetAdjacencyInfoRequired(bool required) { mNeedsAdjacencyInfo = required; }

		/**
		 * Returns whether this geometry program requires adjacency information from the input primitives.
		 *
		 * @note	Only relevant for geometry programs.
		 */
		virtual bool IsAdjacencyInfoRequired() const { return mNeedsAdjacencyInfo; }

		/**	Type of GPU program (for example fragment, vertex). */
		GpuProgramType GetType() const { return mType; }

		/** Returns the debug name of this program (for example "MyShader (Vertex Program)"). May be empty. */
		const String& GetName() const { return mName; }

		/** Returns description of all parameters in this GPU program. */
		TShared<GpuProgramParameterDescription> GetParameterDescription() const { return mParametersDescription; }

		/**	Returns a list of vertex elements that a vertex program expects as inputs. Only relevant for vertex programs. */
		TShared<VertexDescription> GetVertexInputDescription() const { return mVertexInputDescription; }

		/** Returns the compiled bytecode of this program. */
		TShared<GpuProgramBytecode> GetBytecode() const { return mBytecode; }

	protected:
		GpuProgram(const GpuProgramCreateInformation& createInformation);

		bool mNeedsAdjacencyInfo;

		bool mIsCompiled = false;
		String mCompileMessages;

		TShared<GpuProgramParameterDescription> mParametersDescription;
		TShared<VertexDescription> mVertexInputDescription;

		GpuProgramType mType;
		String mLanguage;
		String mName;
		String mEntryPoint;
		String mSource;

		TShared<GpuProgramBytecode> mBytecode;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GpuProgramRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
