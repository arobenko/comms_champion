//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "comms_champion/field_wrapper/IntValueWrapper.h"

#include "comms_champion/field_wrapper/FieldWrapperHandler.h"

namespace comms_champion
{

namespace field_wrapper
{

IntValueWrapper::IntValueWrapper() {}

IntValueWrapper::~IntValueWrapper() noexcept = default;

IntValueWrapper::UnderlyingType IntValueWrapper::minValue() const
{
    return minValueImpl();
}

IntValueWrapper::UnderlyingType IntValueWrapper::maxValue() const
{
    return maxValueImpl();
}

double IntValueWrapper::getScaled() const
{
    return getScaledImpl();
}

void IntValueWrapper::setScaled(double value)
{
    setScaledImpl(value);
}

double IntValueWrapper::scaleValue(UnderlyingType value) const
{
    return scaleValueImpl(value);
}

bool IntValueWrapper::isSigned() const
{
    return isSignedImpl();
}

std::size_t IntValueWrapper::valueTypeSize() const
{
    return valueTypeSizeImpl();
}

IntValueWrapper::Ptr IntValueWrapper::clone()
{
    return cloneImpl();
}

void IntValueWrapper::dispatchImpl(FieldWrapperHandler& handler)
{
    handler.handle(*this);
}

}  // namespace field_wrapper

}  // namespace comms_champion


