project "ImGui"
	kind "StaticLib"
	language "C++"

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

	systemversion "latest"
	cppdialect "C++17"
	staticruntime "On"

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
	
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

		defines "ALLEGRO_STATICLINK"

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
	--pchheader "PsdPch.h"
	--pchsource "psd_sdk/src/Psd/PsdPch.cpp"

	defines { "WIN32","_LIB" }
	staticruntime "On"
	
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

	staticruntime "On"

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

	staticruntime "On"

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
