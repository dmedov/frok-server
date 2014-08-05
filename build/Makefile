include makefile.consts

TARGET := FaceDetection

all: build

build:
	@echo $(BRIGHT)$(WHITE)"[BUILD]        "$(RESET) $(TARGET)
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokServerApp.mk build
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokTestApp.mk build
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokAgentApp.mk build
	@echo $(BRIGHT)$(GREEN)"[BUILD IS DONE]"$(RESET) $(BRIGHT)$(WHITE)$(TARGET)$(RESET)

clean:
	@echo $(BRIGHT)$(WHITE)"[CLEAN]        "$(RESET) $(TARGET)
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokTestApp.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokServerApp.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokAgentApp.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FaceDetectionLib.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FaceCommonLib.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FaceRecognitionLib.mk clean
	@$(MAKE) --no-print-directory CCFLAG="$(CFLAGS)" LDFLAG="$(LDFLAGS)" -f FrokAPILib.mk clean
	@echo $(BRIGHT)$(GREEN)"[CLEAN IS DONE]"$(RESET) $(BRIGHT)$(WHITE)$(TARGET)$(RESET)

rebuild: clean build

help:
	@echo "[USAGE] Build FaceDetection in debug configuration: \"make build\" or \"make CFG=debug build\""
	@echo "[USAGE] Clean FaceDetection in debug configuration: \"make clean\" or \"make CFG=debug clean\""
	@echo "[USAGE] Rebuild FaceDetection in debug configuration: \"make rebuild\" or \"make CFG=debug rebuild\""
	@echo "[USAGE] Build FaceDetection in release configuration: \"make CFG=release build\""
