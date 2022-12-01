server: shserver.c
	gcc shserver.c -o shserver -pthread && gcc shclient.c -o shclient -pthread && gcc pipe.c -o pipe -pthread && gcc mqserver.c -o mqserver -pthread && gcc mqclient.c -o mqclient -pthread

clean:
	rm -f *.o shserver shclient pipe mqclient mqserver
