group("Tests")
project "Performance_Tests"
   kind "ConsoleApp"
   language "C++"


   filter {}
      defines { "BENCHMARK_STATIC_DEFINE" }

   filter { }

   files {"**.h", "**.cpp"}

   includedirs
   {
      "../../Components",
      "../../Vendor/benchmark/include"
   }

   libdirs
   {
      "../Build/Bin/%{cfg.longname}"
   }

   links "benchmark"
   links "Shlwapi"
   links "DiaProcessor"
   links "UniversalSymbolsFormat"