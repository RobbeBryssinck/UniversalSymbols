project "benchmark"
    kind "StaticLib"
    language "C++"

    filter {}
        defines { "BENCHMARK_STATIC_DEFINE" }

    filter { }

    files
    {
        "src/**.h",
        "src/**.cc",
        "include/**.h",
    }

    includedirs
    {
        "include",
        "./"
    }