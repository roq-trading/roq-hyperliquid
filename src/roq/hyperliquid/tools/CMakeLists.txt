set(TARGET_NAME ${PROJECT_NAME}-tools)

set(SOURCES crypto.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

include(RoqAutogen)

target_link_libraries(${TARGET_NAME} PRIVATE roq-utils::roq-utils fmt::fmt)
