//
// Copyright 2015 - 2020 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "SocketPlugin.h"

#include <memory>
#include <cassert>

#include "Socket.h"
#include "SocketConfigWidget.h"

namespace comms_champion
{

namespace plugin
{

namespace tcp_socket
{

namespace server
{

namespace
{

const QString MainConfigKey("cc_tcp_server_socket");
const QString PortSubKey("port");

}  // namespace

SocketPlugin::SocketPlugin()
{
    pluginProperties()
        .setSocketCreateFunc(
            [this]()
            {
                createSocketIfNeeded();
                return m_socket;
            })
        .setConfigWidgetCreateFunc(
            [this]()
            {
                createSocketIfNeeded();
                return new SocketConfigWidget(*m_socket);
            });
}

SocketPlugin::~SocketPlugin() noexcept = default;

void SocketPlugin::getCurrentConfigImpl(QVariantMap& config)
{
    createSocketIfNeeded();

    QVariantMap subConfig;
    subConfig.insert(PortSubKey, QVariant::fromValue(m_socket->getPort()));
    config.insert(MainConfigKey, QVariant::fromValue(subConfig));
}

void SocketPlugin::reconfigureImpl(const QVariantMap& config)
{
    auto subConfigVar = config.value(MainConfigKey);
    if ((!subConfigVar.isValid()) || (!subConfigVar.canConvert<QVariantMap>())) {
        return;
    }

    typedef Socket::PortType PortType;
    auto subConfig = subConfigVar.value<QVariantMap>();
    auto portVar = subConfig.value(PortSubKey);
    if ((!portVar.isValid()) || (!portVar.canConvert<PortType>())) {
        return;
    }

    auto port = portVar.value<PortType>();

    createSocketIfNeeded();

    m_socket->setPort(port);
}

void SocketPlugin::createSocketIfNeeded()
{
    if (!m_socket) {
        m_socket.reset(new Socket());
    }
}

}  // namespace server

}  // namespace tcp_socket

}  // namespace plugin

}  // namespace comms_champion


