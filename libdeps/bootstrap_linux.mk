
PACKAGES = rtaudio rtmidi

all: loopdub

loopdub: $(PACKAGES)
	@echo
	@echo
	@echo "Dependancies are all compiled. LoopDub should now be able to compile and link. Type 'scons' to proceed."
	@echo
	@echo

# RtAudio
rtaudio: $(PKG_RTAUDIO_LIB)
	@if [ -f $(PKG_RTAUDIO_LIB) ]; then echo "RtAudio verified."; else echo "Error processing RtAudio."; false; fi

$(PKG_RTAUDIO_LIB): rtaudio.unpacked
#	Replace the RtAudio build system with Scons.
#	This script can be pointed to the dsound header.
	echo "StaticLibrary('rtaudio', ['RtAudio.cpp'], CCFLAGS='-D__LINUX_ALSA__')" >$(PKG_RTAUDIO_DIR)/SConstruct
	cd $(PKG_RTAUDIO_DIR); scons

rtaudio.unpacked: rtaudio.verified
	@echo "Unpacking RtAudio..."
	tar -xzf $(PKG_RTAUDIO_TAR)
	touch rtaudio.unpacked

rtaudio.verified:
	@echo "Getting RtAudio..."
	wget $(PKG_RTAUDIO_URL) -O $(PKG_RTAUDIO_TAR)
	if [ `md5sum $(PKG_RTAUDIO_TAR) | cut -d" " -f1`x == $(PKG_RTAUDIO_MD5)x ]; then touch rtaudio.verified; else echo "MD5SUM error on $(PKG_RTAUDIO_TAR)"; false; fi


# RtMidi
rtmidi: $(PKG_RTMIDI_LIB)
	@if [ -f $(PKG_RTMIDI_LIB) ]; then echo "RtMidi verified."; else echo "Error processing RtMidi."; false; fi

$(PKG_RTMIDI_LIB): rtmidi.unpacked
#	Replace the RtMidi build system with Scons.
	echo "StaticLibrary('rtmidi', ['RtMidi.cpp'])" >$(PKG_RTMIDI_DIR)/SConstruct
	cd $(PKG_RTMIDI_DIR); scons

rtmidi.unpacked: rtmidi.verified
	@echo "Unpacking RtMidi..."
	tar -xzf $(PKG_RTMIDI_TAR)
	touch rtmidi.unpacked

rtmidi.verified:
	@echo "Getting RtMidi..."
	wget $(PKG_RTMIDI_URL) -O $(PKG_RTMIDI_TAR)
	if [ `md5sum $(PKG_RTMIDI_TAR) | cut -d" " -f1`x == $(PKG_RTMIDI_MD5)x ]; then touch rtmidi.verified; else echo "MD5SUM error on $(PKG_RTMIDI_TAR)"; false; fi