#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_BUFFER_LEN 1024

char* hosts[] = {
//    "tcp://192.168.2.130:9000",
    "tcp://192.168.2.129:9000",
    "tcp://192.168.2.128:9000"

};

int main (void){

    printf ("Connecting to hello world server…\n");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);

    for (int i = 0; i < sizeof(hosts) / sizeof(hosts[0]); i ++){
        if (0 != zmq_connect (requester, hosts[i])) {
            printf("connect server %s failed.\n", hosts[i]);
        }
    }

    for (int i = 0; i < 50; i ++){
        char buffer [MAX_BUFFER_LEN] = {0};
        sprintf(buffer, "hello_%d", i);
        printf ("Send %s to server.\n", buffer);
        zmq_send (requester, buffer, strlen(buffer), 0);
        memset(buffer, 0, MAX_BUFFER_LEN);
        zmq_recv (requester, buffer, 10, 0);
        printf ("Received %s.\n", buffer);
    }

    zmq_close (requester);
    zmq_ctx_destroy (context);
   
    return 0;
}
