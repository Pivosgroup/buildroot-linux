AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libbttv.la

INCLUDES=-I./include

INCLUDES += -I./src/common/include/net/ABoxBase
INCLUDES += -I./src/common/include
INCLUDES += -I./src/common/include/xml/src/inc
INCLUDES += -I./src/common/include/xml/inc
INCLUDES += -I./src/common/include/Unicode
INCLUDES += -I./src/common/include/Unicode/inc

export INCLUDES

#libbttv_la_LDFLAG=
libbttv_la_LIBADD=-lxfer -lQtCore
libbttv_la_SOURCES=./src/bttv2.cpp \
    ./src/moc_MBttvObject.cpp \
    ./src/moc_XmlThread.cpp \
    ./src/search_parse.c \
    ./src/navigate_parse.c \
    ./src/kankan_parse.c \
    ./src/common/src/md5.c \
    ./src/common/src/avos.c \
    ./src/common/src/listop.c \
    ./src/common/src/xml/src/nodeList.c \
    ./src/common/src/xml/src/node.c \
    ./src/common/src/xml/src/namedNodeMap.c \
    ./src/common/src/xml/src/ixmlparser.c \
    ./src/common/src/xml/src/ixmlmembuf.c \
    ./src/common/src/xml/src/ixml.c \
    ./src/common/src/xml/src/element.c \
    ./src/common/src/xml/src/document.c \
    ./src/common/src/xml/src/attr.c \
    ./src/common/src/xml/src/xml_util.c \
    ./src/common/src/q_sem.cpp \
    ./src/bttv2.h \
    ./src/bttv2_global.h \
    ./src/Xfer_msg.h \
    ./src/transfer_utils.h \
    ./src/transfer_manager.h \
    ./src/transfer_def.h \
    ./src/search_parse.h \
    ./src/ref_update_manager.h \
    ./src/netdownload_comm.h \
    ./src/Net_msg.h \
    ./src/navigate_parse.h \
    ./src/movie_channel_xml.h \
    ./src/listop.h \
    ./src/kankan_parse.h \
    ./src/kankan_channel_xml.h \
    ./src/embed_thunder.h \
    ./src/BttvApp_cnt.h \
    ./src/bttv2_global.h \
    ./src/bttv2.h \
    ./src/auto_update_manager.h \
    ./src/XmlThread.h \
    ./src/MBttvObject.h \
    ./src/common/include/includes.h \
    ./src/common/include/typedefine.h \
    ./src/common/include/md5.h \
    ./src/common/include/AVmalloc.h \
    ./src/common/include/net/ABoxBase/abx_error.h \
    ./src/common/include/avos.h \
    ./src/common/include/listop.h \
    ./src/common/include/net/ABoxBase/abx_mem.h \
    ./src/common/include/net/ABoxBase/abx_common.h \
    ./src/common/include/net/ABoxBase/abx_thread.h \
    ./src/common/include/af_engine.h \
    ./src/common/include/xml/inc/ixml.h \
    ./src/common/include/xml/inc/xml_util.h \
    ./src/common/include/xml/src/inc/ixmlparser.h \
    ./src/common/include/xml/src/inc/ixmlmembuf.h \
    ./src/common/include/endian.h \
    ./src/common/include/aw_windows.h \
    ./src/common/include/q_sem.h \
    ./src/common/include/ioapi.h

