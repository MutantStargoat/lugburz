LDFLAGS = -lpng -lz

png2bpl: main.o image.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) main.o image.o png2bpl
