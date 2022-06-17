CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
LDLIBS = -lpthread

EXUTABLE=proj2
PACK=$(EXUTABLE).zip

proj2: proj2.o

.PHONY:all

clean:
	rm -f $(EXUTABLE) *.o *.out$(PACK)
