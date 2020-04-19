#include "zmq.hpp"
#include <iostream>
#include <sstream>

int main (int argc, char *argv[])
{
    zmq::context_t context (1);

    //  Socket to talk to server
    std::cout << "Collecting updates from weather server…\n" << std::endl;
    zmq::socket_t subscriber (context, ZMQ_SUB);
    const char *host1 = (argc > 1)? argv[1]:"localhost:9002";
    char endpoint[128];
    snprintf(endpoint, 128, "%s%s", "tcp://", host1);
    subscriber.connect(endpoint);

#if 1

    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

#else

    if (argc < 2){
        subscriber.setsockopt(ZMQ_SUBSCRIBE, "order ", strlen("order "));
    } else {
        for (int i = 2; i < argc; i++){
            char topic[128] = {0};
            snprintf(topic, 128, "%s ", argv[i]);
            subscriber.setsockopt(ZMQ_SUBSCRIBE, topic, strlen(topic));
        }
    }
 #endif

    const char* host2 = (argc > 2)?argv[2]:"localhost:9003";
    char endpoint2[128] = {0};
    snprintf(endpoint2, 128, "%s%s", "tcp://", host2);
    zmq::socket_t request(context, ZMQ_REQ);
    request.connect(endpoint2);
    zmq::message_t msg;
    request.send("ready", strlen("ready "), 0);
    request.recv(&msg);
    std::cout << (char*)msg.data() << std::endl;

    std::cout << "try to recv publisher msg" << std::endl;   
 
    for (int i = 0; i < 100; i ++) {
        zmq::message_t msg;
        subscriber.recv(&msg);
        std::cout << "recv is : "<<(char*)msg.data() <<std::endl;
    }

    // unsubscribe
    subscriber.setsockopt(ZMQ_UNSUBSCRIBE, "order ", strlen("order "));
    
    for (int i = 0; i < 100; i ++) {
        zmq::message_t msg;
        subscriber.recv(&msg);
        std::cout << "recv is : "<<(char*)msg.data() <<std::endl;
    }

    return 0;
}
