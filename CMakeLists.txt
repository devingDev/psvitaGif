cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

set(SHORT_NAME vitagiftest)
project(${SHORT_NAME})
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Vita Gif Test")
set(VITA_TITLEID  "GIFTEST01")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=gnu11 -fpermissive")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fpermissive")

include_directories(
  ../common
)

add_executable(${SHORT_NAME}
  src/giflib/dgif_lib.c
  src/giflib/egif_lib.c
  src/giflib/gif_err.c
  src/giflib/gif_font.c
  src/giflib/gif_hash.c
  src/giflib/gifalloc.c
  src/giflib/openbsd-reallocarray.c
  src/giflib/quantize.c
  src/main.cpp
  src/Logger.cpp
)

target_link_libraries(${SHORT_NAME}
  vita2d
  SceDisplay_stub
  SceGxm_stub
  ScePgf_stub
  ScePvf_stub
  SceAppUtil_stub
  SceCommonDialog_stub
  SceIme_stub
  freetype
  png
  jpeg
  z
  m
  c
  debugnet
  SceCtrl_stub
  SceSysmodule_stub
  SceNet_stub
  SceNetCtl_stub
  SceHttp_stub
  SceSsl_stub
  SceIofilemgr_stub 
  crypto
  pthread
  
  
)

vita_create_self(${SHORT_NAME}.self ${SHORT_NAME})
vita_create_vpk(${SHORT_NAME}.vpk ${VITA_TITLEID} ${SHORT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  
  
)
