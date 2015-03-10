//
// Copyright 2015 (C). Alex Robenko. All rights reserved.
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

#include "comms_champion/PluginControlInterface.h"

#include <cassert>

#include <QtCore/QObject>

#include "comms_champion/version.h"

#include "MsgMgr.h"
#include "GuiAppMgr.h"
#include "PluginControlInterfaceImpl.h"

namespace comms_champion
{

PluginControlInterface::PluginControlInterface()
  : m_pImpl(new PluginControlInterfaceImpl())
{
}

PluginControlInterface::~PluginControlInterface() = default;

unsigned PluginControlInterface::version()
{
    return COMMS_CHAMPION_VERSION;
}

void PluginControlInterface::setProtocol(ProtocolPtr protocol)
{
    assert(m_pImpl);
    m_pImpl->setProtocol(std::move(protocol));
}

void PluginControlInterface::clearProtocol()
{
    assert(m_pImpl);
    m_pImpl->clearProtocol();
}

void PluginControlInterface::addSocket(SocketPtr socket)
{
    assert(m_pImpl);
    m_pImpl->addSocket(std::move(socket));
}

void PluginControlInterface::removeSocket(SocketPtr socket)
{
    assert(m_pImpl);
    m_pImpl->removeSocket(std::move(socket));
}

void PluginControlInterface::addMainToolbarAction(ActionPtr action)
{
    assert(m_pImpl);
    m_pImpl->addMainToolbarAction(std::move(action));
}

void PluginControlInterface::removeMainToolbarAction(ActionPtr action)
{
    assert(m_pImpl);
    m_pImpl->removeMainToolbarAction(std::move(action));
}

}  // namespace comms_champion


