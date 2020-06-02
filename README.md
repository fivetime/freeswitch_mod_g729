# freeswitch_mod_g729
## 编译g729
1、将mod_g729.tar.gz解压后复制到下面目录  
cd /usr/local/src/freeswitch-1.10.3/src/mod/codecs/mod_g729/  
2、进入该目录，执行make lib命令  
3、退出到源项目首层，执行make install  
4、查看freeswitch@DevServer> show codec  
出现codec,G.729,mod_g729就算正常了。  
  
  
报错/data/software/freeswitch-1.8.2/libs/sofia-sip/libsofia-sip-ua/tport  
tport_type_sctp.c:206:10: error: variable 'initmsg' has initializer but incomplete type  
修改代码struct sctp_initmsg initmsg = { 0 };为  
```
typedef struct {
        uint16_t sinit_num_ostreams;
        uint16_t sinit_max_instreams;
        uint16_t sinit_max_attempts;
        uint16_t sinit_max_init_timeo;
} sctp_initmsg;
#define SCTP_INITMSG 2
sctp_initmsg initmsg = { 0 };
```
