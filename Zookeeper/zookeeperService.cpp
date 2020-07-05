#include "zookeeperService.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include "LogUtils.h"

BOOL CZooKeeperService::connected_ = FALSE;
std::set<std::string> CZooKeeperService::invaild_tts_node_set;
pthread_mutex_t CZooKeeperService::m_invaild_tts_nodes_Lock = PTHREAD_MUTEX_INITIALIZER;


CZooKeeperService::CZooKeeperService() : handle_(NULL), ZK_MAX_CONNECTED_TIMES(5)
{

}

CZooKeeperService& CZooKeeperService::GetInstance()
{
	static CZooKeeperService ins;
	return ins;
}

BOOL CZooKeeperService::zks_init(const std::string& ip, u16 port)
{
    std::stringstream ss;
	ss << ip << ":" << port;
	i32 timeout = 300000;
	zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);

	i32 count = 0;
	do {
        handle_ = zookeeper_init(ss.str().c_str(), watcher, timeout, 0, this, 0);
		count++;
		sleep(1);
	}while((!connected_) && (count < ZK_MAX_CONNECTED_TIMES));

	if (count >= ZK_MAX_CONNECTED_TIMES)
	{
        LOG_WARN("CZooKeeperService : init zoo keeper with host %s failed.", ss.str().c_str());
		return FALSE;
	}
	else
	{
        LOG_WARN("CZooKeeperService : init zoo keeper with host %s success.", ss.str().c_str());
	}

	return TRUE;
}

void CZooKeeperService::watcher(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx)
{
    CZooKeeperService* ctx = (CZooKeeperService*)watcherCtx;
    if (state == ZOO_CONNECTED_STATE) 
	{  
	    ctx->connected_ = TRUE;  
	    LOG_INFO("CZooKeeperService : watcher recved ZOO_CONNECTED_STATE notification.");  
	} 
	else if (state == ZOO_AUTH_FAILED_STATE) 
	{
	    ctx->connected_ = FALSE;
	    LOG_INFO("CZooKeeperService : watcher recved ZOO_AUTH_FAILED_STATE notification.");  
	} 
	else if (state == ZOO_EXPIRED_SESSION_STATE) 
	{
	    ctx->connected_ = FALSE;
	    LOG_INFO("CZooKeeperService : watcher recved ZOO_EXPIRED_SESSION_STATE notification.");  
	} 
	else if (state == ZOO_CONNECTING_STATE) 
	{  
	    ctx->connected_ = FALSE;
	    LOG_INFO("CZooKeeperService : watcher recved ZOO_CONNECTING_STATE notification.");	
	}
	else if (state == ZOO_ASSOCIATING_STATE) 
	{  
	    ctx->connected_ = FALSE;
	    LOG_INFO("CZooKeeperService : watcher recved ZOO_ASSOCIATING_STATE notification.");  
	}
}

void CZooKeeperService::zks_close()
{
	 /* 在ZK服务器上删除tts 服务器节点 */
	std::stringstream path;
	path << m_ttsNodePath  << "/" << m_tts_serviceIP << ":" << m_tts_servicePort;
	
	removeNode(path.str());
	
    if (handle_ != NULL)
    {
	    int ret = zookeeper_close(handle_);
	    if ( ZOK == ret )
	    {
            LOG_INFO("CZooKeeperService : close zoo keeper seccess.");
	    }
	    else
	    {
            LOG_WARN("CZooKeeperService : close zoo keeper failed with result %d", ret);
	    }
		handle_ = NULL;
    }

	connected_ = FALSE;
}

BOOL CZooKeeperService::removeNode(const std::string& path)
{
	if (NULL == handle_)
		return TRUE;
	
	i32 ret = zoo_delete(handle_, path.c_str(), -1);
	if ( ZOK == ret )
	{
        LOG_INFO("CZooKeeperService : remove tts node from zookeeper success with path %s", path.c_str());
		return TRUE;
	}
	else
	{
        LOG_WARN("CZooKeeperService : remove tts node from zookeeper failed with path %s.", path.c_str());
		return FALSE;
	}
}

void CZooKeeperService::set_service_parameters(const std::string& ttsNodePath, const std::string& ttsIP, u16 ttsPort)
{
    m_ttsNodePath = ttsNodePath;
    m_tts_serviceIP = ttsIP;
    m_tts_servicePort = ttsPort;
}

BOOL CZooKeeperService::zks_get_tts(std::string& ip, u16& port)
{
    if (NULL == handle_)
	{
	    LOG_WARN("CZooKeeperService : get speech service failed since zk handle is null.");
        return FALSE;
	}
	String_vector ttsnodes;
	i32 ret = zoo_get_children(handle_, m_ttsNodePath.c_str(), 1, &ttsnodes);
	if ( ZOK != ret )
	{
        LOG_WARN("CZooKeeperService : get tts nodes failed with result %d.", ret);
		return FALSE;
	}
	
	if (ttsnodes.count <= 0)
	{
	    LOG_ERROR("CZooKeeperService : path %s has no any node.", m_ttsNodePath.c_str());
        return FALSE;
	}
	String_vector valid_tts_nodes = {0, NULL};
	bool newed_flag = false;
	if (0 == invaild_tts_node_set.size())
	{
		valid_tts_nodes = ttsnodes;
	}
	else
	{
		valid_tts_nodes.data = new char*[ttsnodes.count];
		newed_flag = true;
		for(int i = 0; i < ttsnodes.count; ++i)
		{

			if (invaild_tts_node_set.end() == invaild_tts_node_set.find(ttsnodes.data[i])) // only not in invaild_tts_node_set can be add to vaild_tts_nodes
			{
				++valid_tts_nodes.count;
				valid_tts_nodes.data[i] = ttsnodes.data[i];
			}
		}
	}

	if (!valid_tts_nodes.count)
	{
		LOG_WARN("CZooKeeperService : no valid tts nodes exist on zookeeper service!");
		return FALSE;
	}
	/* 产生一个0 ~ ttsnodes.count的随机数 */
	srand(time(NULL));
    i32 index = rand()%valid_tts_nodes.count;

	/* 所有的节点名称信息是ip:port格式存储的 */
    char strip[32] = {0};
	char strport[32] = {0};
	char* colon = strchr(valid_tts_nodes.data[index], ':');
	if (!colon)
	{
        LOG_WARN("CZooKeeperService : tts nodes name format is invalid.");
		return FALSE;
	}

	strncpy(strip, valid_tts_nodes.data[index], colon - valid_tts_nodes.data[index]);
	strcpy(strport, colon + 1);

	if(newed_flag)
		delete [] valid_tts_nodes.data;
	
    ip = std::string(strip);
	port = atoi(strport);

	LOG_DEBUG("CZooKeeperService : get tts ip:port is %s:%d", ip.c_str(), port);
	
    return TRUE;
}

void CZooKeeperService::TTSNodeChildrenWatcher(zhandle_t *zh, const char* path)
{
	LOG_INFO("ZOO_CHILD_EVENT occurred. watched path:%s", path);

	String_vector ttsnodes;
	i32 ret = zoo_get_children(zh, path, 1, &ttsnodes);
	if ( ZOK != ret )
	{
	    LOG_WARN("CZooKeeperService : [watcher] get tts nodes failed with result %d.", ret);
		return ;
	}

	if (ttsnodes.count <= 0)
	{
	    LOG_ERROR("CZooKeeperService : [watcher]path %s has no any node.", path);
		return ;
	}

	std::set<std::string> valid_tts_nodes;
	for(int i = 0; i < ttsnodes.count; ++i)
		valid_tts_nodes.insert(ttsnodes.data[i]);

	std::vector<std::string> set_intersection_res(invaild_tts_node_set.size());

	std::vector<std::string>::iterator it;
  	it = std::set_intersection(valid_tts_nodes.begin(), valid_tts_nodes.end(), invaild_tts_node_set.begin(), invaild_tts_node_set.end(), set_intersection_res.begin());
  	set_intersection_res.resize(it - set_intersection_res.begin()); 
	
	pthread_mutex_lock(&m_invaild_tts_nodes_Lock); 
	invaild_tts_node_set.clear();
	if (!set_intersection_res.empty())
		invaild_tts_node_set.insert(set_intersection_res.begin(), set_intersection_res.end());
	pthread_mutex_unlock(&m_invaild_tts_nodes_Lock);
}

