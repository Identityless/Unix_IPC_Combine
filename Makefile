server: server.c
	gcc shserver.c -o shserver -pthread && gcc shclient.c -o shclient -pthread && gcc pipe.c -o pipe -pthread && gcc mqserver.c message.o -o mqserver -pthread && gcc mqclient.c message.o -o mqclient -pthread

clean:
	rm -f *.o shserver shclient pipe mqclient mqserver
