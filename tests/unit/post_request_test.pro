# unit test of PostRequest (what the user wants to post, Guided Post): built and run by tests/run_tests.py
QT       += core
QT       -= gui
CONFIG   += console c++11
CONFIG   -= app_bundle
TARGET    = post_request_test
TEMPLATE  = app
INCLUDEPATH += ../../src/utils
SOURCES  += post_request_test.cpp ../../src/utils/PostRequest.cpp ../../src/utils/PostProfile.cpp
HEADERS  += ../../src/utils/PostRequest.h ../../src/utils/PostProfile.h
