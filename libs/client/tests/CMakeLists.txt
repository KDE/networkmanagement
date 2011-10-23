include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   )

set(test_remoteactivatablelist_SRCS
   test_remoteactivatablelist.cpp
   )

kde4_add_unit_test(test_remoteactivatablelist TESTNAME list-remote-activatables ${test_remoteactivatablelist_SRCS})

target_link_libraries(test_remoteactivatablelist
   ${KDE4_KDECORE_LIBS}
   ${QT_QTTEST_LIBRARY}
   knminternals
   knmclient
   )


