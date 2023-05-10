project "ElfProcessor"
   kind "StaticLib"
   language "C++"

   files {"**.h", "**.cpp", "**.inl"}

   includedirs 
   {
      "../../Vendor/spdlog/include",
      "../UniversalSymbolsFormat",
   }

   links "UniversalSymbolsFormat"