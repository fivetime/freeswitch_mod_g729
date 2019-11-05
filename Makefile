BASE=../../../..
LIB=g729a_v11
LOCAL_INSERT_CFLAGS=echo -I./$(LIB)
LOCAL_INSERT_LDFLAGS=echo ./$(LIB)/libg729.a

include $(BASE)/build/modmake.rules

lib:
	(cd $(LIB) && make)

lib-clean:
	(cd $(LIB) && make clean)
