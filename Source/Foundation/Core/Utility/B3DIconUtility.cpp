//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DIconUtility.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DColor.h"
#include "Error/B3DException.h"

#define MSDOS_SIGNATURE 0x5A4D
#define PE_SIGNATURE 0x00004550
#define PE_32BIT_SIGNATURE 0x10B
#define PE_64BIT_SIGNATURE 0x20B
#define PE_NUM_DIRECTORY_ENTRIES 16
#define PE_SECTION_UNINITIALIZED_DATA 0x00000080
#define PE_IMAGE_DIRECTORY_ENTRY_RESOURCE 2
#define PE_IMAGE_RT_ICON 3

using namespace b3d;

/**	MS-DOS header found at the beggining in a PE format file. */
struct MSDOSHeader
{
	u16 Signature;
	u16 LastSize;
	u16 NumBlocks;
	u16 NumReloc;
	u16 HdrSize;
	u16 MinAlloc;
	u16 MaxAlloc;
	u16 Ss;
	u16 Sp;
	u16 Checksum;
	u16 Ip;
	u16 Cs;
	u16 RelocPos;
	u16 NumOverlay;
	u16 Reserved1[4];
	u16 OemId;
	u16 OemInfo;
	u16 Reserved2[10];
	u32 Lfanew;
};

/**	COFF header found in a PE format file. */
struct COFFHeader
{
	u16 Machine;
	u16 NumSections;
	u32 TimeDateStamp;
	u32 PtrSymbolTable;
	u32 NumSymbols;
	u16 SizeOptHeader;
	u16 Characteristics;
};

/**	Contains address and size of data areas in a PE image. */
struct PEDataDirectory
{
	u32 VirtualAddress;
	u32 Size;
};

/**	Optional header in a 32-bit PE format file. */
struct PEOptionalHeader32
{
	u16 Signature;
	u8 MajorLinkerVersion;
	u8 MinorLinkerVersion;
	u32 SizeCode;
	u32 SizeInitializedData;
	u32 SizeUninitializedData;
	u32 AddressEntryPoint;
	u32 BaseCode;
	u32 BaseData;
	u32 BaseImage;
	u32 AlignmentSection;
	u32 AlignmentFile;
	u16 MajorOsVersion;
	u16 MinorOsVersion;
	u16 MajorImageVersion;
	u16 MinorImageVersion;
	u16 MajorSubsystemVersion;
	u16 MinorSubsystemVersion;
	u32 Reserved;
	u32 SizeImage;
	u32 SizeHeaders;
	u32 Checksum;
	u16 Subsystem;
	u16 Characteristics;
	u32 SizeStackReserve;
	u32 SizeStackCommit;
	u32 SizeHeapReserve;
	u32 SizeHeapCommit;
	u32 LoaderFlags;
	u32 NumRvaAndSizes;
	PEDataDirectory DataDirectory[16];
};

/**	Optional header in a 64-bit PE format file. */
struct PEOptionalHeader64
{
	u16 Signature;
	u8 MajorLinkerVersion;
	u8 MinorLinkerVersion;
	u32 SizeCode;
	u32 SizeInitializedData;
	u32 SizeUninitializedData;
	u32 AddressEntryPoint;
	u32 BaseCode;
	u64 BaseImage;
	u32 AlignmentSection;
	u32 AlignmentFile;
	u16 MajorOsVersion;
	u16 MinorOsVersion;
	u16 MajorImageVersion;
	u16 MinorImageVersion;
	u16 MajorSubsystemVersion;
	u16 MinorSubsystemVersion;
	u32 Reserved;
	u32 SizeImage;
	u32 SizeHeaders;
	u32 Checksum;
	u16 Subsystem;
	u16 Characteristics;
	u64 SizeStackReserve;
	u64 SizeStackCommit;
	u64 SizeHeapReserve;
	u64 SizeHeapCommit;
	u32 LoaderFlags;
	u32 NumRvaAndSizes;
	PEDataDirectory DataDirectory[16];
};

/**	A section header in a PE format file. */
struct PESectionHeader
{
	char Name[8];
	u32 VirtualSize;
	u32 RelativeVirtualAddress;
	u32 PhysicalSize;
	u32 PhysicalAddress;
	u8 Deprecated[12];
	u32 Flags;
};

/**	A resource table header within a .rsrc section in a PE format file. */
struct PEImageResourceDirectory
{
	u32 Flags;
	u32 TimeDateStamp;
	u16 MajorVersion;
	u16 MinorVersion;
	u16 NumNamedEntries;
	u16 NumIdEntries;
};

/**	A single entry in a resource table within a .rsrc section in a PE format file. */
struct PEImageResourceEntry
{
	u32 Type;
	u32 OffsetDirectory : 31;
	u32 IsDirectory : 1;
};

/** An entry in a resource table referencing resource data. Found within a .rsrc section in a PE format file. */
struct PEImageResourceEntryData
{
	u32 OffsetData;
	u32 Size;
	u32 CodePage;
	u32 ResourceHandle;
};

/**	Header used in icon file format. */
struct IconHeader
{
	u32 Size;
	i32 Width;
	i32 Height;
	u16 Planes;
	u16 BitCount;
	u32 Compression;
	u32 SizeImage;
	i32 XPelsPerMeter;
	i32 YPelsPerMeter;
	u32 ClrUsed;
	u32 ClrImportant;
};

void IconUtility::UpdateIconExe(const Path& path, const Map<u32, SPtr<PixelData>>& pixelsPerSize)
{
	// A PE file is structured as such:
	//  - MSDOS Header
	//  - PE Signature
	//  - COFF Header
	//  - PE Optional Header
	//  - One or multiple sections
	//   - .code
	//   - .data
	//   - ...
	//   - .rsrc
	//    - icon/cursor/etc data

	std::fstream stream;
	stream.open(path.ToPlatformString().c_str(), std::ios::in | std::ios::out | std::ios::binary);

	// First check magic number to ensure file is even an executable
	u16 magicNum;
	stream.read((char*)&magicNum, sizeof(magicNum));
	if(magicNum != MSDOS_SIGNATURE)
		B3D_EXCEPT(InvalidStateException, "Provided file is not a valid executable.");

	// Read the MSDOS header and skip over it
	stream.seekg(0);

	MSDOSHeader msdosHeader;
	stream.read((char*)&msdosHeader, sizeof(MSDOSHeader));

	// Read PE signature
	stream.seekg(msdosHeader.Lfanew);

	u32 peSignature;
	stream.read((char*)&peSignature, sizeof(peSignature));

	if(peSignature != PE_SIGNATURE)
		B3D_EXCEPT(InvalidStateException, "Provided file is not in PE format.");

	// Read COFF header
	COFFHeader coffHeader;
	stream.read((char*)&coffHeader, sizeof(COFFHeader));

	if(coffHeader.SizeOptHeader == 0) // .exe files always have an optional header
		B3D_EXCEPT(InvalidStateException, "Provided file is not a valid executable.");

	u32 numSectionHeaders = coffHeader.NumSections;

	// Read optional header
	auto optionalHeaderPos = stream.tellg();

	u16 optionalHeaderSignature;
	stream.read((char*)&optionalHeaderSignature, sizeof(optionalHeaderSignature));

	PEDataDirectory* dataDirectory = nullptr;
	stream.seekg(optionalHeaderPos);
	if(optionalHeaderSignature == PE_32BIT_SIGNATURE)
	{
		PEOptionalHeader32 optionalHeader;
		stream.read((char*)&optionalHeader, sizeof(optionalHeader));

		dataDirectory = optionalHeader.DataDirectory + PE_IMAGE_DIRECTORY_ENTRY_RESOURCE;
	}
	else if(optionalHeaderSignature == PE_64BIT_SIGNATURE)
	{
		PEOptionalHeader64 optionalHeader;
		stream.read((char*)&optionalHeader, sizeof(optionalHeader));

		dataDirectory = optionalHeader.DataDirectory + PE_IMAGE_DIRECTORY_ENTRY_RESOURCE;
	}
	else
		B3D_EXCEPT(InvalidStateException, "Unrecognized PE format.");

	// Read section headers
	auto sectionHeaderPos = optionalHeaderPos + (std::ifstream::pos_type)coffHeader.SizeOptHeader;
	stream.seekg(sectionHeaderPos);

	PESectionHeader* sectionHeaders = B3DStackAllocate<PESectionHeader>(numSectionHeaders);
	stream.read((char*)sectionHeaders, sizeof(PESectionHeader) * numSectionHeaders);

	// Look for .rsrc section header
	std::function<void(PEImageResourceDirectory*, PEImageResourceDirectory*, u8*, u32)> setIconData =
		[&](PEImageResourceDirectory* base, PEImageResourceDirectory* current, u8* imageData, u32 sectionAddress)
	{
		u32 numEntries = current->NumIdEntries; // Not supporting name entries
		PEImageResourceEntry* entries = (PEImageResourceEntry*)(current + 1);

		for(u32 i = 0; i < numEntries; i++)
		{
			// Only at root does the type identify resource type
			if(base == current && entries[i].Type != PE_IMAGE_RT_ICON)
				continue;

			if(entries[i].IsDirectory)
			{
				PEImageResourceDirectory* child = (PEImageResourceDirectory*)(((u8*)base) + entries[i].OffsetDirectory);
				setIconData(base, child, imageData, sectionAddress);
			}
			else
			{
				PEImageResourceEntryData* data = (PEImageResourceEntryData*)(((u8*)base) + entries[i].OffsetDirectory);

				u8* iconData = imageData + (data->OffsetData - sectionAddress);
				UpdateIconData(iconData, pixelsPerSize);
			}
		}
	};

	for(u32 i = 0; i < numSectionHeaders; i++)
	{
		if(sectionHeaders[i].Flags & PE_SECTION_UNINITIALIZED_DATA)
			continue;

		if(strcmp(sectionHeaders[i].Name, ".rsrc") == 0)
		{
			u32 imageSize = sectionHeaders[i].PhysicalSize;
			u8* imageData = (u8*)B3DStackAllocate(imageSize);

			stream.seekg(sectionHeaders[i].PhysicalAddress);
			stream.read((char*)imageData, imageSize);

			u32 resourceDirOffset = dataDirectory->VirtualAddress - sectionHeaders[i].RelativeVirtualAddress;
			PEImageResourceDirectory* resourceDirectory = (PEImageResourceDirectory*)&imageData[resourceDirOffset];

			setIconData(resourceDirectory, resourceDirectory, imageData, sectionHeaders[i].RelativeVirtualAddress);
			stream.seekp(sectionHeaders[i].PhysicalAddress);
			stream.write((char*)imageData, imageSize);

			B3DStackFree(imageData);
		}
	}

	B3DStackFree(sectionHeaders);
	stream.close();
}

void IconUtility::UpdateIconData(u8* iconData, const Map<u32, SPtr<PixelData>>& pixelsPerSize)
{
	IconHeader* iconHeader = (IconHeader*)iconData;

	if(iconHeader->Size != sizeof(IconHeader) || iconHeader->Compression != 0 || iconHeader->Planes != 1 || iconHeader->BitCount != 32)
	{
		// Unsupported format
		return;
	}

	u8* iconPixels = iconData + sizeof(IconHeader);
	u32 width = iconHeader->Width;
	u32 height = iconHeader->Height / 2;

	auto iterFind = pixelsPerSize.find(width);
	if(iterFind == pixelsPerSize.end() || iterFind->second->GetWidth() != width || iterFind->second->GetHeight() != height)
	{
		// No icon of this size provided
		return;
	}

	// Write colors
	SPtr<PixelData> srcPixels = iterFind->second;
	u32* colorData = (u32*)iconPixels;

	u32 idx = 0;
	for(i32 y = (i32)height - 1; y >= 0; y--)
	{
		for(u32 x = 0; x < width; x++)
			colorData[idx++] = srcPixels->GetColorAt(x, y).GetAsBgra();
	}

	// Write AND mask
	u32 colorDataSize = width * height * sizeof(u32);
	u8* maskData = iconPixels + colorDataSize;

	u32 numPackedPixels = width / 8; // One per bit in byte

	for(i32 y = (i32)height - 1; y >= 0; y--)
	{
		u8 mask = 0;
		for(u32 packedX = 0; packedX < numPackedPixels; packedX++)
		{
			for(u32 pixelIdx = 0; pixelIdx < 8; pixelIdx++)
			{
				u32 x = packedX * 8 + pixelIdx;
				Color color = srcPixels->GetColorAt(x, y);
				if(color.A < 0.25f)
					mask |= 1 << (7 - pixelIdx);
			}

			*maskData = mask;
			maskData++;
		}
	}
}
