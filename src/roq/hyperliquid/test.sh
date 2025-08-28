#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="hyperliquid"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-hyperliquid/$CONFIG.toml"

URI="api.hyperliquid-testnet.xyz"

REST_URI="https://$URI"
WS_PUBLIC_URI="wss://$URI/ws"
WS_PRIVATE_URI="wss://$URI/ws"

$PREFIX ./roq-hyperliquid \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink true \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --rest_uri "$REST_URI" \
  --ws_public_uri "$WS_PUBLIC_URI" \
  --ws_private_uri "$WS_PRIVATE_URI" \
  $@
