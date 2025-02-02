LOCAL_PATH := $(call my-dir)

CORE_DIR := $(LOCAL_PATH)/..

DEBUG                    := 0
FRONTEND_SUPPORTS_RGB565 := 1
CACHE_CD                 := 0
NEED_BPP                 := 32
WANT_NEW_API             := 1
NEED_STEREO_SOUND        := 1
HAVE_THREADS             := 1
NEED_CD                  := 1
NEED_SCSI_CD             := 1
NEED_TREMOR              := 1
HAVE_CHD                 := 1
IS_X86                   := 0
FLAGS                    :=

ifeq ($(TARGET_ARCH),x86)
  IS_X86 := 1
endif

include $(CORE_DIR)/Makefile.common

COREFLAGS := -funroll-loops $(INCFLAGS) -DMEDNAFEN_VERSION_NUMERIC=926 -D__LIBRETRO__ -D_LOW_ACCURACY_ -DINLINE="inline" $(FLAGS)
COREFLAGS += -DWANT_PCFX_EMU

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

include $(CLEAR_VARS)
LOCAL_MODULE       := retro
LOCAL_SRC_FILES    := $(SOURCES_CXX) $(SOURCES_C)
LOCAL_CFLAGS       := $(COREFLAGS)
LOCAL_CXXFLAGS     := $(COREFLAGS)
LOCAL_LDFLAGS      := -Wl,-version-script=$(CORE_DIR)/link.T
LOCAL_CPP_FEATURES := exceptions
include $(BUILD_SHARED_LIBRARY)
