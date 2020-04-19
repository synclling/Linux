#include "zmq.hpp"
#include <string>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

struct file_head_t
{
    unsigned int file_path_len;
    char file_path[128];
};

int main () 
{
    zmq::context_t context (1);  // 创建zmq的上下文
    zmq::socket_t socket (context, ZMQ_PULL); // 创建ZMQ_PULL类型的套接字
    socket.setsockopt(ZMQ_RCVTIMEO, 0);
    socket.bind("tcp://*:8080");   // 设置为tcp类型的通信方式，且绑定8080端口

    zmq::pollitem_t items [] = {
        { socket, 0, ZMQ_POLLIN, 0 }
    };

    while (true) 
    {
	    int rc = zmq::poll (&items[0], 1, -1);
	    if (items[0].revents & ZMQ_POLLIN)
	    {
	        zmq::message_t file_pkt;
	        socket.recv(&file_pkt);
	    		
	        file_head_t fh;
                //1. 文件路径长度
	        memcpy(&fh.file_path_len, file_pkt.data(), sizeof(fh.file_path_len));
	        fh.file_path_len = ntohl(fh.file_path_len);
	        std::cout << "received file path len " << fh.file_path_len << std::endl;
	    	
                //2.文件路径名
	        socket.recv(&file_pkt);
	    	memset(fh.file_path, 0, 128);
	        memcpy(&fh.file_path, file_pkt.data(), fh.file_path_len);
	        std::cout << "received file path " << fh.file_path << std::endl;
	    
            FILE *fp = fopen(fh.file_path, "wb+");
	    	if (NULL == fp)
	    	{
	    		std::cout << "fopen "<< fh.file_path <<" file failed." << std::endl;
	    	}
	        while (1)
	        {
                // loop 3. 接收文件内容
	    	    bool more = file_pkt.more();
	    	    if (more)
	    	    {
	       	        socket.recv(&file_pkt);
	    	        int size = fwrite((char*)file_pkt.data(), 1, file_pkt.size(), fp);
	    		if (size != file_pkt.size())
	    		{
	    		    std::cout << "fwrite file failed." << std::endl;
	    		}
	    	    }
	            else
	    	    {
	    	        break;
	    	    }
	        }

	    	fclose(fp);
	        std::cout << "received file content of file " << fh.file_path << std::endl;
	    }
    }
    
    socket.close();
    context.close();
    
    return 0;
}
