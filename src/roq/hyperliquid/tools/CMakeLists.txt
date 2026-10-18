set(TARGET_NAME ${PROJECT_NAME}-tools)

set(SOURCES conversions.cpp crypto.cpp encoder.cpp pkey.cpp throttle.cpp tick_size_steps.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

include(RoqAutogen)

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-protocol-json roq-web::roq-web roq-utils::roq-utils fmt::fmt OpenSSL::SSL)
