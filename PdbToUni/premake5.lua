project "PdbToUni"
   kind "ConsoleApp"
   language "C++"

   files {"**.h", "**.cpp"}

   includedirs 
   {
      "../UniversalSymbolsFormat",
   }

   libdirs
   {
      "../Build/Bin/%{cfg.longname}"
   }