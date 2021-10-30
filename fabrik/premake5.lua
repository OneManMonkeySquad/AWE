project "fabrik"
    kind "StaticLib"
    language "C++"
    systemversion "latest"
	cppdialect "C++latest"

    targetdir "%{wks.location}/lib"
    objdir "%{wks.location}/obj"

    files {
        "inc/fabrik/**.h",
        "src/**.h",
        "src/**.cpp",
    }

    includedirs {
        "inc/fabrik",
        "%{wks.location}/deps",
        "%{wks.location}/deps/imgui",
        "%{wks.location}/deps/entt/src",
        "%{wks.location}/deps/psd_sdk/src",
        "%{wks.location}/deps/glfw/include",
        "%{wks.location}/deps/allegro5/AllegroDeps.1.10.0.0/build/native/include",
        "%{wks.location}/deps/allegro5/Allegro.5.2.5.2/build/native/include",
        "%{wks.location}/deps/concurrentqueue",
        "%{wks.location}/deps/enet/include",
        "%{wks.location}/deps/cxxopts/include",
        "%{wks.location}/deps/cereal/include",
    }

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    defines "ALLEGRO_STATICLINK"

    filter "system:windows"
        editAndContinue "Off"
        stringpooling "On"

    filter "configurations:Debug"
        runtime "Debug"
        defines { "DEBUG", "TRACY_ENABLE", "TRACY_NO_BROADCAST" }
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        defines { "NDEBUG", "TRACY_ENABLE", "TRACY_NO_BROADCAST" }
        optimize "On"

