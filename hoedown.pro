#
# Hoedown library
#
# Github : https://github.com/hoedown/hoedown
#
QT       -= core gui

TARGET = hoedown

TEMPLATE = lib
DEF_FILE = hoedown.def

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
