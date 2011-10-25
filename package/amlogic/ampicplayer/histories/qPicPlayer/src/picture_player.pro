# -------------------------------------------------
# Project created by QtCreator 2010-04-28T14:51:25
# -------------------------------------------------
QT += dbus
TARGET = picture_player
CONFIG += debug
TEMPLATE = app
SOURCES += main.cpp \
    picture_player.cpp \
    picture_player_adapter.cpp \
    cpictransition.cpp \
    qusrgraphicsview.cpp
HEADERS += picture_player.h \
    picture_player_adapter.h \
    cpictransition.h \
    qusrgraphicsview.h
SOURCES += effect/ge2d_osd.c \
    effect/effect_overlap.c \
    effect/effect_blend.c \
	effect/effect_cornercover.c \
	effect/effect_wave.c \
	effect/effect_move.c \
    effect/effect_utils.c
HEADERS += effect/ge2d_osd.h \
    effect/show_effect.h \
    effect/effect_utils.h
RESOURCES += 
