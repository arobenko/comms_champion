//
// Copyright 2016 - 2017 (C). Alex Robenko. All rights reserved.
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

#include "comms_champion/comms_champion.h"
#include "demo/Message.h"

namespace demo
{

namespace cc_plugin
{

class Message : public comms_champion::MessageBase<demo::Message>
{
    using Base = comms_champion::MessageBase<demo::Message>;
public:
    Message();
    virtual ~Message() noexcept;

protected:

    virtual const QVariantList& extraTransportFieldsPropertiesImpl() const override;
    virtual QString idAsStringImpl() const override;
};

}  // namespace cc_plugin

}  // namespace demo

