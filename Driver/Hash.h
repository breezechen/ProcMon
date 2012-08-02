
#include <ntddk.h>

BOOLEAN GetFullPath(IN PUNICODE_STRING NtFullName);

ULONG GetHashCode(IN PUNICODE_STRING NtFullName);

VOID InitHashTable();

typedef struct _IMAGE_DATA_DIRECTORY {
  ULONG VirtualAddress;
  ULONG Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_OPTIONAL_HEADER {
  USHORT               Magic;
  UCHAR                MajorLinkerVersion;
  UCHAR                MinorLinkerVersion;
  ULONG                SizeOfCode;
  ULONG                SizeOfInitializedData;
  ULONG                SizeOfUninitializedData;
  ULONG                AddressOfEntryPoint;
  ULONG                BaseOfCode;
  ULONG                BaseOfData;
  ULONG                ImageBase;
  ULONG                SectionAlignment;
  ULONG                FileAlignment;
  USHORT               MajorOperatingSystemVersion;
  USHORT               MinorOperatingSystemVersion;
  USHORT               MajorImageVersion;
  USHORT               MinorImageVersion;
  USHORT               MajorSubsystemVersion;
  USHORT               MinorSubsystemVersion;
  ULONG                Win32VersionValue;
  ULONG                SizeOfImage;
  ULONG                SizeOfHeaders;
  ULONG                CheckSum;
  USHORT               Subsystem;
  USHORT               DllCharacteristics;
  ULONG                SizeOfStackReserve;
  ULONG                SizeOfStackCommit;
  ULONG                SizeOfHeapReserve;
  ULONG                SizeOfHeapCommit;
  ULONG                LoaderFlags;
  ULONG                NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER64 {
            USHORT Magic;
            UCHAR MajorLinkerVersion;
            UCHAR MinorLinkerVersion;
            ULONG SizeOfCode;
            ULONG SizeOfInitializedData;
            ULONG SizeOfUninitializedData;
            ULONG AddressOfEntryPoint;
            ULONG BaseOfCode;
            ULONGLONG ImageBase;
            ULONG SectionAlignment;
            ULONG FileAlignment;
            USHORT MajorOperatingSystemVersion;
            USHORT MinorOperatingSystemVersion;
            USHORT MajorImageVersion;
            USHORT MinorImageVersion;
            USHORT MajorSubsystemVersion;
            USHORT MinorSubsystemVersion;
            ULONG Win32VersionValue;
            ULONG SizeOfImage;
            ULONG SizeOfHeaders;
            ULONG CheckSum;
            USHORT Subsystem;
            USHORT DllCharacteristics;
            ULONGLONG SizeOfStackReserve;
            ULONGLONG SizeOfStackCommit;
            ULONGLONG SizeOfHeapReserve;
            ULONGLONG SizeOfHeapCommit;
            ULONG LoaderFlags;
            ULONG NumberOfRvaAndSizes;
            IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;


typedef struct _IMAGE_DOS_HEADER {  // DOS .EXE header
    USHORT e_magic;         // Magic number
    USHORT e_cblp;          // Bytes on last page of file
    USHORT e_cp;            // Pages in file
    USHORT e_crlc;          // Relocations
    USHORT e_cparhdr;       // Size of header in paragraphs
    USHORT e_minalloc;      // Minimum extra paragraphs needed
    USHORT e_maxalloc;      // Maximum extra paragraphs needed
    USHORT e_ss;            // Initial (relative) SS value
    USHORT e_sp;            // Initial SP value
    USHORT e_csum;          // Checksum
    USHORT e_ip;            // Initial IP value
    USHORT e_cs;            // Initial (relative) CS value
    USHORT e_lfarlc;        // File address of relocation table
    USHORT e_ovno;          // Overlay number
    USHORT e_res[4];        // Reserved words
    USHORT e_oemid;         // OEM identifier (for e_oeminfo)
    USHORT e_oeminfo;       // OEM information; e_oemid specific
    USHORT e_res2[10];      // Reserved words
    LONG   e_lfanew;        // File address of new exe header
  } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;
