group "Apps"
project "PdbToUni"
   kind "ConsoleApp"
   language "C++"

   files {"**.h", "**.cpp"}

   includedirs 
   {
      "../Components",
      "../Vendor/spdlog/include",
   }

   libdirs
   {
      "../Build/Bin/%{cfg.longname}"
   }

   links "UniversalSymbolsFormat"
   links "RECore"
   links "DiaProcessor"