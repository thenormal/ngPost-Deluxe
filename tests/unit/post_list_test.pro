# unit test of PostList (the posts of the session as the user sees them): built and run by tests/run_tests.py
QT       += core
QT       -= gui
CONFIG   += console c++11
CONFIG   -= app_bundle
TARGET    = post_list_test
TEMPLATE  = app
INCLUDEPATH += ../../src/utils
SOURCES  += post_list_test.cpp ../../src/utils/PostList.cpp
HEADERS  += ../../src/utils/PostList.h
