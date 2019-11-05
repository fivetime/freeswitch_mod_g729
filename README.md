# freeswitch_mod_g729
<br>
编译g729<br>
1、将mod_g729.tar.gz解压后复制到下面目录<br>
cd /usr/local/src/freeswitch-1.4.6/src/mod/codecs/mod_g729/<br>
2、进入该目录，执行make lib命令<br>
3、退出到源项目首层，执行make install<br>
4、查看freeswitch@DevServer> show codec<br>
出现codec,G.729,mod_g729就算正常了。<br>
<br>
<br>
报错/data/software/freeswitch-1.8.2/libs/sofia-sip/libsofia-sip-ua/tport<br>
tport_type_sctp.c:206:10: error: variable 'initmsg' has initializer but incomplete type<br>
修改代码struct sctp_initmsg initmsg  = { 0 };为<br>
typedef struct {<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;uint16_t sinit_num_ostreams;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;uint16_t sinit_max_instreams;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;uint16_t sinit_max_attempts;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;uint16_t sinit_max_init_timeo;<br>
} sctp_initmsg;<br>
#define SCTP_INITMSG    2<br>
sctp_initmsg initmsg  = { 0 };<br>
