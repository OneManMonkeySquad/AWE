workspace "AWE"
   architecture "x86_64"
   configurations { "Debug", "Release" }
   vectorextensions "SSE2"
   rtti "Off"
   startproject "game"

include "deps"
include "fabrik"
include "game"