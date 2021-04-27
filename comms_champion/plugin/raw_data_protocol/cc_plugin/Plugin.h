//
// Copyright 2016 - 2021 (C). Alex Robenko. All rights reserved.
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

#include <QtCore/QObject>
#include <QtCore/QtPlugin>
#include "comms_champion/comms_champion.h"

namespace comms_champion
{

namespace plugin
{

namespace raw_data_protocol
{

namespace cc_plugin
{

class Plugin : public comms_champion::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.RawDataProtocol" FILE "raw_data_protocol.json")
    Q_INTERFACES(comms_champion::Plugin)

public:
    Plugin();
    ~Plugin() noexcept;
};

}  // namespace cc_plugin

}  // namespace raw_data_protocol

}  // namespace plugin

}  // namespace comms_champion
