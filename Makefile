CFLAGS=-s
LDFLAGS=-lX11 -lXi

penxortouch: penxortouch.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f penxortouch *.o
