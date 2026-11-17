# unit test of the comparison of the version numbers (new version notice): built and run by tests/run_tests.py
QT       += core
QT       -= gui
CONFIG   += console c++11
CONFIG   -= app_bundle
TARGET    = version_number_test
TEMPLATE  = app
INCLUDEPATH += ../../src/utils
SOURCES  += version_number_test.cpp
HEADERS  += ../../src/utils/VersionNumber.h
