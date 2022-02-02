# Description: compile application
# Usage: make

app: glthread notification_chain routing_table threaded_subscriber rtm_publisher
	gcc -g glthread.o notification_chain.o routing_table.o threaded_subscriber.o rtm_publisher.o -o main -lpthread
	make clean

glthread: glthread.c glthread.h
	gcc -g -c glthread.c -o glthread.o

notification_chain: notification_chain.c notification_chain.h
	gcc -g -c notification_chain.c -o notification_chain.o

routing_table: routing_table.c routing_table.h
	gcc -g -c routing_table.c -o routing_table.o

threaded_subscriber: threaded_subscriber.c
	gcc -g -c threaded_subscriber.c -o threaded_subscriber.o

rtm_publisher: rtm_publisher.c
	gcc -g -c rtm_publisher.c -o rtm_publisher.o
	
clean:
	rm -f *.o