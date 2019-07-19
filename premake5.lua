require("premake", ">=5.0.0-alpha14")

solution "RDB"
   location "build"
   configurations { "Debug", "Release" }
   platforms {"x64"}

project "RDB"
   kind "SharedLib"
   language "C++"
   characterset "MBCS"
   files {
     "src/rdb.cpp",
   }
   filter "configurations:Release"
     optimize "Speed"

project "RDB Viewer"
   kind "ConsoleApp"
   language "C++"
   characterset "MBCS"
   files {
     "src/viewer.cpp",
   }
   filter "configurations:Release"
     optimize "Speed"

project "test"
   dependson  "RDB"
   kind "ConsoleApp"
   language "C++"
   characterset "MBCS"
   files {
     "src/test.cpp",
   }
   libdirs { "./build/bin/x64/Debug" }

   filter "configurations:Release"
     optimize "Speed"
