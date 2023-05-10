workspace "UniversalSymbols"
   location "Generated"

   architecture "x64"
   configurations { "Debug", "Release" }

   cppdialect "C++20"

   defines { "NOMINMAX" }

   filter { "configurations:Debug" }
      defines { "DEBUG" }
      symbols "On"

   filter { "configurations:Release" }
      defines { "NDEBUG" }
      optimize "On"

   filter { }

   targetdir ("Build/Bin/%{cfg.longname}")
   objdir ("Build/Obj/%{cfg.longname}")

include("UniversalSymbolsFormat")
include("PdbToUni")
include("DwarfToUni")