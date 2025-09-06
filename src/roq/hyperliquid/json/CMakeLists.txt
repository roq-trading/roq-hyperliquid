set(TARGET_NAME ${PROJECT_NAME}-json)

set(SOURCES map.cpp parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    active_asset_ctx_data_ctx.json
    active_asset_ctx_data.json
    active_asset_ctx.json
    bbo_data_bbo_item.json
    bbo_data.json
    bbo.json
    channel.json
    error.json
    l2_book_data.json
    l2_book_data_levels_item_item.json
    l2_book_data_levels_item.json
    l2_book.json
    message.json
    meta.json
    meta_margin_tables_item.json
    meta_margin_tables_item_margin_table.json
    meta_universe_item.json
    perp_dexs_item.json
    perp_dexs.json
    pong.json
    side.json
    spot_meta.json
    spot_meta_tokens_item.json
    spot_meta_universe_item.json
    subscription_response_data.json
    subscription_response_data_subscription.json
    subscription_response.json
    trades_data_item.json
    trades.json)

roq_autogen_hpp(
  OUTPUT
  AUTOGEN_HEADERS
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen_cpp(
  OUTPUT
  AUTOGEN_SOURCES
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME} ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library(${TARGET_NAME} OBJECT ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies(${TARGET_NAME} autogen-json-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-codec::roq-codec roq-utils::roq-utils roq-logging::roq-logging fmt::fmt)
