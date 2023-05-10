group("Tests")
project "UniversalSymbolsFormat_Tests"
   kind "ConsoleApp"
   language "C++"

   files {"**.h", "**.cpp", "../main.cpp"}

   includedirs
   {
      "../../Components/UniversalSymbolsFormat",
      "../../Vendor/googletest/include"
   }

   libdirs
   {
      "../Build/Bin/%{cfg.longname}"
   }

   links "googletest"
   links "UniversalSymbolsFormat"