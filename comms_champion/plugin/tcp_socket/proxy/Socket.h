//
// Copyright 2015 - 2021 (C). Alex Robenko. All rights reserved.
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

#pragma once

#include <list>

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
CC_ENABLE_WARNINGS()

#include "comms_champion/Socket.h"


namespace comms_champion
{

namespace plugin
{

namespace tcp_socket
{

namespace proxy
{

class Socket : public QObject,
                     public comms_champion::Socket
{
    Q_OBJECT
    using Base = comms_champion::Socket;

public:
    typedef unsigned short PortType;

    Socket();
    ~Socket() noexcept;

    void setPort(PortType value)
    {
        m_port = value;
    }

    PortType getPort() const
    {
        return m_port;
    }

    void setRemoteHost(const QString& value)
    {
        m_remoteHost = value;
    }

    const QString& getRemoteHost() const
    {
        return m_remoteHost;
    }

    void setRemotePort(PortType value)
    {
        m_remotePort = value;
    }

    PortType getRemotePort() const
    {
        return m_remotePort;
    }

protected:
    virtual bool socketConnectImpl() override;
    virtual void socketDisconnectImpl() override;
    virtual void sendDataImpl(DataInfoPtr dataPtr) override;
    virtual unsigned connectionPropertiesImpl() const override;

private slots:
    void newConnection();
    void clientConnectionTerminated();
    void readFromClientSocket();
    void socketErrorOccurred(QAbstractSocket::SocketError err);
    void acceptErrorOccurred(QAbstractSocket::SocketError err);
    void connectionSocketConnected();
    void connectionSocketDisconnected();
    void readFromConnectionSocket();

private:
    typedef QTcpSocket* ClientSocketPtr;
    typedef std::unique_ptr<QTcpSocket> ConnectionSocketPtr;
    typedef std::pair<ClientSocketPtr, ConnectionSocketPtr> ConnectedPair;
    typedef std::list<ConnectedPair> SocketsList;

    SocketsList::iterator findByClient(QTcpSocket* socket);
    SocketsList::iterator findByConnection(QTcpSocket* socket);
    void removeConnection(SocketsList::iterator iter);
    void performReadWrite(QTcpSocket& readFromSocket, QTcpSocket& writeToSocket);

    static const PortType DefaultPort = 20000;
    PortType m_port = DefaultPort;
    QString m_remoteHost;
    PortType m_remotePort = DefaultPort;

    QTcpServer m_server;
    SocketsList m_sockets;
};

}  // namespace proxy

}  // namespace tcp_socket

} // namespace plugin

} // namespace comms_champion
