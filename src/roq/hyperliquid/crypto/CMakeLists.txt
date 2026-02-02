set(TARGET_NAME ${PROJECT_NAME}-crypto)

set(SOURCES
    conversions.cpp
    ecdsa.cpp
    eip712.cpp
    exchange.cpp
    keccak.cpp
    signing.cpp
    wallet.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

include(RoqAutogen)

target_link_libraries(${TARGET_NAME} PRIVATE OpenSSL::SSL)
