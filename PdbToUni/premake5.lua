group "Apps"
project "PdbToUni"
   kind "ConsoleApp"
   language "C++"

   files {"**.h", "**.cpp"}

   includedirs 
   {
      "../Components",
      "../Vendor/spdlog/include",
      "../Vendor/DIASDK/include"
   }

   libdirs
   {
      "../Build/Bin/%{cfg.longname}",
      "../Vendor/DIASDK/lib/amd64"
   }

   links "UniversalSymbolsFormat"
   links "diaguids"
   links "RECore"