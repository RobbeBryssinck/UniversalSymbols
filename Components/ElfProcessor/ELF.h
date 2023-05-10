#pragma once

#include <cstdint>

#pragma pack(push, 1)

namespace ELF
{

  // e_ident size and indices.
  enum {
    EI_MAG0 = 0,       // File identification index.
    EI_MAG1 = 1,       // File identification index.
    EI_MAG2 = 2,       // File identification index.
    EI_MAG3 = 3,       // File identification index.
    EI_CLASS = 4,      // File class.
    EI_DATA = 5,       // Data encoding.
    EI_VERSION = 6,    // File version.
    EI_OSABI = 7,      // OS/ABI identification.
    EI_ABIVERSION = 8, // ABI version.
    EI_PAD = 9,        // Start of padding bytes.
    EI_NIDENT = 16     // Number of bytes in e_ident.
  };

  enum {
    ELFCLASSNONE = 0,
    ELFCLASS32 = 1, // 32-bit object file
    ELFCLASS64 = 2  // 64-bit object file
  };

  struct Elf32_Ehdr
  {
    unsigned char e_ident[EI_NIDENT]; // ELF Identification bytes
    uint16_t e_type;                // Type of file (see ET_* below)
    uint16_t e_machine;   // Required architecture for this file (see EM_*)
    uint32_t e_version;   // Must be equal to 1
    uint32_t e_entry;     // Address to jump to in order to start program
    uint32_t e_phoff;      // Program header table's file offset, in bytes
    uint32_t e_shoff;      // Section header table's file offset, in bytes
    uint32_t e_flags;     // Processor-specific flags
    uint16_t e_ehsize;    // Size of ELF header, in bytes
    uint16_t e_phentsize; // Size of an entry in the program header table
    uint16_t e_phnum;     // Number of entries in the program header table
    uint16_t e_shentsize; // Size of an entry in the section header table
    uint16_t e_shnum;     // Number of entries in the section header table
    uint16_t e_shstrndx;  // Sect hdr table index of sect name string table
  };

  struct Elf64_Ehdr
  {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
  };

  struct Elf32_Phdr
  {
    uint32_t p_type;   // Type of segment
    uint32_t p_offset;  // File offset where segment is located, in bytes
    uint32_t p_vaddr;  // Virtual address of beginning of segment
    uint32_t p_paddr;  // Physical address of beginning of segment (OS-specific)
    uint32_t p_filesz; // Num. of bytes in file image of segment (may be zero)
    uint32_t p_memsz;  // Num. of bytes in mem image of segment (may be zero)
    uint32_t p_flags;  // Segment flags
    uint32_t p_align;  // Segment alignment constraint
  };

  struct Elf64_Phdr
  {
    uint32_t p_type;    // Type of segment
    uint32_t p_flags;   // Segment flags
    uint64_t p_offset;   // File offset where segment is located, in bytes
    uint64_t p_vaddr;   // Virtual address of beginning of segment
    uint64_t p_paddr;   // Physical addr of beginning of segment (OS-specific)
    uint64_t p_filesz; // Num. of bytes in file image of segment (may be zero)
    uint64_t p_memsz;  // Num. of bytes in mem image of segment (may be zero)
    uint64_t p_align;  // Segment alignment constraint
  };

  struct Elf32_Shdr
  {
    uint32_t sh_name;      // Section name (index into string table)
    uint32_t sh_type;      // Section type (SHT_*)
    uint32_t sh_flags;     // Section flags (SHF_*)
    uint32_t sh_addr;      // Address where section is to be loaded
    uint32_t sh_offset;     // File offset of section data, in bytes
    uint32_t sh_size;      // Size of section, in bytes
    uint32_t sh_link;      // Section type-specific header table index link
    uint32_t sh_info;      // Section type-specific extra information
    uint32_t sh_addralign; // Section address alignment
    uint32_t sh_entsize;   // Size of records contained within the section
  };

  struct Elf64_Shdr
  {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
  };

  // Symbol bindings.
  enum {
    STB_LOCAL = 0,  // Local symbol, not visible outside obj file containing def
    STB_GLOBAL = 1, // Global symbol, visible to all object files being combined
    STB_WEAK = 2,   // Weak symbol, like global but lower-precedence
    STB_GNU_UNIQUE = 10,
    STB_LOOS = 10,   // Lowest operating system-specific binding type
    STB_HIOS = 12,   // Highest operating system-specific binding type
    STB_LOPROC = 13, // Lowest processor-specific binding type
    STB_HIPROC = 15  // Highest processor-specific binding type
  };

  // Symbol types.
  enum {
    STT_NOTYPE = 0,     // Symbol's type is not specified
    STT_OBJECT = 1,     // Symbol is a data object (variable, array, etc.)
    STT_FUNC = 2,       // Symbol is executable code (function, etc.)
    STT_SECTION = 3,    // Symbol refers to a section
    STT_FILE = 4,       // Local, absolute symbol that refers to a file
    STT_COMMON = 5,     // An uninitialized common block
    STT_TLS = 6,        // Thread local data object
    STT_GNU_IFUNC = 10, // GNU indirect function
    STT_LOOS = 10,      // Lowest operating system-specific symbol type
    STT_HIOS = 12,      // Highest operating system-specific symbol type
    STT_LOPROC = 13,    // Lowest processor-specific symbol type
    STT_HIPROC = 15,    // Highest processor-specific symbol type

    // AMDGPU symbol types
    STT_AMDGPU_HSA_KERNEL = 10
  };

  struct Elf32_Sym
  {
    uint32_t st_name;     // Symbol name (index into string table)
    uint32_t st_value;    // Value or address associated with the symbol
    uint32_t st_size;     // Size of the symbol
    unsigned char st_info;  // Symbol's type and binding attributes
    unsigned char st_other; // Must be zero; reserved
    uint16_t st_shndx;    // Which section (header table index) it's defined in

    unsigned char getBinding() const { return st_info >> 4; }
    unsigned char getType() const { return st_info & 0x0f; }
  };

  struct Elf64_Sym
  {
    uint32_t st_name;     // Symbol name (index into string table)
    unsigned char st_info;  // Symbol's type and binding attributes
    unsigned char st_other; // Must be zero; reserved
    uint16_t st_shndx;    // Which section (header tbl index) it's defined in
    uint64_t st_value;    // Value or address associated with the symbol
    uint64_t st_size;    // Size of the symbol

    unsigned char getBinding() const { return st_info >> 4; }
    unsigned char getType() const { return st_info & 0x0f; }
  };

} // namespace ELF

#pragma pack(pop)