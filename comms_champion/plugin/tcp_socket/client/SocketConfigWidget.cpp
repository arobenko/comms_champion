//
// Copyright 2015 - 2019 (C). Alex Robenko. All rights reserved.
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

#include "SocketConfigWidget.h"

#include <limits>

namespace comms_champion
{

namespace plugin
{

namespace tcp_socket
{

namespace client
{

SocketConfigWidget::SocketConfigWidget(
    Socket& socket,
    QWidget* parentObj)
  : Base(parentObj),
    m_socket(socket)
{
    m_ui.setupUi(this);

    m_ui.m_portSpinBox->setRange(
        1,
        static_cast<int>(std::numeric_limits<PortType>::max()));

    m_ui.m_hostLineEdit->setText(m_socket.getHost());

    m_ui.m_portSpinBox->setValue(
        static_cast<int>(m_socket.getPort()));

    connect(
        m_ui.m_hostLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(hostValueChanged(const QString&)));

    connect(
        m_ui.m_portSpinBox, SIGNAL(valueChanged(int)),
        this, SLOT(portValueChanged(int)));
}

SocketConfigWidget::~SocketConfigWidget() noexcept = default;

void SocketConfigWidget::hostValueChanged(const QString& value)
{
    m_socket.setHost(value);
}

void SocketConfigWidget::portValueChanged(int value)
{
    m_socket.setPort(static_cast<PortType>(value));
}

}  // namespace client

}  // namespace tcp_socket

}  // namespace plugin

}  // namespace comms_champion


