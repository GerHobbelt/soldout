# Hoedown
# Github: https://github.com/hoedown/hoedown.git

QT -= core gui

TARGET = hoedown

TEMPLATE = lib

CONFIG += warn_off

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
