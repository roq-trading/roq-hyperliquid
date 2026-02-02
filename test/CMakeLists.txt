include(RoqAutogen)

set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES
    crypto.cpp
    json_active_asset_ctx.cpp
    json_bbo.cpp
    json_error.cpp
    json_get_clearing_house_state_ack.cpp
    json_get_meta_ack.cpp
    json_get_perp_dexs_ack.cpp
    json_get_spot_clearing_house_state_ack.cpp
    json_get_spot_meta_ack.cpp
    json_l2_book.cpp
    json_pong.cpp
    json_spot_meta.cpp
    json_subscription_response.cpp
    json_trades.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-crypto ${PROJECT_NAME}-json ${PROJECT_NAME}-tools Catch2::Catch2)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
