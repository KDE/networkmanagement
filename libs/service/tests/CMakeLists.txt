include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../../internals
   )

set(test_connectionlist_SRCS
   test_connectionlist.cpp
   )

kde4_add_unit_test(test_connectionlist TESTNAME list-knm-service ${test_connectionlist_SRCS})

target_link_libraries(test_connectionlist
   ${KDE4_KDECORE_LIBS}
   ${QT_QTTEST_LIBRARY}
   knminternals
   knmservice
   )

set(test_connectionlistpersistence_SRCS
   test_connectionlistpersistence.cpp
   )

kde4_add_unit_test(test_connectionlistpersistence TESTNAME listpersistence-knm-service ${test_connectionlistpersistence_SRCS})

target_link_libraries(test_connectionlistpersistence
   ${KDE4_KDECORE_LIBS}
   ${QT_QTTEST_LIBRARY}
   knminternals
   knmservice
   )


