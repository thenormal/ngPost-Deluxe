# unit test of NfoWriter (styles of the NFO file, ASCII version, widths): built and run by tests/run_tests.py
QT       += core
QT       -= gui
CONFIG   += console c++11
CONFIG   -= app_bundle
TARGET    = nfo_writer_test
TEMPLATE  = app
INCLUDEPATH += ../../src/utils
SOURCES  += nfo_writer_test.cpp ../../src/utils/NfoWriter.cpp
HEADERS  += ../../src/utils/NfoWriter.h ../../src/utils/NfoBanners.h
