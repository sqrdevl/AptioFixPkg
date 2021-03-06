/**

  Various helper functions.

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/SimpleFileSystem.h>

#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>

#include "Config.h"
#include "Lib.h"
#include "ServiceOverrides.h"

CHAR16 *mEfiMemoryTypeDesc[EfiMaxMemoryType] = {
  L"Reserved",
  L"LDR_code",
  L"LDR_data",
  L"BS_code",
  L"BS_data",
  L"RT_code",
  L"RT_data",
  L"Available",
  L"Unusable",
  L"ACPI_recl",
  L"ACPI_NVS",
  L"MemMapIO",
  L"MemPortIO",
  L"PAL_code"
};

CHAR16 *mEfiAllocateTypeDesc[MaxAllocateType] = {
  L"AllocateAnyPages",
  L"AllocateMaxAddress",
  L"AllocateAddress"
};

CHAR16 *mEfiLocateSearchType[] = {
  L"AllHandles",
  L"ByRegisterNotify",
  L"ByProtocol"
};

// From xnu: pexpert/i386/efi.h (legacy no longer used variable guid?)
EFI_GUID mAppleVendorGuid                            = {0xAC39C713, 0x7E50, 0x423D, {0x88, 0x9D, 0x27, 0x8F, 0xCC, 0x34, 0x22, 0xB6}};
// Unknown (no longer used?) guids generally from Clover and rEFInd
EFI_GUID mDataHubOptionsGuid                         = {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xC6, 0xEC, 0xF5, 0xDA, 0x75, 0x47, 0x31}};
EFI_GUID mNotifyMouseActivity                        = {0xF913C2C2, 0x5351, 0x4FDB, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}};
EFI_GUID mMsgLogProtocolGuid                         = {0x511CE018, 0x0018, 0x4002, {0x20, 0x12, 0x17, 0x38, 0x05, 0x01, 0x02, 0x03}};

MAP_EFI_GUID_STR EfiGuidStrMap[] = {
#if !defined (MDEPKG_NDEBUG)
  {&gEfiFileInfoGuid, L"gEfiFileInfoGuid"},
  {&gEfiFileSystemInfoGuid, L"gEfiFileSystemInfoGuid"},
  {&gEfiFileSystemVolumeLabelInfoIdGuid, L"gEfiFileSystemVolumeLabelInfoIdGuid"},
  {&gEfiLoadedImageProtocolGuid, L"gEfiLoadedImageProtocolGuid"},
  {&gEfiDevicePathProtocolGuid, L"gEfiDevicePathProtocolGuid"},
  {&gEfiSimpleFileSystemProtocolGuid, L"gEfiSimpleFileSystemProtocolGuid"},
  {&gEfiBlockIoProtocolGuid, L"gEfiBlockIoProtocolGuid"},
  {&gEfiBlockIo2ProtocolGuid, L"gEfiBlockIo2ProtocolGuid"},
  {&gEfiDiskIoProtocolGuid, L"gEfiDiskIoProtocolGuid"},
  {&gEfiDiskIo2ProtocolGuid, L"gEfiDiskIo2ProtocolGuid"},
  {&gEfiGraphicsOutputProtocolGuid, L"gEfiGraphicsOutputProtocolGuid"},

  {&gEfiConsoleControlProtocolGuid, L"gEfiConsoleControlProtocolGuid"},
  {&gAppleFirmwarePasswordProtocolGuid, L"gAppleFirmwarePasswordProtocolGuid"},
  {&gEfiGlobalVariableGuid, L"gEfiGlobalVariableGuid"},
  {&gEfiDevicePathPropertyDatabaseProtocolGuid, L"gEfiDevicePathPropertyDatabaseProtocolGuid"},
  {&gAppleBootVariableGuid, L"gAppleBootVariableGuid"},
  {&gAppleVendorVariableGuid, L"gAppleVendorVariableGuid"},
  {&gAppleFramebufferInfoProtocolGuid, L"gAppleFramebufferInfoProtocolGuid"},
  {&gAppleKeyMapAggregatorProtocolGuid, L"gAppleKeyMapAggregatorProtocolGuid"},
  {&gAppleNetBootProtocolGuid, L"gAppleNetBootProtocolGuid"},
  {&gAppleImageCodecProtocolGuid, L"gAppleImageCodecProtocolGuid"},
  {&mAppleVendorGuid, L"mAppleVendorGuid"},
  {&gAppleTrbSecureVariableGuid, L"gAppleTrbSecureVariableGuid"},
  {&mDataHubOptionsGuid, L"mDataHubOptionsGuid"},
  {&mNotifyMouseActivity, L"mNotifyMouseActivity"},
  {&gEfiDataHubProtocolGuid, L"gEfiDataHubProtocolGuid"},
  {&gEfiMiscSubClassGuid, L"gEfiMiscSubClassGuid"},
  {&gEfiProcessorSubClassGuid, L"gEfiProcessorSubClassGuid"},
  {&gEfiMemorySubClassGuid, L"gEfiMemorySubClassGuid"},
  {&mMsgLogProtocolGuid, L"mMsgLogProtocolGuid"},
  {&gEfiLegacy8259ProtocolGuid, L"gEfiLegacy8259ProtocolGuid"},
#endif
  {NULL, NULL}
};

STATIC CHAR16 EfiGuidStrTmp[48];

/** Returns GUID as string, with friendly name for known guids. */
CHAR16 *
EFIAPI
GuidStr (
  IN EFI_GUID *Guid
  )
{
  UINTN       i;
  CHAR16      *Str = NULL;

  for(i = 0; EfiGuidStrMap[i].Guid != NULL; i++) {
    if (CompareGuid(EfiGuidStrMap[i].Guid, Guid)) {
      Str = EfiGuidStrMap[i].Str;
      break;
    }
  }

  if (Str == NULL) {
    UnicodeSPrint(EfiGuidStrTmp, sizeof(EfiGuidStrTmp) - sizeof(EfiGuidStrTmp[0]), L"%g", Guid);
    Str = EfiGuidStrTmp;
  }
  return Str;
}

/** Returns upper case version of char - valid only for ASCII chars in unicode. */
CHAR16
EFIAPI
ToUpperChar (
  IN CHAR16 Chr
  )
{
  CHAR8   C;

  if (Chr > 0xFF) return Chr;
  C = (CHAR8)Chr;
  return ((C >= 'a' && C <= 'z') ? C - ('a' - 'A') : C);
}

/** Returns the first occurrence of a Null-terminated Unicode SearchString
  * in a Null-terminated Unicode String.
  * Compares just first 8 bits of chars (valid for ASCII), case insensitive.
  * Copied from MdePkg/Library/BaseLib/String.c and modified
  */
CHAR16 *
EFIAPI
StrStriBasic (
  IN CONST CHAR16   *String,
  IN CONST CHAR16   *SearchString
  )
{
  CONST CHAR16 *FirstMatch;
  CONST CHAR16 *SearchStringTmp;

  if (*SearchString == L'\0') {
    return (CHAR16 *) String;
  }

  while (*String != L'\0') {
    SearchStringTmp = SearchString;
    FirstMatch = String;

    while ((ToUpperChar(*String) == ToUpperChar(*SearchStringTmp)) 
        && (*String != L'\0')) {
      String++;
      SearchStringTmp++;
    }

    if (*SearchStringTmp == L'\0') {
      return (CHAR16 *) FirstMatch;
    }

    if (*String == L'\0') {
      return NULL;
    }

    String = FirstMatch + 1;
  }

  return NULL;
}

/** Returns TRUE if String1 starts with String2, FALSE otherwise. Compares just first 8 bits of chars (valid for ASCII), case insensitive.. */
BOOLEAN
EFIAPI
StriStartsWithBasic(
  IN CHAR16 *String1,
  IN CHAR16 *String2
  )
{
  CHAR16   Chr1;
  CHAR16   Chr2;
  BOOLEAN  Result;

  if (String1 == NULL || String2 == NULL) {
    return FALSE;
  }
  if (*String1 == L'\0' && *String2 == L'\0') {
    return TRUE;
  }
  if (*String1 == L'\0' || *String2 == L'\0') {
    return FALSE;
  }

  Chr1 = ToUpperChar(*String1);
  Chr2 = ToUpperChar(*String2);
  while ((Chr1 != L'\0') && (Chr2 != L'\0') && (Chr1 == Chr2)) {
    String1++;
    String2++;
    Chr1 = ToUpperChar(*String1);
    Chr2 = ToUpperChar(*String2);
  }

  Result = ((Chr1 == L'\0') && (Chr2 == L'\0'))
  || ((Chr1 != L'\0') && (Chr2 == L'\0'));

  return Result;
}

VOID
EFIAPI
ShrinkMemMap (
  IN UINTN                    *MemoryMapSize,
  IN EFI_MEMORY_DESCRIPTOR    *MemoryMap,
  IN UINTN                    DescriptorSize
  )
{
  UINTN                   SizeFromDescToEnd;
  UINT64                  Bytes;
  EFI_MEMORY_DESCRIPTOR   *PrevDesc;
  EFI_MEMORY_DESCRIPTOR   *Desc;
  BOOLEAN                 CanBeJoined;
  BOOLEAN                 HasEntriesToRemove;

  PrevDesc           = MemoryMap;
  Desc               = NEXT_MEMORY_DESCRIPTOR(PrevDesc, DescriptorSize);
  SizeFromDescToEnd  = *MemoryMapSize - DescriptorSize;
  *MemoryMapSize     = DescriptorSize;
  HasEntriesToRemove = FALSE;

  while (SizeFromDescToEnd > 0) {
    Bytes = (((UINTN) PrevDesc->NumberOfPages) * EFI_PAGE_SIZE);
    CanBeJoined = FALSE;
    if (Desc->Attribute == PrevDesc->Attribute && PrevDesc->PhysicalStart + Bytes == Desc->PhysicalStart) {
      // It *should* be safe to join this with conventional memory, because the firmware should not use
      // GetMemoryMap for allocation, and for the kernel it does not matter, since it joins them.
      CanBeJoined = (Desc->Type == EfiBootServicesCode ||
        Desc->Type == EfiBootServicesData ||
        Desc->Type == EfiConventionalMemory ||
        Desc->Type == EfiLoaderCode ||
        Desc->Type == EfiLoaderData) && (
        PrevDesc->Type == EfiBootServicesCode ||
        PrevDesc->Type == EfiBootServicesData ||
        PrevDesc->Type == EfiConventionalMemory ||
        PrevDesc->Type == EfiLoaderCode ||
        PrevDesc->Type == EfiLoaderData);
    }

    if (CanBeJoined) {
      // two entries are the same/similar - join them
      PrevDesc->Type = EfiConventionalMemory;
      PrevDesc->NumberOfPages += Desc->NumberOfPages;
      HasEntriesToRemove = TRUE;
    } else {
      // can not be joined - we need to move to next
      *MemoryMapSize += DescriptorSize;
      PrevDesc = NEXT_MEMORY_DESCRIPTOR(PrevDesc, DescriptorSize);
      if (HasEntriesToRemove) {
        // have entries between PrevDesc and Desc which are joined to PrevDesc
        // we need to copy [Desc, end of list] to PrevDesc + 1
        CopyMem(PrevDesc, Desc, SizeFromDescToEnd);
        Desc = PrevDesc;
        HasEntriesToRemove = FALSE;
      }
    }
    // move to next
    Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
    SizeFromDescToEnd -= DescriptorSize;
  }
}

VOID
EFIAPI
PrintMemMap (
  IN CONST CHAR16             *Name,
  IN UINTN                    MemoryMapSize,
  IN UINTN                    DescriptorSize,
  IN EFI_MEMORY_DESCRIPTOR    *MemoryMap,
  IN VOID                     *Shims,
  IN EFI_PHYSICAL_ADDRESS     SysTable
  )
{
  UINTN                   NumEntries;
  UINTN                   Index;
  UINT64                  Bytes;
  EFI_MEMORY_DESCRIPTOR   *Desc;

  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;
  PrintScreen (L"--- Dump Memory Map (%s) start ---\n", Name);
  PrintScreen (L"MEMMAP: Size=%d, Addr=%p, DescSize=%d, Shims=%08lX, ST=%08lX\n",
    MemoryMapSize, MemoryMap, DescriptorSize, (UINTN)Shims, (UINTN)SysTable);
  PrintScreen (L"Type      Start      End        Virtual          # Pages    Attributes\n");
  for (Index = 0; Index < NumEntries; Index++) {

    Bytes = EFI_PAGES_TO_SIZE(Desc->NumberOfPages);

    PrintScreen (L"%-9s %010lX %010lX %016lX %010lX %016lX\n",
      mEfiMemoryTypeDesc[Desc->Type],
      Desc->PhysicalStart,
      Desc->PhysicalStart + Bytes - 1,
      Desc->VirtualStart,
      Desc->NumberOfPages,
      Desc->Attribute
    );
    Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
    if ((Index + 1) % 16 == 0)
      gBS->Stall(5000000);
  }

  PrintScreen (L"--- Dump Memory Map (%s) end ---\n", Name);
  gBS->Stall (5000000);
}

VOID
EFIAPI
PrintSystemTable (
  IN EFI_SYSTEM_TABLE  *ST
  )
{
#if !defined (MDEPKG_NDEBUG)
  UINTN  Index;

  DEBUG ((DEBUG_VERBOSE, "SysTable: %p\n", ST));
  DEBUG ((DEBUG_VERBOSE, "- FirmwareVendor: %p, %s\n", ST->FirmwareVendor, ST->FirmwareVendor));
  DEBUG ((DEBUG_VERBOSE, "- ConsoleInHandle: %p, ConIn: %p\n", ST->ConsoleInHandle, ST->ConIn));
  DEBUG ((DEBUG_VERBOSE, "- RuntimeServices: %p, BootServices: %p, ConfigurationTable: %p\n", ST->RuntimeServices, ST->BootServices, ST->ConfigurationTable));
  DEBUG ((DEBUG_VERBOSE, "RT:\n"));
  DEBUG ((DEBUG_VERBOSE, "- GetVariable: %p, SetVariable: %p\n", ST->RuntimeServices->GetVariable, ST->RuntimeServices->SetVariable));

  for(Index = 0; Index < ST->NumberOfTableEntries; Index++) {
    DEBUG ((DEBUG_VERBOSE, "ConfTab: %p\n", ST->ConfigurationTable[Index].VendorTable));
  }
#endif
}

VOID
WaitForKeyPress (
  CHAR16 *Message
  )
{
  EFI_STATUS      Status;
  UINTN           Index;
  EFI_INPUT_KEY   Key;

  PrintScreen (L"%a", Message);
  do {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  } while(Status == EFI_SUCCESS);
  gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
  do {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  } while(Status == EFI_SUCCESS);
}

/** Returns file path from FilePathProto in allocated memory. Mem should be released by caler.*/
CHAR16 *
EFIAPI
FileDevicePathToText (
  EFI_DEVICE_PATH_PROTOCOL *FilePathProto
  )
{
  FILEPATH_DEVICE_PATH    *FilePath;
  CHAR16                  FilePathText[256]; // possible problem: if filepath is bigger
  CHAR16                  *OutFilePathText;
  UINTN                   Size;
  UINTN                   SizeAll;
  UINTN                   i;

  FilePathText[0] = L'\0';
  i = 4;
  SizeAll = 0;
  //DEBUG ((DEBUG_VERBOSE, "FilePathProto->Type: %d, SubType: %d, Length: %d\n", FilePathProto->Type, FilePathProto->SubType, DevicePathNodeLength(FilePathProto)));
  while (FilePathProto != NULL && FilePathProto->Type != END_DEVICE_PATH_TYPE && i > 0) {
    if (FilePathProto->Type == MEDIA_DEVICE_PATH && FilePathProto->SubType == MEDIA_FILEPATH_DP) {
      FilePath = (FILEPATH_DEVICE_PATH *) FilePathProto;
      Size = (DevicePathNodeLength(FilePathProto) - 4) / 2;
      if (SizeAll + Size < 256) {
        if (SizeAll > 0 && FilePathText[SizeAll / 2 - 2] != L'\\') {
          StrCatS(FilePathText, 256, L"\\");
        }
        StrCatS(FilePathText, 256, FilePath->PathName);
        SizeAll = StrSize(FilePathText);
      }
    }
    FilePathProto = NextDevicePathNode(FilePathProto);
    //DEBUG ((DEBUG_VERBOSE, "FilePathProto->Type: %d, SubType: %d, Length: %d\n", FilePathProto->Type, FilePathProto->SubType, DevicePathNodeLength(FilePathProto)));
    i--;
    //DEBUG ((DEBUG_VERBOSE, "FilePathText: %s\n", FilePathText));
  }

  OutFilePathText = NULL;
  Size = StrSize(FilePathText);
  if (Size > 2) {
    // we are allocating mem here - should be released by caller
    OutFilePathText = (CHAR16 *)DirectAllocatePool(Size);
    if (OutFilePathText)
      StrCpyS(OutFilePathText, Size/sizeof(CHAR16), FilePathText);
  }

  return OutFilePathText;
}

/** Helper function that calls GetMemoryMap(), allocates space for mem map and returns it. */
EFI_STATUS
EFIAPI
GetMemoryMapAlloc (
  IN OUT UINTN                    *AllocatedTopPages,
  OUT UINTN                       *MemoryMapSize,
  OUT EFI_MEMORY_DESCRIPTOR       **MemoryMap,
  OUT UINTN                       *MapKey,
  OUT UINTN                       *DescriptorSize,
  OUT UINT32                      *DescriptorVersion
  )
{
  EFI_STATUS               Status;

  *MemoryMapSize       = 0;
  *MemoryMap           = NULL;
  Status = OrgGetMemoryMap (
    MemoryMapSize,
    *MemoryMap,
    MapKey,
    DescriptorSize,
    DescriptorVersion
    );

  if (Status != EFI_BUFFER_TOO_SMALL) {
    DEBUG ((DEBUG_WARN, "Insane GetMemoryMap %r\n", Status));
    return Status;
  }

  do {
    //
    // This is done because extra allocations may increase memory map size.
    //
    *MemoryMapSize   += 256;

    //
    // Requested to allocate from top via pages.
    // This may be needed, because the pool memory may collide with the kernel.
    //
    if (AllocatedTopPages) {
      *MemoryMap         = (EFI_MEMORY_DESCRIPTOR *)BASE_4GB;
      *AllocatedTopPages = EFI_SIZE_TO_PAGES (*MemoryMapSize);
      Status = AllocatePagesFromTop (
        EfiBootServicesData,
        *AllocatedTopPages,
        (EFI_PHYSICAL_ADDRESS *)MemoryMap,
        FALSE
        );
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_WARN, "Temp memory map allocation from top failure %r\n", Status));
        *MemoryMap = NULL;
        return Status;
      }
    } else {
      *MemoryMap = DirectAllocatePool (*MemoryMapSize);
      if (!*MemoryMap) {
        DEBUG ((DEBUG_WARN, "Temp memory map direct allocation failure\n"));
        return EFI_OUT_OF_RESOURCES;
      }
    }

    Status = OrgGetMemoryMap (
      MemoryMapSize,
      *MemoryMap,
      MapKey,
      DescriptorSize,
      DescriptorVersion
      );

    if (EFI_ERROR (Status)) {
      if (AllocatedTopPages)
        gBS->FreePages ((EFI_PHYSICAL_ADDRESS)*MemoryMap, *AllocatedTopPages);
      else
        DirectFreePool (*MemoryMap);
      *MemoryMap = NULL;
    }
  } while (Status == EFI_BUFFER_TOO_SMALL);

  if (Status != EFI_SUCCESS)
    DEBUG ((DEBUG_WARN, "Failed to obtain memory map %r\n", Status));

  return Status;
}

/** Alloctes Pages from the top of mem, up to address specified in Memory. Returns allocated address in Memory. */
EFI_STATUS
EFIAPI
AllocatePagesFromTop (
  IN EFI_MEMORY_TYPE           MemoryType,
  IN UINTN                     Pages,
  IN OUT EFI_PHYSICAL_ADDRESS  *Memory,
  IN BOOLEAN                   CheckRange
  )
{
  EFI_STATUS              Status;
  UINTN                   MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR   *MemoryMap;
  UINTN                   MapKey;
  UINTN                   DescriptorSize;
  UINT32                  DescriptorVersion;
  EFI_MEMORY_DESCRIPTOR   *MemoryMapEnd;
  EFI_MEMORY_DESCRIPTOR   *Desc;

  Status = GetMemoryMapAlloc (NULL, &MemoryMapSize, &MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
  if (EFI_ERROR (Status))
    return Status;

  Status = EFI_NOT_FOUND;

  MemoryMapEnd = NEXT_MEMORY_DESCRIPTOR (MemoryMap, MemoryMapSize);
  Desc = PREV_MEMORY_DESCRIPTOR (MemoryMapEnd, DescriptorSize);

  for ( ; Desc >= MemoryMap; Desc = PREV_MEMORY_DESCRIPTOR (Desc, DescriptorSize)) {
    //
    // We are looking for some free memory descriptor that contains enough space below the specified memory
    // 
    if (Desc->Type == EfiConventionalMemory && Pages <= Desc->NumberOfPages &&
      Desc->PhysicalStart + EFI_PAGES_TO_SIZE (Pages) <= *Memory) {
      //
      // Free block found
      //
      if (Desc->PhysicalStart + EFI_PAGES_TO_SIZE ((UINTN)Desc->NumberOfPages) <= *Memory) {
        //
        // The whole block is under Memory - allocate from the top of the block
        //
        *Memory = Desc->PhysicalStart + EFI_PAGES_TO_SIZE ((UINTN)Desc->NumberOfPages - Pages);
      } else {
        //
        // The block contains enough pages under Memory, but spans above it - allocate below Memory
        //
        *Memory = *Memory - EFI_PAGES_TO_SIZE (Pages);
      }
      //
      // Ensure that the found block does not overlap with the kernel area
      //
      if (CheckRange && OverlapsWithSlide (*Memory, EFI_PAGES_TO_SIZE (Pages)))
        continue;

      Status = gBS->AllocatePages (
        AllocateAddress,
        MemoryType,
        Pages,
        Memory
        );
      break;
    }
  }

  DirectFreePool (MemoryMap);

  return Status;
}

/** Helper function to call ExitBootServices that can handle outdated MapKey issues. */
EFI_STATUS
ForceExitBootServices (
  IN EFI_EXIT_BOOT_SERVICES  ExitBs,
  IN EFI_HANDLE              ImageHandle,
  IN UINTN                   MapKey
  )
{
  EFI_STATUS               Status;
  EFI_MEMORY_DESCRIPTOR    *MemoryMap;
  UINTN                    MemoryMapSize;
  UINTN                    DescriptorSize;
  UINT32                   DescriptorVersion;

  //
  // Firstly try the easy way
  //
  Status = ExitBs (ImageHandle, MapKey);

  if (EFI_ERROR (Status)) {
    //
    // Just report error as var in nvram to be visible from macOS with "nvram -p"
    //
    gRT->SetVariable (L"aptiomemfix-exitbs",
      &gAppleBootVariableGuid,
      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
      4,
      "fail"
      );

    //
    // It is too late to free memory map here, but it does not matter,
    // because boot.efi has an old one and will freely use the memory.
    //
    Status = GetMemoryMapAlloc (NULL, &MemoryMapSize, &MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    DEBUG ((DEBUG_VERBOSE, "ExitBootServices: GetMemoryMapKey = %r\n", Status));
    if (Status == EFI_SUCCESS) {
      //
      // We have the latest memory map and its key, try again!
      //
      Status = ExitBs (ImageHandle, MapKey);
      DEBUG ((DEBUG_VERBOSE, "ExitBootServices: 2nd try = %r\n", Status));
      if (EFI_ERROR (Status))
        PrintScreen (L"AMF: ExitBootServices failed twice - %r\n", Status);
    } else {
      PrintScreen (L"AMF: Failed to get MapKey for ExitBootServices - %r\n", Status);
      Status = EFI_INVALID_PARAMETER;
    }

    if (EFI_ERROR (Status)) {
      PrintScreen (L"Waiting 10 secs...\n");
      gBS->Stall(10*1000000);
    }
  }

  return Status;
}

/** Prints via gST->ConOut without any allocations. */
VOID
EFIAPI
PrintScreen (
  IN  CONST CHAR16   *Format,
  ...
  )
{
  CHAR16 Buffer[1024];
  VA_LIST Marker;

  VA_START (Marker, Format);
  UnicodeVSPrint (Buffer, sizeof (Buffer), Format, Marker);
  VA_END (Marker);

  //
  // It is safe to call gST->ConOut->OutputString, because gBS->AllocatePool
  // is overridden by our own implementation with a custom allocator.
  //
  if (gST->ConOut)
    gST->ConOut->OutputString (gST->ConOut, Buffer);
}

/** Allocate without any pool allocations from the top of memory. */
VOID *
DirectAllocatePool (
  UINTN     Size
  )
{
  EFI_STATUS         Status;
  VOID               *Buffer;

  //
  // It is safe to call gBS->AllocatePool, because it is overridden
  // by our own implementation with a custom allocator.
  // This is left in case we decide to enforce the use of a custom allocator
  // for our own needs. For example, one could do the following for debug reasons:
  // DisableDynamicPoolAllocs(); gBS->AllocatePool(...); EnableDynamicPoolAllocs();
  //
  Status = gBS->AllocatePool (EfiBootServicesData, Size, &Buffer);
  if (!EFI_ERROR (Status))
    return Buffer;

  return NULL;
}

/** Free memory allocated by DirectAllocatePool. You are allowed to free AllocatePool memory as well. */
VOID
DirectFreePool (
  VOID      *Buffer
  )
{
  gBS->FreePool (Buffer);
}
