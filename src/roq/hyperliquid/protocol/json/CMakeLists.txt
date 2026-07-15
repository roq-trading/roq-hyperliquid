set(TARGET_NAME ${PROJECT_NAME}-protocol-json)

set(SOURCES cancel_order_ack_parser.cpp create_order_ack_parser.cpp map.cpp parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    action_cancel.json
    action_cancel_data.json
    action_cancel_data_response.json
    action_cancel_data_response_payload.json
    action_cancel_data_response_payload_response.json
    action_cancel_data_response_payload_response_data.json
    action_order.json
    action_order_data.json
    action_order_data_response.json
    action_order_data_response_payload.json
    action_order_data_response_payload_response.json
    action_order_data_response_payload_response_data.json
    action_order_data_response_payload_response_data_status.json
    action_order_data_response_payload_response_data_status_resting.json
    active_asset_ctx.json
    active_asset_ctx_data.json
    active_asset_ctx_data_ctx.json
    bbo.json
    bbo_data.json
    bbo_data_bbo_item.json
    cancel_order_ack.json
    cancel_order_ack_response.json
    cancel_order_ack_response_data.json
    cancel_order_ack_response_data_statuses_item.json
    channel.json
    create_order_ack.json
    create_order_ack_response.json
    create_order_ack_response_data.json
    create_order_ack_response_data_statuses_item.json
    create_order_ack_response_data_statuses_item_filled.json
    create_order_ack_response_data_statuses_item_resting.json
    direction.json
    error.json
    general_error.json
    get_clearing_house_state_ack.json
    get_clearing_house_state_ack_cross_margin_summary.json
    get_clearing_house_state_ack_margin_summary.json
    get_meta_ack.json
    get_meta_ack_margin_tables_item.json
    get_meta_ack_margin_tables_item_margin_table.json
    get_meta_ack_universe_item.json
    get_open_orders_ack.json
    get_open_orders_ack_item.json
    get_perp_dexs_ack.json
    get_perp_dexs_ack_item.json
    get_spot_clearing_house_state_ack.json
    get_spot_clearing_house_state_ack_balances_item.json
    get_spot_meta_ack.json
    get_spot_meta_ack_tokens_item.json
    get_spot_meta_ack_universe_item.json
    get_user_fills_ack.json
    get_user_fills_ack_item.json
    get_user_fills_ack_item_liquidation.json
    l2_book.json
    l2_book_data.json
    l2_book_data_levels_item.json
    l2_book_data_levels_item_item.json
    modify_order_ack.json
    modify_order_ack.json
    modify_order_ack_response.json
    modify_order_ack_response_data.json
    notification.json
    notification_data_item.json
    order_status.json
    order_updates.json
    order_updates_data_item.json
    order_updates_data_item_order.json
    payload_type.json
    pong.json
    response_type.json
    side.json
    spot_meta.json
    spot_meta_data.json
    spot_meta_data_response.json
    spot_meta_data_response_payload.json
    spot_meta_data_response_payload_data.json
    status.json
    subscription_response.json
    subscription_response_data.json
    subscription_response_data_subscription.json
    time_in_force.json
    trades.json
    trades_data_item.json
    user.json
    user_data.json
    user_data_fills_item.json
    user_data_funding.json
    user_fills.json
    user_fills_data.json
    user_fills_data_fills_item.json
    user_fundings.json
    user_fundings_data.json
    user_fundings_data_fundings_item.json)

roq_autogen_hpp(
  OUTPUT
  AUTOGEN_HEADERS
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

add_custom_target(protocol-json-autogen-headers ALL DEPENDS ${AUTOGEN_HEADERS})

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

add_dependencies(${TARGET_NAME} protocol-json-autogen-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-codec::roq-codec roq-utils::roq-utils roq-logging::roq-logging fmt::fmt)
