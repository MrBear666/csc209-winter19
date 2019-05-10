FLAGS = -Wall -g -std=gnu99 
DEPENDENCIES = family.h reading.h

all: wheel

wheel: wheel.o family.o reading.o 
	gcc ${FLAGS} -o $@ $^

%.o: %.c ${DEPENDENCIES}
	gcc ${FLAGS} -c $<

clean: 
	rm -f *.o family wheel reading 
