ENABLE_QT()

#include_directories(AFTER "gnuboy")
#add_subdirectory(gnuboy)

file(GLOB_RECURSE HDRS *.h)
file(GLOB_RECURSE SRCS *.cpp *.c)
set (SRCS ${HDRS} ${SRCS})

# TODO: put QT Moc files here.
#set (MOC_HDRS mwo_application.h mwo_mainwindow.h)
#QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})

# TODO: put resources here.
#set (RESOURCE_SRCS images.qrc)
#QT4_ADD_RESOURCES(IMAGES_RES ${RESOURCE_SRCS})

add_executable(${OUTPUT_NAME}.oar ${SRCS} ${MOC_SRCS} ${IMAGES_RES})
target_link_libraries(${OUTPUT_NAME}.oar
   ${ADD_LIB}
)
