include makefile.consts

TARGET := FaceDetection

all: build

build:
	@echo $(BRIGHT)$(WHITE)"[BUILD]        "$(RESET) $(TARGET)
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokTestApp.mk build
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokAgentApp.mk build
	@echo $(BRIGHT)$(GREEN)"[BUILD IS DONE]"$(RESET) $(BRIGHT)$(WHITE)$(TARGET)$(RESET)

clean:
	@echo $(BRIGHT)$(WHITE)"[CLEAN]        "$(RESET) $(TARGET)
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokTestApp.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokAgentApp.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FaceDetectionLib.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokLibCommon.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FaceRecognitionLib.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokAPILib.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokJsonlib.mk clean
	@echo $(BRIGHT)$(GREEN)"[CLEAN IS DONE]"$(RESET) $(BRIGHT)$(WHITE)$(TARGET)$(RESET)

rebuild: clean build

help:
	@echo "[USAGE] Build FrokSolution in debug configuration: \"make build\" or \"make CFG=debug build\""
	@echo "[USAGE] Clean FrokSolution in debug configuration: \"make clean\" or \"make CFG=debug clean\""
	@echo "[USAGE] Rebuild FrokSolution in debug configuration: \"make rebuild\" or \"make CFG=debug rebuild\""
	@echo "[USAGE] Build FrokSolution in release configuration: \"make CFG=release build\""
	@echo "[USAGE] Build with custom flags: make CCFLAG+=-DFLAG_NAME build"
	@echo "[USAGE] Flags examples:"
	@echo "[USAGE] NO_DAEMON - switch off daemon mode"
	@echo "[USAGE] FAST_SEARCH_ENABLED - enable fast mode search algorythm"
