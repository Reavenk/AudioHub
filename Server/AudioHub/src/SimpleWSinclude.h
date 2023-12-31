#pragma once

// Common typing for Simple Websocket client stuff

#include "vendored/simplews/server_ws.hpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

typedef std::shared_ptr<WsServer::Connection> WSConPtr;
typedef std::shared_ptr<WsServer::InMessage> WSInMsgPtr;