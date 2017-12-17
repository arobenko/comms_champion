//
// Copyright 2014 - 2017 (C). Alex Robenko. All rights reserved.
//

// This library is free software: you can redistribute it and/or modify
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

/// @file
/// Contains definition of Message object interface and various base classes
/// for custom messages.

#pragma once

#include <cstdint>
#include <memory>
#include <type_traits>

#include "ErrorStatus.h"
#include "Assert.h"
#include "Field.h"

#include "details/MessageInterfaceBuilder.h"

namespace comms
{

/// @brief Main interface class for all the messages.
/// @details Provides basic interface for all the messages.
/// @tparam TOptions Variadic template parameter that contain any number of
///     options to define functionality/behaviour of the message.
///     The options may be comma separated as well as bundled
///     into std::tuple. Supported options are:
///     @li comms::option::BigEndian or comms::option::LittleEndian - options
///         used to specify endianness of the serialisation. If this option is
///         used, readData() functions as well as @ref Endian and
///         @ref Field internal types get defined.
///     @li comms::option::MsgIdType - an option used to specify type of the ID
///         value used to identify the message. If this option is used,
///         the @ref MsgIdType and
///         @ref MsgIdParamType types get defined.
///     @li comms::option::IdInfoInterface - an option used to provide polymorphic
///         id retrieval functionality. If this option is used in conjunction with
///         comms::option::MsgIdType, the
///         getId() member function is defined.
///     @li comms::option::ReadIterator - an option used to specify type of iterator
///         used for reading. If this option is not used, then read()
///         member function doesn't exist.
///     @li comms::option::WriteIterator - an option used to specify type of iterator
///         used for writing. If this option is not used, then write()
///         member function doesn't exist.
///     @li comms::option::ValidCheckInterface - an option used to add valid()
///         member function to the default interface.
///     @li comms::option::LengthInfoInterface - an option used to add length()
///         member function to the default interface.
///     @li comms::option::RefreshInterface - an option used to add refresh()
///         member function to the default interface.
///     @li comms::option::Handler - an option used to specify type of message handler
///         object used to handle the message when it received. If this option
///         is not used, then dispatch() member function doesn't exist. See
///         dispatch() documentation for details.
///     @li comms::option::NoVirtualDestructor - Force the destructor to be
///         non-virtual, even if there are virtual functions in use.
///     @headerfile comms/Message.h
template <typename... TOptions>
class Message : public details::MessageInterfaceBuilderT<TOptions...>
{
    using BaseImpl = details::MessageInterfaceBuilderT<TOptions...>;
public:

    /// @brief All the options bundled into struct.
    /// @details See @ref page_message_options_interface for reference.
    using InterfaceOptions = details::MessageInterfaceOptionsParser<TOptions...>;

    /// @brief Destructor.
    /// @details Becomes @b virtual if the message interface is defined to expose
    ///     any polymorphic behavior, i.e. if there is at least one virtual function.
    ///     It is possible to explicitly suppress @b virtual declaration by
    ///     using comms::option::NoVirtualDestructor option.
    ~Message() noexcept = default;

#ifdef FOR_DOXYGEN_DOC_ONLY
    /// @brief Type used for message ID.
    /// @details The type exists only if comms::option::MsgIdType option
    ///     was provided to comms::Message to specify it.
    using MsgIdType = typename BaseImpl::MsgIdType;

    /// @brief Type used for message ID passed as parameter or returned from function.
    /// @details It is equal to @ref MsgIdType for numeric types and becomes
    ///     "const-reference-to" @ref MsgIdType for more complex types.
    ///      The type exists only if @ref MsgIdType exists, i.e.
    ///      the comms::option::MsgIdType option was used.
    using MsgIdParamType = typename BaseImpl::MsgIdParamType;

    /// @brief Serialisation endian type.
    /// @details The type exists only if comms::option::BigEndian or
    ///     comms::option::LittleEndian options were used to specify it.
    using Endian = typename BaseImpl::Endian;

    /// @brief Type of default base class for all the fields.
    /// @details Requires definition of the @ref Endian type, i.e. the type
    ///     exist only if comms::option::BigEndian or
    ///     comms::option::LittleEndian options were used.
    using Field = BaseImpl::Field;

    /// @brief Retrieve ID of the message.
    /// @details Invokes pure virtual getIdImpl(). This function exists
    ///     only if comms::option::MsgIdType option was used to specify type
    ///     of the ID value and comms::option::IdInfoInterface option are used.
    /// @return ID of the message.
    MsgIdParamType getId() const;

    /// @brief Type of the iterator used for reading message contents from
    ///     sequence of bytes stored somewhere.
    /// @details The type exists only if comms::option::ReadIterator option
    ///     was provided to comms::Message to specify one.
    using ReadIterator = TypeProvidedWithOption;

    /// @brief Read message contents using provided iterator.
    /// @details The function exists only if comms::option::ReadIterator option
    ///     was provided to comms::Message to specify type of the @ref ReadIterator.
    ///     The contents of the message are updated with bytes being read.
    ///     The buffer is external and maintained by the caller.
    ///     The provided iterator is advanced. The function invokes virtual
    ///     readImpl() function.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @param[in] size Maximum number of bytes that can be read.
    /// @return Status of the operation.
    ErrorStatus read(ReadIterator& iter, std::size_t size);

    /// @brief Type of the iterator used for writing message contents into
    ///     sequence of bytes stored somewhere.
    /// @details The type exists only if comms::option::WriteIterator option
    ///     was provided to comms::Message to specify one.
    using WriteIterator = TypeProvidedWithOption;

    /// @brief Write message contents using provided iterator.
    /// @details The function exists only if comms::option::WriteIterator option
    ///     was provided to comms::Message to specify type of the @ref WriteIterator.
    ///     The contents of the message are serialised into buffer. The buffer
    ///     is external and is maintained by the caller.
    ///     The provided iterator is advanced. The function invokes virtual
    ///     writeImpl() function.
    /// @param[in, out] iter Iterator used for writing the data.
    /// @param[in] size Maximum number of bytes that can be written.
    /// @return Status of the operation.
    ErrorStatus write(WriteIterator& iter, std::size_t size) const;

    /// @brief Check validity of message contents.
    /// @details The function exists only if comms::option::ValidCheckInterface option
    ///     was provided to comms::Message. The function invokes virtual validImpl() function.
    /// @return true for valid contents, false otherwise.
    bool valid() const;

    /// @brief Get number of bytes required to serialise this message.
    /// @details The function exists only if comms::option::LengthInfoInterface option
    ///     was provided to comms::Message. The function invokes virtual lengthImpl() function.
    /// @return Number of bytes required to serialise this message.
    std::size_t length() const;

    /// @brief Refresh to contents of the message.
    /// @details Many protocols define their messages in a way that the content
    ///     of some fields may depend on the value of the other field(s). For
    ///     example, providing in one field the information about number of
    ///     elements in the list that will follow later. Another example is
    ///     having bits in a bitmask field specifying whether other optional
    ///     fields exist. In this case, directly modifying value of some
    ///     fields may leave a message contents in an inconsistent state.
    ///     Having refresh() member function allows the developer to bring
    ///     the message into a consistent state prior to sending it over
    ///     I/O link . @n
    ///     The function exists only if comms::option::RefreshInterface option
    ///     was provided to comms::Message. The function invokes virtual
    ///     refreshImpl() function.
    /// @return true in case the contents of the message were modified, false if
    ///     all the fields of the message remained unchanged.
    bool refresh();

    /// @brief Type of the message handler object.
    /// @details The type exists only if comms::option::Handler option
    ///     was provided to comms::Message to specify one.
    using Handler = TypeProvidedWithOption;

    /// @brief Return type of the @ref dispatch() member function.
    /// @details Equal to @b Handler::RetType if such exists, @b void otherwise.
    using DispatchRetType = typename Handler::RetType;

    /// @brief Dispatch message to the handler for processing.
    /// @details The function exists only if comms::option::Handler option
    ///     was provided to comms::Message to specify type of the handler.
    ///     The function invokes virtual dispatchImpl() function.
    /// @param handler Handler object to dispatch message to.
    DispatchRetType dispatch(Handler& handler);

#endif // #ifdef FOR_DOXYGEN_DOC_ONLY

protected:

#ifdef FOR_DOXYGEN_DOC_ONLY
    /// @brief Pure virtual function used to retrieve ID of the message.
    /// @details Called by getId(), must be implemented in the derived class.
    ///     This function exists
    ///     only if comms::option::MsgIdType option was used to specify type
    ///     of the ID value as well as comms::option::IdInfoInterface.
    /// @return ID of the message.
    virtual MsgIdParamType getIdImpl() const = 0;

    /// @brief Virtual function used to implement read operation.
    /// @details Called by read(), expected be implemented in the derived class.
    ///     The function exists only if comms::option::ReadIterator option
    ///     was provided to comms::Message to specify type of the @ref ReadIterator.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @param[in] size Maximum number of bytes that can be read.
    /// @return Status of the operation. If not overridden returns
    ///     comms::ErrorStatus::NotSupported.
    virtual comms::ErrorStatus readImpl(ReadIterator& iter, std::size_t size);

    /// @brief Virtual function used to implement write operation.
    /// @details Called by write(), expected be implemented in the derived class.
    ///     The function exists only if comms::option::WriteIterator option
    ///     was provided to comms::Message to specify type of the @ref WriteIterator.
    /// @param[in, out] iter Iterator used for writing the data.
    /// @param[in] size Maximum number of bytes that can be written.
    /// @return Status of the operation. If not overridden returns
    ///     comms::ErrorStatus::NotSupported.
    virtual comms::ErrorStatus writeImpl(WriteIterator& iter, std::size_t size) const;

    /// @brief Pure virtual function used to implement contents validity check.
    /// @details Called by valid(), must be implemented in the derived class.
    ///     The function exists only if comms::option::ValidCheckInterface option
    ///     was provided to comms::Message.
    /// @return true for valid contents, false otherwise.
    virtual bool validImpl() const = 0;

    /// @brief Pure virtual function used to retrieve number of bytes required
    ///     to serialise this message.
    /// @details Called by length(), must be implemented in the derived class.
    ///     The function exists only if comms::option::LengthInfoInterface option
    ///     was provided to comms::Message.
    /// @return Number of bytes required to serialise this message.
    virtual std::size_t lengthImpl() const = 0;

    /// @brief Virtual function used to bring contents of the message
    ///     into a consistent state.
    /// @details Called by refresh(), can be overridden in the derived class.
    ///     If not overridden, does nothing and returns false indicating that
    ///     contents of the message haven't been changed.
    ///     The function exists only if comms::option::RefreshInterface option
    ///     was provided to comms::Message.
    /// @return true in case the contents of the message were modified, false if
    ///     all the fields of the message remained unchanged.
    virtual bool refreshImpl();

    /// @brief Pure virtual function used to dispatch message to the handler
    ///     object for processing.
    /// @details Called by dispatch(), must be implemented in the derived class.
    ///     The function exists only if comms::option::Handler option was
    ///     provided to comms::Message to specify type of the handler.
    /// @param handler Handler object to dispatch message to.
    virtual DispatchRetType dispatchImpl(Handler& handler) = 0;

    /// @brief Write data into the output area.
    /// @details Use this function to write data to the output area using
    ///     provided iterator. This function requires knowledge about serialisation
    ///     endian. It exists only if endian type was
    ///     specified using comms::option::BigEndian or comms::option::LittleEndian
    ///     options to the class.
    /// @tparam T Type of the value to write. Must be integral.
    /// @tparam Type of output iterator
    /// @param[in] value Integral type value to be written.
    /// @param[in, out] iter Output iterator.
    /// @pre The iterator must be valid and can be successfully dereferenced
    ///      and incremented at least sizeof(T) times.
    /// @post The iterator is advanced.
    /// @note Thread safety: Safe for distinct buffers, unsafe otherwise.
    template <typename T, typename TIter>
    static void writeData(T value, TIter& iter);

    /// @brief Write partial data into the output area.
    /// @details Use this function to write partial data to the output area using
    ///     provided iterator. This function requires knowledge about serialisation
    ///     endian. It exists only if endian type was
    ///     specified using comms::option::BigEndian or comms::option::LittleEndian
    ///     options to the class.
    /// @tparam TSize Length of the value in bytes known in compile time.
    /// @tparam T Type of the value to write. Must be integral.
    /// @tparam TIter Type of output iterator
    /// @param[in] value Integral type value to be written.
    /// @param[in, out] iter Output iterator.
    /// @pre TSize <= sizeof(T)
    /// @pre The iterator must be valid and can be successfully dereferenced
    ///      and incremented at least TSize times.
    /// @post The iterator is advanced.
    /// @note Thread safety: Safe for distinct buffers, unsafe otherwise.
    template <std::size_t TSize, typename T, typename TIter>
    static void writeData(T value, TIter& iter);

    /// @brief Read data from input area.
    /// @details Use this function to read data from the input area using
    ///     provided iterator. This function requires knowledge about serialisation
    ///     endian. It exists only if endian type was
    ///     specified using comms::option::BigEndian or comms::option::LittleEndian
    ///     options to the class.
    /// @tparam T Return type
    /// @tparam TIter Type of input iterator
    /// @param[in, out] iter Input iterator.
    /// @return The integral type value.
    /// @pre TSize <= sizeof(T)
    /// @pre The iterator must be valid and can be successfully dereferenced
    ///      and incremented at least sizeof(T) times.
    /// @post The iterator is advanced.
    /// @note Thread safety: Safe for distinct stream buffers, unsafe otherwise.
    template <typename T, typename TIter>
    static T readData(TIter& iter);

    /// @brief Read partial data from input area.
    /// @details Use this function to read partial data from the input area using
    ///     provided iterator. This function requires knowledge about serialisation
    ///     endian. It exists only if endian type was
    ///     specified using comms::option::BigEndian or comms::option::LittleEndian
    ///     options to the class.
    /// @tparam T Return type
    /// @tparam TSize number of bytes to read
    /// @tparam TIter Type of input iterator
    /// @param[in, out] iter Input iterator.
    /// @return The integral type value.
    /// @pre TSize <= sizeof(T)
    /// @pre The iterator must be valid and can be successfully dereferenced
    ///      and incremented at least TSize times.
    /// @post The internal pointer of the stream buffer is advanced.
    /// @note Thread safety: Safe for distinct stream buffers, unsafe otherwise.
    template <typename T, std::size_t TSize, typename TIter>
    static T readData(TIter& iter);

#endif // #ifdef FOR_DOXYGEN_DOC_ONLY
};

/// @brief Create and initialise iterator for polymorphic read
/// @tparam TMessage Type of message interface class.
/// @param[in] val Value to initialise the iterator with.
/// @return Initialised iterator for polymorphic read.
template <typename TMessage, typename TVal>
typename TMessage::ReadIterator readIteratorFor(
    const TVal& val)
{
    return typename TMessage::ReadIterator(val);
}

/// @brief Create and initialise iterator for polymorphic write
/// @tparam TMessage Type of message interface class.
/// @param[in] val Value to initialise the iterator with.
/// @return Initialised iterator for polymorphic write.
template <typename TMessage, typename TVal>
typename TMessage::WriteIterator writeIteratorFor(
    const TVal& val)
{
    return typename TMessage::WriteIterator(val);
}

}  // namespace comms

