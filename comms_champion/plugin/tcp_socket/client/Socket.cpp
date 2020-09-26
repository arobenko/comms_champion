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

#include <cassert>

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtNetwork/QHostAddress>
CC_ENABLE_WARNINGS()

#include "Socket.h"

namespace comms_champion
{

namespace plugin
{

namespace tcp_socket
{

namespace client
{

namespace
{

const QString FromPropName("tcp.from");
const QString ToPropName("tcp.to");

}  // namespace


Socket::Socket()
{
    connect(
        &m_socket, SIGNAL(disconnected()),
        this, SLOT(socketDisconnected()));
    connect(
        &m_socket, SIGNAL(readyRead()),
        this, SLOT(readFromSocket()));
    connect(
        &m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(socketErrorOccurred(QAbstractSocket::SocketError)));
}

Socket::~Socket() noexcept
{
    m_socket.blockSignals(true);
}

bool Socket::socketConnectImpl()
{
    if ((m_socket.state() == QTcpSocket::ConnectedState) ||
        (m_socket.state() == QTcpSocket::ConnectingState)) {
        static constexpr bool Already_connected = false;
        static_cast<void>(Already_connected);
        assert(Already_connected); 
        static const QString AlreadyConnectedError(
            tr("TCP/IP Client is already connected or trying to connect."));
        reportError(AlreadyConnectedError);
        return false;
    }

    if (m_host.isEmpty()) {
        m_host = QHostAddress(QHostAddress::LocalHost).toString();
    }

    m_socket.connectToHost(m_host, m_port);
    if (!m_socket.waitForConnected(1000)) {
        return false;
    }
    return true;
}

void Socket::socketDisconnectImpl()
{
    m_socket.blockSignals(true);
    m_socket.flush();
    m_socket.disconnectFromHost();
    m_socket.close();
    m_socket.blockSignals(false);
}

void Socket::sendDataImpl(DataInfoPtr dataPtr)
{
    assert(dataPtr);
    m_socket.write(
        reinterpret_cast<const char*>(&dataPtr->m_data[0]),
        static_cast<qint64>(dataPtr->m_data.size()));

    QString from =
        m_socket.localAddress().toString() + ':' +
                    QString("%1").arg(m_socket.localPort());
    QString to =
        m_socket.peerAddress().toString() + ':' +
                    QString("%1").arg(m_socket.peerPort());


    dataPtr->m_extraProperties.insert(FromPropName, from);
    dataPtr->m_extraProperties.insert(ToPropName, to);

}

void Socket::socketDisconnected()
{
//    static const QString DisconnectedError(
//        tr("Connection to TCP/IP Server was disconnected."));
//    reportError(DisconnectedError);

    reportDisconnected();
}

void Socket::readFromSocket()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    assert(socket != nullptr);

    auto dataPtr = makeDataInfo();
    dataPtr->m_timestamp = DataInfo::TimestampClock::now();

    auto dataSize = socket->bytesAvailable();
    dataPtr->m_data.resize(static_cast<std::size_t>(dataSize));
    auto result =
        socket->read(reinterpret_cast<char*>(&dataPtr->m_data[0]), dataSize);
    if (result != dataSize) {
        dataPtr->m_data.resize(static_cast<std::size_t>(result));
    }

    QString from =
        m_socket.peerAddress().toString() + ':' +
                    QString("%1").arg(m_socket.peerPort());
    QString to =
        m_socket.localAddress().toString() + ':' +
                    QString("%1").arg(m_socket.localPort());

    dataPtr->m_extraProperties.insert(FromPropName, from);
    dataPtr->m_extraProperties.insert(ToPropName, to);
    reportDataReceived(std::move(dataPtr));
}

void Socket::socketErrorOccurred(QAbstractSocket::SocketError err)
{
    static_cast<void>(err);
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    assert(socket != nullptr);

    reportError(socket->errorString());

    if (socket->state() != QTcpSocket::ConnectedState) {
        reportDisconnected();
    }
}

}  // namespace client

}  // namespace tcp_socket

} // namespace plugin

} // namespace comms_champion
