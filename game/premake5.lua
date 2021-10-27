project "game"
   kind "WindowedApp"
   language "C++"
   systemversion "latest"
   cppdialect "C++latest"

   targetname "game_%{cfg.buildcfg}"

   targetdir "%{wks.location}/bin"
   objdir "%{wks.location}/obj"

   links {
      "secur32",
      "winmm",
      "dmoguids",
      "wmcodecdspuuid",
      "msdmo",
      "Strmiids",
      "OpenGL32",
      "shlwapi",

      "ImGui", "ImGuiAllegro5Backend",

      "psd_sdk",
      
      "OpenGL32", "dumb", "FLAC", "freetype", "jpeg", "libpng16", "ogg", "opus", 
      "opusfile", "physfs", "theoradec", "vorbis", "vorbisfile", "webp",
      "webpdecoder", "webpdemux", "zlib", "allegro_monolith-static",

      "tracy_client",

      "sodium", "mbedtls", "mbedx509", "mbedcrypto", "yojimbo",

      "fabrik"
   }
   libdirs {
      "%{wks.location}/deps/allegro5/AllegroDeps.1.10.0.0/build/native/v142/x64/deps/lib",
      "%{wks.location}/deps/allegro5/Allegro.5.2.5.2/build/native/v142/x64/lib",
      "%{wks.location}/deps/yojimbo/windows"
   }

   files {
      "src/**.h",
      "src/**.cpp"
   }
   includedirs {
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
      "%{wks.location}/fabrik/inc",
   }
   pchheader "pch.h"
   pchsource "src/pch.cpp"

   defines {"ALLEGRO_STATICLINK","CXXOPTS_NO_RTTI"}

   debugdir "%{wks.location}/bin"

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