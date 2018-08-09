CFLAGS = -I /usr/include/NVCtrl
LIBS   = -lXNVCtrl -ldl -lX11

all:nvtop.elf

nvtop.elf:nvtop.c
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.elf



