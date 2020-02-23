TEMPLATE = subdirs
SUBDIRS += lib \
    tests \
    src

CONFIG += ordered

tests.depends = lib
src.depends = lib
