#-------------------------------------------------
#
# Project created 2016-07-03
#
#-------------------------------------------------

TEMPLATE = lib
TARGET = hoedown

# Use common project definitions
include(../../common.pri)

QT -= core gui widgets

CONFIG -= qt app_bundle
CONFIG += staticlib

# suppress compiler warnings
CONFIG += warn_off

SOURCES += \
    src/autolink.c \
    src/buffer.c \
    src/document.c \
    src/escape.c \
    src/html.c \
    src/html_blocks.c \
    src/html_smartypants.c \
    src/stack.c \
    src/version.c

HEADERS += \
    src/autolink.h \
    src/buffer.h \
    src/document.h \
    src/escape.h \
    src/html.h \
    src/stack.h \
    src/version.h

