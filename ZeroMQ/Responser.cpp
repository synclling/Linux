#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define MAX_BUFFER_LEN 1024

int main (void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:9000");
    if (rc != 0)
    {
        printf ("Error occurred during zmq_bind(): %s\n", zmq_strerror (errno));
        return -1;
    }

    int count = 0;

    while (1) 
    {
        char buffer [MAX_BUFFER_LEN] = {0};
        zmq_recv (responder, buffer, 10, 0);
        printf ("%s.\n", buffer);
        sleep (1);
        zmq_send (responder, "World", 5, 0);
        printf("send world to client with count=%d.\n", count);
        count ++;
    }

    return 0;
}
