#
# Hoedown library
#
# Github : https://github.com/hoedown/hoedown
#
QT       -= core gui

TARGET = hoedown

TEMPLATE = lib

SOURCES += \
    src/autolink.c \
    src/buffer.c \
    src/escape.c \
    src/html.c \
    src/html_blocks.c \
    src/html_smartypants.c \
    src/markdown.c \
    src/stack.c

HEADERS += \
    src/autolink.h \
    src/buffer.h \
    src/escape.h \
    src/html.h \
    src/markdown.h \
    src/stack.h

unix:!symbian {
    # install header files
    header_files.files = $$HEADERS
    header_files.path = /usr/include/hoedown

    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target header_files
}
