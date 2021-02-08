src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = tilegen

CFLAGS = -pedantic -Wall -g -MMD
LDFLAGS = -lGL -lGLU -lglut -lGLEW -lm -ltreestore

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: clean
clean:
	$(RM) $(obj) $(bin)

.PHONY: cleandep
cleandep:
	$(RM) $(dep)
