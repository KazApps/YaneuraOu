CXX=clang++

EXE = YaneuraOu-by-gcc

ifeq ($(strip $(EVALFILE)),)
    EVAL_EMBEDDING=OFF
	EVALFILE=
else
    EVAL_EMBEDDING=ON
endif

.PHONY: build

build:
	$(MAKE) -C source \
		-j \
		tournament \
		COMPILER=$(CXX) \
		TARGET=$(abspath $(EXE)) \
		TARGET_CPU=NATIVE \
		YANEURAOU_EDITION=YANEURAOU_ENGINE_NNUE \
		EVAL_EMBEDDING=$(EVAL_EMBEDDING) \
		EVALFILE=$(abspath $(EVALFILE))
