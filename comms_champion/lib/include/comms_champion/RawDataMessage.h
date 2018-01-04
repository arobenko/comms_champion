//
// Copyright 2015 - 2016 (C). Alex Robenko. All rights reserved.
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

#include <cassert>
#include <tuple>

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtCore/QString>
CC_ENABLE_WARNINGS()

#include "comms/comms.h"
#include "property/field.h"
#include "ProtocolMessageBase.h"

namespace comms_champion
{

namespace details
{

template <typename TMsgBase, typename TField>
class RawDataMessageImpl : public
    comms::MessageBase<
        TMsgBase,
        comms::option::NoIdImpl,
        comms::option::FieldsImpl<std::tuple<TField> >,
        comms::option::MsgType<RawDataMessageImpl<TMsgBase, TField> >
    >
{

};

}  // namespace details

template <typename TProtStack>
class RawDataMessage : public
    ProtocolMessageBase<
        details::RawDataMessageImpl<
            typename TProtStack::MsgPtr::element_type,
            typename std::tuple_element<std::tuple_size<typename TProtStack::AllFields>::value - 1, typename TProtStack::AllFields>::type>,
        RawDataMessage<TProtStack>
    >
{
public:
    virtual ~RawDataMessage() noexcept = default;

protected:
    virtual const char*
    nameImpl() const override
    {
        static const char* Str = "Generic Raw Data Message";
        return Str;
    }

    virtual const QVariantList&  extraTransportFieldsPropertiesImpl() const override
    {
        static const QVariantList List;
        return List;
    }

    virtual const QVariantList& fieldsPropertiesImpl() const override
    {
        static const QVariantList Props = createFieldsProperties();
        return Props;
    }

    virtual QString idAsStringImpl() const override
    {
        assert(!"Mustn't be called");
        return QString();
    }

    virtual void resetImpl() override
    {
        assert(!"Mustn't be called");
    }

    virtual bool assignImpl(const comms_champion::Message& other) override
    {
        static_cast<void>(other);
        assert(!"Mustn't be called");
        return false;
    }

private:

    static QVariantList createFieldsProperties()
    {
        QVariantList props;
        props.append(
            property::field::ArrayList().name("Data").asMap());
        return props;
    }
};

}  // namespace comms_champion


