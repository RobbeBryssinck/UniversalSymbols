project "UniversalSymbolsFormat"
   kind "StaticLib"
   language "C++"

   files {"**.h", "**.cpp", "**.inl"}

   includedirs 
   {
      "../../Vendor/spdlog/include",
      "../../Vendor/json",
      "../../Libraries/RECore"
   }

   links "RECore"