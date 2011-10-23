include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs
   )

## next target: test for ConfigXml to make sure it can handle our connections

set(testconfigxml_SRCS
   testconfigxml.cpp
)

#kde4_add_executable(testconfigxml ${testconfigxml_SRCS})
#target_link_libraries(testconfigxml knmstorage ${KDE4_KDEUI_LIBS})

## next target: test for new storage layer to make sure it can handle our connections

#set(testnewstorage_SRCS
   #testnewstorage.cpp
#)

#kde4_add_executable(testnewstorage ${testnewstorage_SRCS})
#target_link_libraries(testnewstorage knminternals ${KDE4_KDEUI_LIBS} knm_nm)

## next target: hacked up qdbus that can display NM data types

kde4_add_executable(qdbusfornm qdbusfornm.cpp )
target_link_libraries(qdbusfornm knminternals ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY}
                      ${KDE4_KDECORE_LIBS})

