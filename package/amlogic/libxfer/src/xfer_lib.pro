QT       -= gui

TARGET = xfer

TEMPLATE = lib

HEADERS += include/transfer_manager.h \
    include/transfer_def.h \
    include/xfer_common.h \
    include/xfer_error.h \
    include/ixml.h \
    src/common/listop.h \
    src/common/llist.h \
    src/common/xfer_debug.h \
    src/common/transfer_ctrl.h \
    src/common/transfer_utils.h \
    src/http/http_manager.h \
    src/http/http_curl.h \
    src/thunder/thunder_manager.h \
    src/thunder/embed_thunder.h \
    ixml/ixmlmembuf.h \
    ixml/ixmlparser.h \
    ixml/xml_util.h 

SOURCES += src/common/transfer_manager.c \
    src/common/transfer_ctrl.c \
    src/common/transfer_utils.c \
    src/http/http_manager.c \
    src/http/http_curl.c \
    src/http/http_demo.c \
    src/thunder/thunder_manager.c \
    ixml/attr.c \
    ixml/document.c \
    ixml/element.c \
    ixml/ixml.c \
    ixml/ixmlmembuf.c \
    ixml/ixmlparser.c \
    ixml/namedNodeMap.c \
    ixml/node.c \
    ixml/nodeList.c \
    ixml/xml_util.c 


INCLUDEPATH += include
LIBPATH += lib
LIBS += -lcurldll -lws2_32
