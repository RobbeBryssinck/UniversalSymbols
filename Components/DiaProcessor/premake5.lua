project "DiaProcessor"
   kind "StaticLib"
   language "C++"

   files {"**.h", "**.cpp", "**.inl"}

   includedirs 
   {
      "../",
      "../../Vendor/spdlog/include",
      "../../Vendor/DIASDK/include",
   }
   
   libdirs
   {
      "../../Build/Bin/%{cfg.longname}",
      "../../Vendor/DIASDK/lib/amd64"
   }

   links "UniversalSymbolsFormat"
   links "diaguids"