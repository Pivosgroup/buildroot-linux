# -------------------------------------------------
# Project created by QtCreator 2010-04-15T17:13:15
# -------------------------------------------------
QT -= gui
TARGET = bttv
TEMPLATE = lib

SOURCES += bttv2.cpp \
    search_parse.c \
    navigate_parse.c \
    kankan_parse.c
HEADERS += bttv2.h \
    bttv2_global.h \
    Xfer_msg.h \
    transfer_utils.h \
    transfer_manager.h \
    transfer_def.h \
    search_parse.h \
    ref_update_manager.h \
    netdownload_comm.h \
    Net_msg.h \
    navigate_parse.h \
    movie_channel_xml.h \
    listop.h \
    kankan_parse.h \
    kankan_channel_xml.h \
    embed_thunder.h \
    BttvApp_cnt.h \
    bttv2_global.h \
    bttv2.h \
    auto_update_manager.h \
    XmlThread.h \
    MBttvObject.h

#////////////////common code //////////////
SOURCES += common/src/md5.c \
    common/src/avos.c \
    common/src/listop.c \
    common/src/xml/src/nodeList.c \
    common/src/xml/src/node.c \
    common/src/xml/src/namedNodeMap.c \
    common/src/xml/src/ixmlparser.c \
    common/src/xml/src/ixmlmembuf.c \
    common/src/xml/src/ixml.c \
    common/src/xml/src/element.c \
    common/src/xml/src/document.c \
    common/src/xml/src/attr.c \
    common/src/xml/src/xml_util.c \
    common/src/q_sem.cpp
INCPATH += "common/include/net/ABoxBase"
INCPATH += "common/include"
INCPATH += "common/include/xml/src/inc"
INCPATH += "common/include/xml/inc"
INCPATH += "common/include/Unicode"
INCPATH += "common/include/Unicode/inc"
INCPATH += $$system(echo $SYSLIB)/../include/
HEADERS += common/include/includes.h \
    common/include/typedefine.h \
    common/include/md5.h \
    common/include/AVmalloc.h \
    common/include/net/ABoxBase/abx_error.h \
    common/include/avos.h \
    common/include/listop.h \
    common/include/net/ABoxBase/abx_mem.h \
    common/include/net/ABoxBase/abx_common.h \
    common/include/net/ABoxBase/abx_thread.h \
    common/include/af_engine.h \
    common/include/xml/inc/ixml.h \
    common/include/xml/inc/xml_util.h \
    common/include/xml/src/inc/ixmlparser.h \
    common/include/xml/src/inc/ixmlmembuf.h \
    common/include/endian.h \
    common/include/aw_windows.h \
    common/include/q_sem.h \
    common/include/ioapi.h
#////////////////common code //////////////
#OTHER_FILES +=
#INCPATH += "../common/include"
#INCPATH += "../common/include/xml/src/inc"
#INCPATH += "../common/include/xml/inc"
#INCPATH += "../common/include/net/ABoxBase"
#INCPATH += "./include/Unicode"
#LIBPATH += /mnt/nfsroot/share/rootfs-0624/jiademei/lib
LIBPATH += $$system(echo $SYSLIB)
#LIBPATH += "/home/jeffrey.jin/Workspace/linux_build/ui_ref/trunk/bld_7266_h_64x2/./rootfs/lib"
#INCLUDEPATH += /mnt/nfsroot/kui.zhang/rootfs-0602/include
#LIBS += -lcommon
LIBS += -lcurl
LIBS += -lxfer

#DESTDIR = ../test/debug
DESTDIR =  /mnt/nfsroot/share/rootfs-0624/app_gadmei_kui/lib
