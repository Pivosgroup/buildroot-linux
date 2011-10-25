QT       -= gui

TARGET = xfer_test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


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
    tests/xfer_update.h \
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
    ixml/xml_util.c \
    tests/xfer_update.c \
    tests/xfer_test.c 


INCLUDEPATH += include
LIBPATH += lib
LIBS += -lcurldll -lws2_32
#DEFINES += XFER_DEBUG
