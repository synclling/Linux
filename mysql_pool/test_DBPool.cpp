/*
 * @Author: your name
 * @Date: 2020-01-09 11:54:16
 * @LastEditTime : 2020-01-09 21:09:18
 * @LastEditors  : Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \mysql-pool\test_DBPool.cpp
 */
#include <sstream>
#include "DBPool.h"
using namespace std;

static string int2string(uint32_t user_id)
{
    stringstream ss;
    ss << user_id;
    return ss.str();
}

#define DROP_IMUSER_TABLE	"DROP TABLE IF EXISTS IMUser"     /* if EXISTS 好处 是如果表不存在,执行不会报错 */
// #define CREATE_IMUSER_TABLE	"CREATE TABLE user(id int unsigned auto_increment not null,\
//                                                  name varchar(30) not null default '',\
//                                                  email varchar(30) not null default '',\
//                                                  phone varchar(30) not null default '', \
// 												 primary key (id))"

 #define CREATE_IMUSER_TABLE "CREATE TABLE IMUser (     \
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT '用户id',   \
  `sex` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '1男2女0未知', \
  `name` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '用户名',  \
  `domain` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '拼音',  \
  `nick` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '花名,绰号等', \
  `password` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '密码',    \
  `salt` varchar(4) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '混淆码',   \
  `phone` varchar(11) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '手机号码',   \
  `email` varchar(64) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT 'email',  \
  `company` varchar(64) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '公司名称', \
  `address` varchar(64) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '所在地区', \
  `avatar` varchar(255) COLLATE utf8mb4_bin DEFAULT '' COMMENT '自定义用户头像',    \
  `validateMethod` tinyint(2) unsigned DEFAULT '1' COMMENT '好友验证方式',  \
  `departId` int(11) unsigned NOT NULL DEFAULT '1' COMMENT '所属部门Id',    \
  `status` tinyint(2) unsigned DEFAULT '0' COMMENT '1. 试用期 2. 正式 3. 离职 4.实习',  \
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',   \
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',   \
  `push_shield_status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0关闭勿扰 1开启勿扰',  \
  `sign_info` varchar(128) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '个性签名',  \
  PRIMARY KEY (`id`),   \
  KEY `idx_domain` (`domain`),  \
  KEY `idx_name` (`name`),  \
  KEY `idx_phone` (`phone`) \
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;"     

#define INSERT_SAMPLE		"INSERT INTO user(name,email,phone) VALUES(?,?,?)"
#define SELECT_SAMPLE 		"SELECT name,email,phone FROM user"


//1.创建数据库mysql_pool_test:  create database mysql_pool_test;
// show databases;    查看数据库
// show tables;       查看有哪些表
// desc table_name;   查看表结构
// 

static uint32_t IMUser_nId = 0;

bool insertUser(CDBConn* pDBConn)
{
    bool bRet = false;
    string strSql;
    strSql = "insert into IMUser(`salt`,`sex`,`nick`,`password`,`domain`,`name`,`phone`,`email`,`company`,`address`,`avatar`,`sign_info`,`departId`,`status`,`created`,`updated`) values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";

    CPrepareStatement* stmt = new CPrepareStatement();
    if (stmt->Init(pDBConn->GetMysql(), strSql))
    {
        uint32_t nNow = (uint32_t) time(NULL);
        uint32_t index = 0;
        string strOutPass = "987654321";
        string strSalt = "abcd";

        int nSex = 1;// 用户性别 1.男;2.女
        int nStatus = 0; // 用户状态0 正常， 1 离职
        uint32_t nValidateMethod = 1; //好友验证方式
        uint32_t nDeptId = 0;// 所属部门
        string strNick = "明华";// 花名
        string strDomain = "minghua";// 花名拼音
        string strName = "廖庆富";// 真名
        string strTel = "18570368134";// 手机号码
        string strEmail = "326873713@qq.com";// Email
        string strAvatar = "";// 头像
        string sign_info = "一切只为你";//个性签名
        string strPass = "123456"; //密码
        string strCompany = "零声学院"; //公司
        string strAddress = "长沙岳麓区雅阁国际"; //地址

        stmt->SetParam(index++, strSalt);
        stmt->SetParam(index++, nSex);
        stmt->SetParam(index++, strNick);
        stmt->SetParam(index++, strOutPass);
        stmt->SetParam(index++, strDomain);
        stmt->SetParam(index++, strName);
        stmt->SetParam(index++, strTel);
        stmt->SetParam(index++, strEmail);
        stmt->SetParam(index++, strCompany);
        stmt->SetParam(index++, strAddress);
        stmt->SetParam(index++, strAvatar);
        stmt->SetParam(index++, sign_info);
        stmt->SetParam(index++, nDeptId);
        stmt->SetParam(index++, nStatus);
        stmt->SetParam(index++, nNow);
        stmt->SetParam(index++, nNow);
        bRet = stmt->ExecuteUpdate();
        
        if (!bRet)
        {
            printf("insert user failed: %s\n", strSql.c_str());
        }
        else
        {
            IMUser_nId = stmt->GetInsertId();
            printf("register then get user_id:%d\n", IMUser_nId);
        }
    }
    delete stmt;

    return true;
}

bool queryUser(CDBConn* pDBConn)
{
    string strSql = "select * from IMUser where id="+int2string(IMUser_nId);
    CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
    bool bRet = false;
    if(pResultSet)
    {
        if(pResultSet->Next())
        {
            uint32_t nId;//用户ID
            uint8_t nSex;// 用户性别 1.男;2.女
            uint8_t nStatus; // 用户状态0 正常， 1 离职
            uint32_t nValidateMethod; //好友验证方式
            uint32_t nDeptId;// 所属部门
            string strNick;// 花名
            string strDomain;// 花名拼音
            string strName;// 真名
            string strTel;// 手机号码
            string strEmail;// Email
            string strAvatar;// 头像
            string sign_info;//个性签名
            string strPass; //密码
            string strCompany; //公司
            string strAddress; //地址
            nId = pResultSet->GetInt("id");
            nSex = pResultSet->GetInt("sex");
            strNick = pResultSet->GetString("nick");
            strDomain = pResultSet->GetString("domain");
            strName = pResultSet->GetString("name");
            strTel = pResultSet->GetString("phone");
            strEmail = pResultSet->GetString("email");
            strAvatar = pResultSet->GetString("avatar");
            sign_info = pResultSet->GetString("sign_info");
            nDeptId = pResultSet->GetInt("departId");
            nValidateMethod = pResultSet->GetInt("validateMethod");
            nStatus = pResultSet->GetInt("status");

            printf("nId:%u\n", nId);
            printf("nSex:%u\n", nSex);
            printf("strNick:%s\n", strNick.c_str());
            printf("strDomain:%s\n", strDomain.c_str());
            printf("strName:%s\n", strName.c_str());
            printf("strTel:%s\n", strTel.c_str());
            printf("strEmail:%s\n", strEmail.c_str());
            printf("sign_info:%s\n", sign_info.c_str());

            bRet = true;
        }
        delete pResultSet;
    }
    else
    {
        printf("no result set for sql:%s\n", strSql.c_str());
    }

    return bRet;
}

bool updateUser(CDBConn* pDBConn)
{
    bool bRet = false;
    if (pDBConn)
    {
        int nSex = 1;// 用户性别 1.男;2.女
        int nStatus = 0; // 用户状态0 正常， 1 离职
        uint32_t nValidateMethod = 1; //好友验证方式
        uint32_t nDeptId = 0;// 所属部门
        string strNick = "耐寒";// 花名
        string strDomain = "naihan";// 花名拼音
        string strName = "李智勇";// 真名
        string strTel = "18570368134";// 手机号码
        string strEmail = "517609429@qq.com";// Email
        string strAvatar = "";// 头像
        string sign_info = "一切只为你";//个性签名
        string strPass = "123456"; //密码
        string strCompany = "零声学院"; //公司
        string strAddress = "长沙岳麓区雅阁国际"; //地址

        uint32_t nNow = (uint32_t)time(NULL);
        string strSql = "update IMUser set `sex`=" + int2string(nSex)+ ", `nick`='" + strNick
			+"', `domain`='"+ strDomain + "', `name`='" + strName + "', `phone`='" + strTel 
			+ "', `email`='" + strEmail+ "', `avatar`='" + strAvatar + "', `sign_info`='" + sign_info 
			+"', `departId`='" + int2string(nDeptId) + "', `status`=" + int2string(nStatus) + ", `updated`="+int2string(nNow) 
			+", `company`='" + strCompany + "', `address`='" + strAddress + "' where id="+int2string(IMUser_nId);
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            printf("updateUser: update failed:%s\n", strSql.c_str());
        }
    }
    else
    {
        printf("no db connection!\n");
    }
    return bRet;
}

void testOneConnect()
{
    char *db_pool_name = "mypool";
    char* db_host = "127.0.0.1";
    int   db_port =  3306;
    char* db_dbname = "mysql_pool_test";
    char* db_username = "root";
    char* db_password = "123456";
    int db_maxconncnt = 4;
    CDBPool* pDBPool = new CDBPool(db_pool_name, db_host, db_port, 
        db_username, db_password, db_dbname, db_maxconncnt);
    if (pDBPool->Init()) {
        printf("init db instance failed: %s", db_pool_name);
        return;
    }

    CDBConn* pDBConn = pDBPool->GetDBConn();
    if(pDBConn)
    {
        bool ret = pDBConn->ExecuteDrop(DROP_IMUSER_TABLE);
        if(ret)
        {
            printf("DROP_IMUSER_TABLE ok\n");
        }
        //  // 1. 创建表
        // ret = pDBConn->ExecuteCreate(CREATE_IMUSER_TABLE);
        // if(ret)
        // {
        //     printf("CREATE_IMUSER_TABLE ok\n");
        // }

        pDBPool->RelDBConn(pDBConn);
    }
    else
    {
        printf("pDBConn is null\n");
    }
    delete pDBPool;
}
// 它代表创建（Create）、更新（Update）、读取（Retrieve）和删除（Delete）操作。
// 测试增删改查
void testCURD()
{
    char *db_pool_name = "mypool";
    char* db_host = "127.0.0.1";
    int   db_port =  3306;
    char* db_dbname = "mysql_pool_test";
    char* db_username = "root";
    char* db_password = "123456";
    int db_maxconncnt = 4;
    CDBPool* pDBPool = new CDBPool(db_pool_name, db_host, db_port, 
        db_username, db_password, db_dbname, db_maxconncnt);
    if (pDBPool->Init()) {
        printf("init db instance failed: %s", db_pool_name);
        return;
    }

    CDBConn* pDBConn = pDBPool->GetDBConn();
    if(pDBConn)
    {
        bool ret = pDBConn->ExecuteDrop(DROP_IMUSER_TABLE);
        if(ret)
        {
            printf("DROP_IMUSER_TABLE ok\n");
        }
        // 1. 创建表
        ret = pDBConn->ExecuteCreate(CREATE_IMUSER_TABLE);
        if(ret)
        {
            printf("CREATE_IMUSER_TABLE ok\n");
        }

        // 2. 插入内容
        ret = insertUser(pDBConn);
        if(ret)
        {
            printf("insertUser ok -------\n\n");
        }
        // 3. 查询内容
        ret = queryUser(pDBConn);
        if(ret)
        {
            printf("queryUser ok -------\n\n");
        }
        // 4. 修改内容
        ret = updateUser(pDBConn);
        if(ret)
        {
            printf("updateUser ok -------\n\n");
        }
        ret = queryUser(pDBConn);
        if(ret)
        {
            printf("queryUser ok -------\n\n");
        }
        // 5. 删除表
        pDBPool->RelDBConn(pDBConn);
    }
    else
    {
        printf("pDBConn is null\n");
    }
    delete pDBPool;
}

// 默认端口 3306
// 测试一次连接和端口的情况： tcpdump -i any port 3306  
int main()
{
    // printf("test testOneConnect begin\n");
    // testOneConnect();
    // printf("test testOneConnect finish\n");

    printf("test testCURD begin\n");
    testCURD();
    printf("test testCURD finish\n");
    return 0;
}