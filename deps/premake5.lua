project "ImGui"
	kind "StaticLib"
	language "C++"
	systemversion "latest"
	cppdialect "C++17"

	targetdir "%{wks.location}/lib"
    objdir "%{wks.location}/obj"

	files {
		"imgui/imconfig.h",
		"imgui/imgui.h",
		"imgui/imgui.cpp",
		"imgui/imgui_draw.cpp",
		"imgui/imgui_internal.h",
		"imgui/imgui_tables.cpp",
		"imgui/imgui_widgets.cpp",
		"imgui/imstb_rectpack.h",
		"imgui/imstb_textedit.h",
		"imgui/imstb_truetype.h",
		"imgui/imgui_demo.cpp"
	}

	defines "ALLEGRO_STATICLINK"

	filter "system:linux"
		pic "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"



project "ImGuiAllegro5Backend"
		kind "StaticLib"
		language "C++"
	
		targetdir "%{wks.location}/lib"
		objdir "%{wks.location}/obj"

		files {
			"imgui/backends/imgui_impl_allegro5.cpp",
			"imgui/backends/imgui_impl_allegro5.h"
		}
	
		includedirs {
			"imgui",
			"%{wks.location}/deps/allegro5/AllegroDeps.1.10.0.0/build/native/include",
			"%{wks.location}/deps/allegro5/Allegro.5.2.5.2/build/native/include"
		}
		
		defines "ALLEGRO_STATICLINK"

		systemversion "latest"
		cppdialect "C++17"


		filter "system:linux"
			pic "On"
	
		filter "configurations:Debug"
			runtime "Debug"
			symbols "on"
	
		filter "configurations:Release"
			runtime "Release"
			optimize "on"



project "psd_sdk"
	kind "StaticLib"
	language "C++"

	targetdir "%{wks.location}/lib"
	objdir "%{wks.location}/obj"
	
	files {
		"psd_sdk/src/Psd/**.h",
		"psd_sdk/src/Psd/**.cpp",
		"psd_sdk/src/Psd/**.c"
	}
	removefiles {
		"psd_sdk/src/Psd/PsdNativeFile_Linux.cpp",
		"psd_sdk/src/Psd/PsdNativeFile_Linux.h",
		"psd_sdk/src/Psd/PsdNativeFile_Mac.h"
	}

	--pchheader "PsdPch.h"
	--pchsource "psd_sdk/src/Psd/PsdPch.cpp"

	defines { "WIN32","_LIB" }
	cppdialect "C++17"
	
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"



project "tracy_client"
	kind "StaticLib"
	language "C++"

	targetdir "%{wks.location}/lib"
	objdir "%{wks.location}/obj"

	files {
		"tracy/TracyClient.cpp"
	}

	defines "TRACY_ENABLE"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"



project "glfw"
	kind "StaticLib"
	language "C++"

	targetdir "%{wks.location}/lib"
	objdir "%{wks.location}/obj"

	files {
		"glfw/include/GLFW/glfw3.h",
		"glfw/include/GLFW/glfw3native.h",
		"glfw/src/glfw_config.h",
		"glfw/src/context.c",
		"glfw/src/init.c",
		"glfw/src/input.c",
		"glfw/src/monitor.c",
		"glfw/src/vulkan.c",
		"glfw/src/window.c"
	}

	filter "system:windows"
		files {
			"glfw/src/win32_init.c",
			"glfw/src/win32_joystick.c",
			"glfw/src/win32_monitor.c",
			"glfw/src/win32_time.c",
			"glfw/src/win32_thread.c",
			"glfw/src/win32_window.c",
			"glfw/src/wgl_context.c",
			"glfw/src/egl_context.c",
			"glfw/src/osmesa_context.c"
		}

		defines {
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
    	defines { "DEBUG" }
    	symbols "On"

   	filter "configurations:Release"
    	defines { "NDEBUG" }
    	optimize "On"


project "yojimbo"
	kind "StaticLib"

	targetdir "%{wks.location}/lib"
	objdir "%{wks.location}/obj"

	files { 
		"yojimbo/yojimbo.h", 
		"yojimbo/yojimbo.cpp", 
		"yojimbo/certs.h", 
		"yojimbo/certs.c", 
		"yojimbo/tlsf/tlsf.h", 
		"yojimbo/tlsf/tlsf.c", 
		"yojimbo/netcode.io/netcode.c", 
		"yojimbo/netcode.io/netcode.h", 
		"yojimbo/reliable.io/reliable.c", 
		"yojimbo/reliable.io/reliable.h"
	}
	includedirs {
		"yojimbo",
		"yojimbo/windows",
		"yojimbo/netcode.io",
		"yojimbo/reliable.io"
	}
	libdirs { "yojimbo/windows" }

	filter "configurations:Debug"
		defines { "YOJIMBO_DEBUG", "NETCODE_DEBUG", "RELIABLE_DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "YOJIMBO_RELEASE", "NETCODE_RELEASE", "RELIABLE_RELEASE" }
		optimize "On"
