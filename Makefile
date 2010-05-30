CC=gcc
OBJS=tiles/tiles.o tiles/layer.o tiles/stack.o tiles/tiles_makers.o \
	tiles/stack_composers.o tiles/cairo_util.o confdir/confdir.o main.o
CFLAGS=`pkg-config --cflags gtk+-2.0` -g
LDFLAGS=`pkg-config --libs gtk+-2.0`

.PHONY: clean valgrind valgrind-gen profile

potash: $(OBJS)
	gcc $(LDFLAGS) -o potash $(OBJS)

clean:
	rm -f $(OBJS) potash

valgrind:
	G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --leak-check=full --suppressions=valgrind.sup  ./potash

valgrind-gen:
	G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --leak-check=full --show-reachable=yes --suppressions=valgrind.sup --gen-suppressions=yes  ./potash

profile:
	mkdir profile
	(cd profile ; valgrind --tool=callgrind -v --dump-every-bb=10000000  ../potash)
	(cd profile ; kcachegrind `ls -1 | grep -Ex 'callgrind.out.[0-9]+'`)
	rm -r profile

# dependencies
main.o: tiles/tiles.h tiles/layer.h tiles/stack.h tiles/tiles_makers.h tiles/stack_composers.h confdir/confdir.h
tiles/tiles.o: tiles/tiles.h tiles/tiles_makers.h
tiles/layer.h: tiles/tiles.h
tiles/layer.o: tiles/layer.h tiles/tiles_makers.h
tiles/tiles.h: confdir/confdir.h
tiles/stack.h: tiles/layer.h tiles/tiles.h
tiles/stack.o: tiles/stack.h tiles/layer.h tiles/tiles.h tiles/tiles_makers.h tiles/cairo_util.h
tiles/tiles_makers.h: tiles/tiles.h
tiles/tiles_makers.o: tiles/tiles_makers.h tiles/tiles.h
tiles/stack_composers.o: tiles/stack_composers.h
tiles/cairo_util.o: tiles/cairo_util.h
confdir/confdir.o: confdir/confdir.h


