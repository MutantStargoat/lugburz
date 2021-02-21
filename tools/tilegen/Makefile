src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = tilegen

warn = -pedantic -Wall -Wno-int-to-pointer-cast

CFLAGS = $(warn) -g -fcommon -MMD
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
