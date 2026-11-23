# unit test of QueueOrder (order of the posts that wait) and of the order of PostList: built and run by tests/run_tests.py
QT       += core
QT       -= gui
CONFIG   += console c++11
CONFIG   -= app_bundle
TARGET    = queue_order_test
TEMPLATE  = app
INCLUDEPATH += ../../src/utils
SOURCES  += queue_order_test.cpp ../../src/utils/QueueOrder.cpp ../../src/utils/PostList.cpp
HEADERS  += ../../src/utils/QueueOrder.h ../../src/utils/PostList.h
