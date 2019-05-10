SHELL = /bin/bash
FLAGS = -Wall -std=gnu99 -g
.PHONY: test_life test_trim test_trcount clean

all: life trim trcount

life: life.o life_helpers.o
	gcc ${FLAGS} -o $@ $^

%.o: %.c 
	gcc ${FLAGS} -c $<

trim : trim.c
	gcc ${FLAGS} -o $@ $^ 

trcount : trcount.c
	gcc ${FLAGS} -o $@ $^

test_life: life
	@test_life_output=`./life ......X.......X.......X...... 10 | cmp sample-life-output`; \
	if [ -z "$$test_life_output" ]; then \
		echo Compiled and life sanity check passed; \
	else \
	 	echo Failed life sanity check; \
	fi

test_trim: trim
	@test_trim_output=`./trim sample-full-simple.tr sample-marker-simple | cmp sample-trim-output`; \
	if [ -z "$$test_trim_output" ]; then \
		echo Compiled and trim sanity check passed; \
	else \
	 	echo Failed trim sanity check; \
	fi

test_trcount: trcount
	@test_trcount_output=`./trcount sample-trim-output | cmp sample-trcount-output`; \
	if [ -z "$$test_trcount_output" ]; then \
		echo Compiled and trcount sanity check passed; \
	else \
	 	echo Failed trcount sanity check; \
	fi

clean:
	rm -f *.o life trim trcount
