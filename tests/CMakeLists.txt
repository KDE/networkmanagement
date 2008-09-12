include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/storage
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs
   )

## next target: test for ConfigXml to make sure it can handle our connections

set(testconfigxml_SRCS
   testconfigxml.cpp
)

kde4_add_executable(testconfigxml ${testconfigxml_SRCS})
target_link_libraries(testconfigxml knmstorage ${KDE4_KDEUI_LIBS})


