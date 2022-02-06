MODEL      ?= german-base.m
MODEL_NAME := $(basename $(MODEL))
CFILE      := _$(MODEL_NAME).c
OUTFILE    := _$(MODEL_NAME)

.PHONY: run build clean
run: build
	./$(OUTFILE)

build: $(MODEL)
	rumur $(MODEL) --verbose --output $(CFILE)
	gcc -march=native -O3 $(CFILE) -lpthread -o $(OUTFILE)

clean:
	rm -rf _*
