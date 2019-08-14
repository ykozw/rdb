require("premake", ">=5.0.0-alpha14")

solution "rdb"
   location "build"
   configurations { "Debug", "Release" }
   platforms {"x64"}

project "rdb"
   kind "ConsoleApp"
   language "C++"
   characterset "MBCS"
   files {
     "viewer.cpp",
     "thirdparty/imgui/examples/imgui_impl_glfw.cpp",
     "thirdparty/imgui/examples/imgui_impl_opengl2.cpp",
     "thirdparty/imgui/imgui.cpp",
     "thirdparty/imgui/imgui_draw.cpp",
     "thirdparty/imgui/imgui_widgets.cpp",
     "thirdparty/imgui/imgui_demo.cpp"
   }
   sysincludedirs{
     "thirdparty/glfw-3.3.bin.WIN64/include",
     "thirdparty/imgui/",
     "thirdparty/imgui/examples",
     "thirdparty/glm/",
     "thirdparty/glew-2.1.0/include",
   }
   libdirs {
     "thirdparty/glfw-3.3.bin.WIN64/lib-vc2019",
     "thirdparty/glew-2.1.0/lib/Release/x64",
   }
   links {
     "glew32.lib",
     "glu32.lib",
     "glfw3.lib",
   }
   filter "configurations:Release"
     optimize "Speed"

project "test"
   dependson  "rdb"
   kind "ConsoleApp"
   language "C++"
   characterset "MBCS"
   files {
     "test.cpp",
   }
   libdirs { "./build/bin/x64/Debug" }

   filter "configurations:Release"
     optimize "Speed"
