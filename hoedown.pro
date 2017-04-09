# Hoedown
# Github: https://github.com/hoedown/hoedown.git

QT -= core gui

TARGET = hoedown

TEMPLATE = lib
DEF_FILE = hoedown.def

CONFIG += warn_off
CONFIG += staticlib

SOURCES += src/autolink.c \
    src/buffer.c \
    src/escape.c \
    src/html.c \
    src/html_blocks.c \
    src/html_smartypants.c \
    src/document.c \
    src/stack.c \
    src/version.c

HEADERS += src/autolink.h \
    src/buffer.h \
    src/escape.h \
    src/html.h \
    src/document.h \
    src/stack.h \
    src/version.h

unix:!macx {
    isEmpty(PREFIX): PREFIX = /usr
    target.path = $${PREFIX}/lib
    INSTALLS += target
    message("VNote hoedown will be installed in prefix $${PREFIX}")
}
