require("premake", ">=5.0.0-alpha14")

solution "RDB"
   location "build"
   configurations { "Debug", "Release" }
   platforms {"x64"}

project "RDB Viewer"
   kind "ConsoleApp"
   language "C++"
   characterset "MBCS"
   files {
     "src/viewer.cpp",
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
	 "thirdparty/glm/"
   }
   libdirs {
     "thirdparty/glfw-3.3.bin.WIN64/lib-vc2019"
   }
   links {
     "glfw3.lib"
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
