#include "ElfInterface.h"

#include "ELF.h"

#include <spdlog/spdlog.h>
#include <vector>
#include <Reader.h>

namespace ElfInterface
{
	struct Elf_EhdrEx : public ELF::Elf64_Ehdr
	{
		void Parse(Reader& aReader, bool aIs64Bit)
		{
			if (aIs64Bit)
				Convert<ELF::Elf64_Ehdr>(aReader);
			else
				Convert<ELF::Elf32_Ehdr>(aReader);
		}

	private:
		template <class T>
		void Convert(Reader& aReader)
		{
			T elfHeader{};
			aReader.Read(elfHeader);

			e_type = elfHeader.e_type;
			e_machine = elfHeader.e_machine;
			e_version = elfHeader.e_version;
			e_entry = elfHeader.e_entry;
			e_phoff = elfHeader.e_phoff;
			e_shoff = elfHeader.e_shoff;
			e_flags = elfHeader.e_flags;
			e_ehsize = elfHeader.e_ehsize;
			e_phentsize = elfHeader.e_phentsize;
			e_phnum = elfHeader.e_phnum;
			e_shentsize = elfHeader.e_shentsize;
			e_shnum = elfHeader.e_shnum;
			e_shstrndx = elfHeader.e_shstrndx;
		}
	};

	struct Elf_PhdrEx : public ELF::Elf64_Phdr
	{
		void Parse(Reader& aReader, bool aIs64Bit)
		{
			if (aIs64Bit)
				Convert<ELF::Elf64_Phdr>(aReader);
			else
				Convert<ELF::Elf32_Phdr>(aReader);
		}

	private:
		template <class T>
		void Convert(Reader& aReader)
		{
			T programHeader{};
			aReader.Read(programHeader);

			p_type = programHeader.p_type;
			p_offset = programHeader.p_offset;
			p_vaddr = programHeader.p_vaddr;
			p_paddr = programHeader.p_paddr;
			p_filesz = programHeader.p_filesz;
			p_memsz = programHeader.p_memsz;
			p_flags = programHeader.p_flags;
			p_align = programHeader.p_align;
		}
	};

	struct Elf_ShdrEx : public ELF::Elf64_Shdr
	{
		void Parse(Reader& aReader, bool aIs64Bit)
		{
			if (aIs64Bit)
				Convert<ELF::Elf64_Shdr>(aReader);
			else
				Convert<ELF::Elf32_Shdr>(aReader);
		}

	private:
		template <class T>
		void Convert(Reader& aReader)
		{
			T sectionHeader{};
			aReader.Read(sectionHeader);
			
			sh_name = sectionHeader.sh_name;
			sh_type = sectionHeader.sh_type;
			sh_flags = sectionHeader.sh_flags;
			sh_addr = sectionHeader.sh_addr;
			sh_offset = sectionHeader.sh_offset;
			sh_size = sectionHeader.sh_size;
			sh_link = sectionHeader.sh_link;
			sh_info = sectionHeader.sh_info;
			sh_addralign = sectionHeader.sh_addralign;
			sh_entsize = sectionHeader.sh_entsize;
		}

	public:
		std::string name{};
	};

	struct Elf_SymEx : public ELF::Elf64_Sym
	{
		void Parse(Reader& aReader, bool aIs64Bit)
		{
			if (aIs64Bit)
				Convert<ELF::Elf64_Sym>(aReader);
			else
				Convert<ELF::Elf32_Sym>(aReader);
		}

		bool IsFunctionSymbol() const
		{
			return getType() == ELF::STT_FUNC;
		}

		bool IsTypeSymbol() const
		{
			// TODO: this isn't good, check what type enum is used for user defined types.
			return getType() != ELF::STT_FUNC;
		}
		
	private:
		template <class T>
		void Convert(Reader& aReader)
		{
			T symbol{};
			aReader.Read(symbol);

			st_name = symbol.st_name;
			st_info = symbol.st_info;
			st_other = symbol.st_other;
			st_shndx = symbol.st_shndx;
			st_value = symbol.st_value;
			st_size = symbol.st_size;
		}

	public:
		std::string name{};
	};

	struct ElfEx
	{
		bool Parse(const char* apFileName)
		{
			Reader reader{};
			if (!reader.LoadFromFile(apFileName))
				return false;

			ReadFileClass(reader);
			ReadElfProgramHeader(reader);
			ReadSectionHeaders(reader);
			ReadSymbols(reader);

			return true;
		}

	private:
		void ReadFileClass(Reader& aReader)
		{
			aReader.position = 4;

			uint8_t fileClass = 0;
			aReader.Read(fileClass);
			aReader.Reset();
			
			is64Bit = fileClass == ELF::ELFCLASS64;
		}

		void ReadElfProgramHeader(Reader& aReader)
		{
			elfHeader.Parse(aReader, is64Bit);

			// TODO: this set position probably shouldn't be necessary,
			// since the program header should come right after the ELF header
			aReader.position = elfHeader.e_phoff;
			programHeader.Parse(aReader, is64Bit);
		}

		void ReadSectionHeaders(Reader& aReader)
		{
			sections.clear();

			aReader.position = elfHeader.e_shoff;
			sections.reserve(elfHeader.e_shnum);
			
			for (size_t i = 0; i < elfHeader.e_shnum; i++)
			{
				// TODO: advance the reader one section entry? since the first one is null.
				// also, reserve - 1. same for symbols. skip by e_shentsize
				auto& section = sections.emplace_back();
				section.Parse(aReader, is64Bit);
			}
		}

		void ReadSymbols(Reader& aReader)
		{
			symbols.clear();

			size_t symtabOffset = 0;
			size_t symtabSize = 0;

			for (auto& section : sections)
			{
				section.name = GetSectionName(aReader, section);
				if (section.name == ".symtab")
				{
					symtabOffset = section.sh_offset;
					symtabSize = section.sh_size;
				}
			}

			if (symtabSize == 0)
				return;
			
			aReader.position = symtabOffset;
			size_t typeSize = is64Bit ? sizeof(ELF::Elf64_Sym) : sizeof(ELF::Elf32_Sym);
			const size_t count = symtabSize / typeSize;

			// TODO: skip first symbol entry, which is null (ala sections)?
			symbols.reserve(count);
			for (size_t i = 0; i < count; i++)
			{
				ELF::Elf64_Sym& symbol = symbols.emplace_back();
				aReader.Read(symbol);
			}

			for (const auto& section : sections)
			{
				if (section.name == ".strtab")
				{
					for (auto& symbol : symbols)
					{
						if (symbol.st_name != 0)
						{
							aReader.position = section.sh_offset + symbol.st_name;
							symbol.name = aReader.ReadString();
						}
					}

					break;
				}
			}
		}

		std::string GetSectionName(Reader& aReader, const Elf_ShdrEx& aSection)
		{
			if (elfHeader.e_shstrndx == 0)
				return "";

			const auto& stringSection = sections[elfHeader.e_shstrndx];

			size_t oldPosition = aReader.position;
			aReader.position = stringSection.sh_offset + aSection.sh_name;
			
			std::string name = aReader.ReadString();
			
			aReader.position = oldPosition;

			return name;
		}

	public:
		bool is64Bit;
		Elf_EhdrEx elfHeader{};
		Elf_PhdrEx programHeader{};
		std::vector<Elf_ShdrEx> sections{};
		std::vector<Elf_SymEx> symbols{};
	};

	static ElfEx s_elf{};

	std::optional<USYM> CreateUsymFromFile(const char* apFileName)
	{
		USYM usym{};

		if (!s_elf.Parse(apFileName))
			return std::nullopt;

		for (const auto& symbol : s_elf.symbols)
		{
			spdlog::info("{}", symbol.name);
			if (symbol.IsTypeSymbol())
			{
				// TODO: do type processing
			}
			else if (symbol.IsFunctionSymbol())
			{
				// TODO: do function processing
			}
		}

		return usym;
	}
}
