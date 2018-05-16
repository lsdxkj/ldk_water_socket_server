all:	../build/water_socket_server ../build/demo

../build/water_socket_server:	water_socket_server.c tools.c device.c php_server.c
	gcc -Wall -D_GNU_SOURCE -std=c99 $^ -o $@ -pthread
	scp $@ cb-server:~/receive_files/

../build/demo:	demo.c tools.c
	gcc -g -Wall -D_GNU_SOURCE -std=c11 $^ -o $@

.PHONY:	clean

clean:
	rm -f ../build/water_socket_server ../build/demo