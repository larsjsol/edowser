######################################################################
# Automatically generated by qmake (2.01a) Sat Jun 22 22:07:54 2013
######################################################################

#TEMPLATE = app
TARGET = edembed
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../manymouse
LIBS += -lX11 -lXi

# Input
HEADERS += edembed.h xmouse.h 
SOURCES += edembed.cpp xmouse.cpp

# manymouse
HEADERS += ../manymouse/*.h 
SOURCES += ../manymouse/*.c
# ../manymouse/x11_xinput2.c ../manymouse/linux_evdev.c

# qtbrowserplugin
include(../qt-solutions/qtbrowserplugin/src/qtbrowserplugin.pri)
 
 
