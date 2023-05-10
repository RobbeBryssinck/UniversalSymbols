project "ElfProcessor"
   kind "StaticLib"
   language "C++"

   files {"**.h", "**.cpp", "**.inl"}

   includedirs 
   {
      "../",
      "../../Vendor/spdlog/include",
      "../../Libraries/RECore"
   }

   libdirs
   {
      "../../Build/Bin/%{cfg.longname}"
   }

   links "UniversalSymbolsFormat"
   links "RECore"