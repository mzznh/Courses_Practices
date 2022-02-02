INC=-I./threadlib -I./threadlib/gluethread

glthread:
	gcc -g -c $(INC) threadlib/gluethread/glthread.c -o threadlib/gluethread/glthread.o

threadlib: glthread
	gcc -g -c $(INC) threadlib/threadlib.c -o threadlib/threadlib.o 

thread_pool_app: glthread threadlib
	gcc -g $(INC) threadlib/gluethread/glthread.o threadlib/threadlib.o Thread_pool_app/thread_pool_app.c -o Thread_pool_app/thread_pool_app -lpthread

thread_barrier_app: glthread threadlib
	gcc -g $(INC) threadlib/gluethread/glthread.o threadlib/threadlib.o Thread_barrier_app/thread_barrier_app.c -o Thread_barrier_app/thread_barrier_app -lpthread

wait_queue_app: glthread threadlib
	gcc -g $(INC) threadlib/gluethread/glthread.o threadlib/threadlib.o Wait_queue_app/traffic_light.c -o Wait_queue_app/traffic_light -lpthread

all: thread_pool_app thread_barrier_app wait_queue_app