# unit test of PostProfile and PostTargets (lines of the config file, name templates): built and run by tests/run_tests.py
QT       += core
QT       -= gui
CONFIG   += console c++11
CONFIG   -= app_bundle
TARGET    = post_profile_test
TEMPLATE  = app
INCLUDEPATH += ../../src/utils
SOURCES  += post_profile_test.cpp ../../src/utils/PostProfile.cpp
HEADERS  += ../../src/utils/PostProfile.h
