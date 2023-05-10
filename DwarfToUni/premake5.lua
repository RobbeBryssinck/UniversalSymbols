group "Apps"
project "DwarfToUni"
   kind "ConsoleApp"
   language "C++"

   files {"**.h", "**.cpp"}

   includedirs 
   {
      "../Components",
      "../Vendor/spdlog/include"
   }

   libdirs
   {
      "../Build/Bin/%{cfg.longname}"
   }

   links "UniversalSymbolsFormat"