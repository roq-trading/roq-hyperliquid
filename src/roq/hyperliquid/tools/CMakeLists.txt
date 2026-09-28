set(TARGET_NAME ${PROJECT_NAME}-tools)

set(SOURCES
    bignum.cpp
    conversions.cpp
    crypto.cpp
    ecdsa.cpp
    eip712.cpp
    encoder.cpp
    group.cpp
    key.cpp
    pkey.cpp
    point.cpp
    throttle.cpp
    tick_size_steps.cpp
    wallet.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

include(RoqAutogen)

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-protocol-json roq-web::roq-web roq-utils::roq-utils fmt::fmt OpenSSL::SSL)
