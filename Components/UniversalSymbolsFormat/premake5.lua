project "UniversalSymbolsFormat"
   kind "StaticLib"
   language "C++"

   files {"**.h", "**.cpp", "**.inl"}

   includedirs 
   {
      "../../Vendor/spdlog/include",
      "../../Libraries/RECore"
   }

   links "RECore"