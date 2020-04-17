#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

#define FILE_FREAM_SIZE 1024

struct file_head_t
{
    unsigned int file_path_len;
    char file_path[128];
};

int read_file(char *filename, int pos, char *buffer, int length) 
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) 
    {
	printf("fopen failed\n");
	return -1;
    }

    fseek(fp, pos, SEEK_SET);

    int size = fread(buffer, 1, length, fp);
    if (size != length) 
    {
	printf("read file end\n");
    }

_exit:
    fclose(fp);

    return size;
}

int main(int argc, char** argv) 
{
    if (argc != 2)
    {
	std::cout << "please useage " << argv[0] <<" <file path>" << std::endl;
	return -1;
    }
	
    if (0 != access(argv[1], R_OK))
    {
	std::cout << "input file is not exist "<< argv[1] << std::endl;
	return -1;
    }
	
    zmq::context_t context(1);  // 初始化zmq上下文
    zmq::socket_t socket(context, ZMQ_PUSH);  // 创建ZMQ_REQ类型的套接字
    socket.setsockopt(ZMQ_SNDTIMEO, 0);
    socket.connect("tcp://localhost:8080");

    sleep(1);
    
    file_head_t head;
    memset(&head, 0, sizeof(head));
    head.file_path_len = strlen(argv[1]);
    head.file_path_len = htonl(head.file_path_len);
    strcpy(head.file_path, argv[1]);
    zmq::message_t file_pkt(sizeof(head.file_path_len) + strlen(argv[1]));
    memcpy(file_pkt.data(), &head, sizeof(head.file_path_len) + strlen(argv[1]));
    //socket.send(&file_pkt, 0);
    socket.send(&head.file_path_len, 4, ZMQ_SNDMORE);
    socket.send(&head.file_path, strlen(argv[1]), ZMQ_SNDMORE);

    int rpos = 0;
    while(1)
    {
	    char buffer[FILE_FREAM_SIZE] = {0};
	    int size = read_file(argv[1], rpos, buffer, FILE_FREAM_SIZE);
	    rpos += size;
	    if (size < 0)
	    {
                std::cout << "read file content failed." << std::endl;
	        socket.send(NULL, 0, 0);
	        break;
	    }
	    else if (size == FILE_FREAM_SIZE)
	    {
	        zmq::message_t file_cnt_pkt(buffer, size);
	        socket.send(file_cnt_pkt, ZMQ_SNDMORE);
	    }
	    else if (size < FILE_FREAM_SIZE)
	    {
	        std::cout << "read file content over ." << rpos << " bytes"<< std::endl;
	        zmq::message_t file_cnt_pkt(buffer, size);
	        socket.send(file_cnt_pkt, 0);
	        break;
	    }
    }

    socket.close();
    context.close();

    return 0;
}

