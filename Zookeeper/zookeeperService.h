
#ifndef NS_ZOOKEEPER_SERVICE_H_
#define NS_ZOOKEEPER_SERVICE_H_

#include "glo_def.h"

#include <zookeeper/zookeeper.h>
#include <string>
#include <set>

class CZooKeeperService
{
public :
	BOOL zks_init(const std::string& ip, u16 port);
    void zks_close();
	BOOL zks_get_tts(std::string& ip, u16& port);
	static void watcher(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx);
	static CZooKeeperService& GetInstance();
	void set_service_parameters(const std::string& ttsNodePath, const std::string& ttsIP, u16 ttsPort);
	void TTSNodeChildrenWatcher(zhandle_t *zh, const char* path);
protected :
	CZooKeeperService();
	BOOL removeNode(const std::string& path);
	
private :
	zhandle_t* handle_;
	static BOOL connected_;
	const i32 ZK_MAX_CONNECTED_TIMES;
    std::string m_ttsNodePath;
    std::string m_tts_serviceIP;
    u16 m_tts_servicePort;
	static std::set<std::string> invaild_tts_node_set;
	static pthread_mutex_t m_invaild_tts_nodes_Lock;
};


#endif

