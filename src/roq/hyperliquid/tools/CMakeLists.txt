set(TARGET_NAME ${PROJECT_NAME}-tools)

set(SOURCES bignum.cpp crypto.cpp keccak256.cpp key.cpp point.cpp wallet.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

include(RoqAutogen)

target_link_libraries(${TARGET_NAME} PRIVATE roq-utils::roq-utils fmt::fmt OpenSSL::SSL)
