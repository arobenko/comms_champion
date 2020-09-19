//
// Copyright 2019 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "comms/Message.h"
#include "comms/details/message_check.h"
#include "comms/util/type_traits.h"
#include "comms/details/tag.h"

namespace comms
{

namespace details
{

template <typename TAllMessages, std::size_t TCount>
class DispatchMsgStrongLinearSwitchHelper    
{
    static_assert(TCount <= std::tuple_size<TAllMessages>::value, 
        "Invalid template params");
    static_assert(2 <= TCount, "Invalid invocation");

    static const std::size_t From = std::tuple_size<TAllMessages>::value - TCount;
    using FromElem = typename std::tuple_element<From, TAllMessages>::type;
    static_assert(messageHasStaticNumId<FromElem>(), "Message must define static ID");

public:
    template <
        typename TMsg,
        typename THandler>
    static auto dispatch(typename TMsg::MsgIdParamType id, TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        using RetType = 
            MessageInterfaceDispatchRetType<
                typename std::decay<decltype(handler)>::type>;

        static constexpr typename TMsg::MsgIdParamType fromId = FromElem::doGetId();
        switch(id) {
            case fromId: {
                auto& castedMsg = static_cast<FromElem&>(msg);
                return static_cast<RetType>(handler.handle(castedMsg));
                break;
            }
            default:
                return 
                    DispatchMsgStrongLinearSwitchHelper<TAllMessages, TCount - 1>::
                        dispatch(id, msg, handler);
                
        };
        // dead code (just in case), should not reach here
        return static_cast<RetType>(handler.handle(msg));
    }

    template <typename THandler>
    static bool dispatchType(typename FromElem::MsgIdParamType id, THandler& handler) 
    {
        static constexpr typename FromElem::MsgIdParamType fromId = FromElem::doGetId();
        switch(id) {
            case fromId: {
                handler.template handle<FromElem>();
                return true;
                break;
            }
            default:
                return 
                    DispatchMsgStrongLinearSwitchHelper<TAllMessages, TCount - 1>::
                        dispatchType(id, handler);
                
        };
        // dead code (just in case), should not reach here
        return false;
    }
  
};

template <typename TAllMessages>
class DispatchMsgStrongLinearSwitchHelper<TAllMessages, 1>
{
    static_assert(1 <= std::tuple_size<TAllMessages>::value, 
        "Invalid template params");

    static const std::size_t From = std::tuple_size<TAllMessages>::value - 1U;
    using Elem = typename std::tuple_element<From, TAllMessages>::type;
    static_assert(messageHasStaticNumId<Elem>(), "Message must define static ID");

public:
    template <
        typename TMsg,
        typename THandler>
    static auto dispatch(typename TMsg::MsgIdParamType id, TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        using RetType = 
            MessageInterfaceDispatchRetType<
                typename std::decay<decltype(handler)>::type>;

        typename TMsg::MsgIdParamType elemId = Elem::doGetId();
        if (id != elemId) {
            return static_cast<RetType>(handler.handle(msg));
        }

        auto& castedMsg = static_cast<Elem&>(msg);
        return static_cast<RetType>(handler.handle(castedMsg));
    }

    template <typename THandler>
    static bool dispatchType(typename Elem::MsgIdParamType id, THandler& handler) 
    {
        typename Elem::MsgIdParamType elemId = Elem::doGetId();
        if (id != elemId) {
            return false;
        }

        handler.template handle<Elem>();
        return true;
    }
};

template <typename TAllMessages>
class DispatchMsgStrongLinearSwitchHelper<TAllMessages, 0>
{
public:
    template <
        typename TMsg,
        typename THandler>
    static auto dispatch(typename TMsg::MsgIdParamType id, TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        static_cast<void>(id);

        using RetType = 
            MessageInterfaceDispatchRetType<
                typename std::decay<decltype(handler)>::type>;
        return static_cast<RetType>(handler.handle(msg));
    }

    template <typename TId, typename THandler>
    static bool dispatchType(TId&& id, THandler& handler) 
    {
        static_cast<void>(id);
        static_cast<void>(handler);
        return false;
    }
};

template <typename TAllMessages, std::size_t TOrigIdx, std::size_t TRem>
class DispatchMsgLinearSwitchWeakCountFinder
{
    using OrigMsgType = typename std::tuple_element<TOrigIdx, TAllMessages>::type;
    static const std::size_t Idx = std::tuple_size<TAllMessages>::value - TRem;
    using CurrMsgType = typename std::tuple_element<Idx, TAllMessages>::type;
    static const bool IdsMatch = OrigMsgType::doGetId() == CurrMsgType::doGetId();
public:
    static const std::size_t Value = IdsMatch ? DispatchMsgLinearSwitchWeakCountFinder<TAllMessages, TOrigIdx, TRem - 1>::Value + 1 : 0U;
};

template <typename TAllMessages, std::size_t TOrigIdx>
class DispatchMsgLinearSwitchWeakCountFinder<TAllMessages, TOrigIdx, 0U>
{
public:
    static const std::size_t Value = 0U;
};

template <typename TAllMessages, std::size_t TFrom, std::size_t TCount>
class DispatchMsgWeakLinearSwitchHelper    
{
    static_assert(TFrom + TCount <= std::tuple_size<TAllMessages>::value, 
        "Invalid template params");
    static_assert(2 <= TCount, "Invalid invocation");

    using FromElem = typename std::tuple_element<TFrom, TAllMessages>::type;
    static_assert(messageHasStaticNumId<FromElem>(), "Message must define static ID");

    static const std::size_t SameIdsCount = 
        DispatchMsgLinearSwitchWeakCountFinder<
            TAllMessages, 
            TFrom, 
            TCount
        >::Value;
    
    static_assert(SameIdsCount <= TCount, "Invalid template params");
    static_assert(0U < SameIdsCount, "Invalid template params");

public:
    template <typename TMsg, typename THandler>
    static auto dispatch(typename TMsg::MsgIdParamType id, std::size_t offset, TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        using RetType = 
            MessageInterfaceDispatchRetType<
                typename std::decay<decltype(handler)>::type>;

        static constexpr typename TMsg::MsgIdParamType fromId = FromElem::doGetId();
        switch(id) {
            case fromId:
                return 
                    DispatchMsgWeakLinearSwitchHelper<TAllMessages, TFrom, SameIdsCount>::
                        dispatchOffset(offset, msg, handler);

            default:
                return 
                    DispatchMsgWeakLinearSwitchHelper<TAllMessages, TFrom + SameIdsCount, TCount - SameIdsCount>::
                        dispatch(id, offset, msg, handler);
        };
        // dead code (just in case), should not reach here
        return static_cast<RetType>(handler.handle(msg));
    }

    template <typename TMsg, typename THandler>
    static auto dispatchOffset(std::size_t offset, TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        using RetType = 
            MessageInterfaceDispatchRetType<
                typename std::decay<decltype(handler)>::type>;

        switch(offset) {
            case 0:
            {
                auto& castedMsg = static_cast<FromElem&>(msg);
                return static_cast<RetType>(handler.handle(castedMsg));
            }
            default:
                return 
                    DispatchMsgWeakLinearSwitchHelper<TAllMessages, TFrom + 1, TCount -1>::
                        dispatchOffset(offset - 1, msg, handler);
        };

        // dead code (just in case), should not reach here
        return static_cast<RetType>(handler.handle(msg));
    }

    template <typename THandler>
    static bool dispatchType(typename FromElem::MsgIdParamType id, std::size_t offset, THandler& handler) 
    {
        static constexpr typename FromElem::MsgIdParamType fromId = FromElem::doGetId();
        switch(id) {
            case fromId:
                return 
                    DispatchMsgWeakLinearSwitchHelper<TAllMessages, TFrom, SameIdsCount>::
                        dispatchTypeOffset(offset, handler);

            default:
                return 
                    DispatchMsgWeakLinearSwitchHelper<TAllMessages, TFrom + SameIdsCount, TCount - SameIdsCount>::
                        dispatchType(id, offset, handler);
        };
        // dead code (just in case), should not reach here
        return false;
    }

    template <typename THandler>
    static bool dispatchTypeOffset(std::size_t offset, THandler& handler)
    {
        switch(offset) {
            case 0:
                handler.template handle<FromElem>();
                return true;
            default:
                return 
                    DispatchMsgWeakLinearSwitchHelper<TAllMessages, TFrom + 1, TCount -1>::
                        dispatchTypeOffset(offset - 1, handler);
        };

        // dead code (just in case), should not reach here
        return false;
    }
};

template <typename TAllMessages, std::size_t TFrom>
class DispatchMsgWeakLinearSwitchHelper<TAllMessages, TFrom, 1>
{
    static_assert(TFrom + 1 <= std::tuple_size<TAllMessages>::value, 
        "Invalid template params");

    using Elem = typename std::tuple_element<TFrom, TAllMessages>::type;
    static_assert(messageHasStaticNumId<Elem>(), "Message must define static ID");

public:
    template <typename TMsg, typename THandler>
    static auto dispatch(typename TMsg::MsgIdParamType id, std::size_t offset, TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        using RetType = 
            MessageInterfaceDispatchRetType<
                typename std::decay<decltype(handler)>::type>;

        if (offset != 0U) {
            return static_cast<RetType>(handler.handle(msg));
        }

        typename TMsg::MsgIdParamType elemId = Elem::doGetId();
        if (id != elemId) {
            return static_cast<RetType>(handler.handle(msg));
        }

        auto& castedMsg = static_cast<Elem&>(msg);
        return static_cast<RetType>(handler.handle(castedMsg));
    }

    template <typename TMsg, typename THandler>
    static auto dispatchOffset(std::size_t offset, TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        using RetType = 
            MessageInterfaceDispatchRetType<
                typename std::decay<decltype(handler)>::type>;

        if (offset != 0U) {
            return static_cast<RetType>(handler.handle(msg));
        }

        auto& castedMsg = static_cast<Elem&>(msg);
        return static_cast<RetType>(handler.handle(castedMsg));
    }

    template <typename THandler>
    static bool dispatchType(typename Elem::MsgIdParamType id, std::size_t offset, THandler& handler) 
    {
        if (offset != 0U) {
            return false;
        }

        typename Elem::MsgIdParamType elemId = Elem::doGetId();
        if (id != elemId) {
            return false;
        }

        handler.template handle<Elem>();
        return true;
    }

    template <typename THandler>
    static bool dispatchTypeOffset(std::size_t offset, THandler& handler)
    {
        if (offset != 0U) {
            return false;
        }

        handler.template handle<Elem>();
        return true;
    }
};

template <typename TAllMessages, std::size_t TFrom>
class DispatchMsgWeakLinearSwitchHelper<TAllMessages, TFrom, 0>
{
public:
    template <typename TMsg, typename THandler>
    static auto dispatch(typename TMsg::MsgIdParamType id, std::size_t offset, TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        static_cast<void>(id);
        static_cast<void>(offset);

        using RetType = 
            MessageInterfaceDispatchRetType<
                typename std::decay<decltype(handler)>::type>;
        return static_cast<RetType>(handler.handle(msg));
    }

    template <typename TMsg, typename THandler>
    static auto dispatchOffset(std::size_t offset, TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        static_cast<void>(offset);

        using RetType = 
            MessageInterfaceDispatchRetType<
                typename std::decay<decltype(handler)>::type>;
        return static_cast<RetType>(handler.handle(msg));
    }

    template <typename TId, typename THandler>
    static bool dispatchType(TId&& id, std::size_t offset, THandler& handler)
    {
        static_cast<void>(id);
        static_cast<void>(offset);
        static_cast<void>(handler);
        return false;
    }

    template <typename THandler>
    static bool dispatchTypeOffset(std::size_t offset, THandler& handler)
    {
        static_cast<void>(offset);
        static_cast<void>(handler);
        return false;
    }
};

template <typename...>
class DispatchMsgLinearSwitchHelper    
{
    template <typename... TParams>
    using EmptyTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using StrongTag = comms::details::tag::Tag2<>;

    template <typename... TParams>
    using WeakTag = comms::details::tag::Tag3<>;    

    template <typename TAllMessages, typename...>
    using StrongWeakTag = 
        typename comms::util::LazyShallowConditional<
            allMessagesAreStrongSorted<TAllMessages>()
        >::template Type<
            StrongTag,
            WeakTag
        >;    

    template <typename TAllMessages, typename...>
    using BinSearchTag = 
        typename comms::util::LazyShallowConditional<
            std::tuple_size<TAllMessages>::value == 0U
        >::template Type<
            EmptyTag,
            StrongWeakTag,
            TAllMessages
        >;

    template <typename TAllMessages, typename TMsg>
    using AdjustedTag =
        typename comms::util::LazyShallowConditional<
            comms::isMessageBase<TMsg>()
        >::template Type<
            EmptyTag,
            BinSearchTag,
            TAllMessages
        >;
    
public:
    template <typename TAllMessages, typename TMsg, typename THandler>
    static auto dispatch(TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        using MsgType = typename std::decay<decltype(msg)>::type;
        static_assert(MsgType::hasGetId(), 
            "The used message object must provide polymorphic ID retrieval function");
        static_assert(MsgType::hasMsgIdType(), 
            "Message interface class must define its id type");            
        return dispatchInternal<TAllMessages>(msg.getId(), msg, handler, AdjustedTag<TAllMessages, MsgType>());
    }

    template <typename TAllMessages, typename TId, typename TMsg, typename THandler>
    static auto dispatch(TId&& id, TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        using MsgType = typename std::decay<decltype(msg)>::type;
        static_assert(MsgType::hasMsgIdType(), 
            "Message interface class must define its id type");            
        using MsgIdParamType = typename MsgType::MsgIdParamType;
        return dispatchInternal<TAllMessages>(static_cast<MsgIdParamType>(id), msg, handler, AdjustedTag<TAllMessages, MsgType>());
    }

    template <typename TAllMessages, typename TId, typename TMsg, typename THandler>
    static auto dispatch(TId&& id, std::size_t offset, TMsg& msg, THandler& handler) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        using MsgType = typename std::decay<decltype(msg)>::type;
        static_assert(MsgType::hasMsgIdType(), 
            "Message interface class must define its id type");            
        using MsgIdParamType = typename MsgType::MsgIdParamType;
        return dispatchInternal<TAllMessages>(static_cast<MsgIdParamType>(id), offset, msg, handler, AdjustedTag<TAllMessages, MsgType>());
    }

    template <typename TAllMessages, typename TId, typename THandler>
    static bool dispatchType(TId&& id, THandler& handler)
    {
        return dispatchTypeInternal<TAllMessages>(std::forward<TId>(id), handler, BinSearchTag<TAllMessages>());
    }

    template <typename TAllMessages, typename TId, typename THandler>
    static bool dispatchType(TId&& id, std::size_t offset, THandler& handler)
    {
        return dispatchTypeInternal<TAllMessages>(std::forward<TId>(id), offset, handler, BinSearchTag<TAllMessages>());
    }

private:
    template <typename TAllMessages, typename TMsg, typename THandler, typename... TParams>
    static auto dispatchInternal(typename TMsg::MsgIdParamType id, TMsg& msg, THandler& handler, EmptyTag<TParams...>) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        static_cast<void>(id);
        return handler.handle(msg);
    }

    template <typename TAllMessages, typename TMsg, typename THandler, typename... TParams>
    static auto dispatchInternal(typename TMsg::MsgIdParamType id, std::size_t offset, TMsg& msg, THandler& handler, EmptyTag<TParams...>) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        static_cast<void>(id);
        static_cast<void>(offset);
        return handler.handle(msg);
    }

    template <typename TAllMessages, typename TMsg, typename THandler, typename... TParams>
    static auto dispatchInternal(typename TMsg::MsgIdParamType id, TMsg& msg, THandler& handler, StrongTag<TParams...>) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        return 
            DispatchMsgStrongLinearSwitchHelper<TAllMessages, std::tuple_size<TAllMessages>::value>::
                dispatch(id, msg, handler);
    }

    template <typename TAllMessages, typename TMsg, typename THandler, typename... TParams>
    static auto dispatchInternal(typename TMsg::MsgIdParamType id, std::size_t offset, TMsg& msg, THandler& handler, StrongTag<TParams...>) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        if (offset != 0U) {
            using RetType = 
                MessageInterfaceDispatchRetType<
                    typename std::decay<decltype(handler)>::type>;
            return static_cast<RetType>(handler.handle(msg));

        }
        return 
            DispatchMsgStrongLinearSwitchHelper<TAllMessages, std::tuple_size<TAllMessages>::value>::
                dispatch(id, msg, handler);

    }

    template <typename TAllMessages, typename TMsg, typename THandler, typename... TParams>
    static auto dispatchInternal(typename TMsg::MsgIdParamType id, TMsg& msg, THandler& handler, WeakTag<TParams...>) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        return dispatchInternal<TAllMessages>(id, 0U, msg, handler, WeakTag<>());
    }

    template <typename TAllMessages, typename TMsg, typename THandler, typename... TParams>
    static auto dispatchInternal(typename TMsg::MsgIdParamType id, std::size_t offset, TMsg& msg, THandler& handler, WeakTag<TParams...>) ->
        MessageInterfaceDispatchRetType<
            typename std::decay<decltype(handler)>::type>
    {
        return 
            DispatchMsgWeakLinearSwitchHelper<TAllMessages, 0, std::tuple_size<TAllMessages>::value>::
                dispatch(id, offset, msg, handler);

    }

    template <typename TAllMessages, typename TId, typename THandler, typename... TParams>
    static bool dispatchTypeInternal(TId&& id, THandler& handler, EmptyTag<TParams...>) 
    {
        static_cast<void>(id); 
        static_cast<void>(handler);
        return false;
    }

    template <typename TAllMessages, typename TId, typename THandler, typename... TParams>
    static bool dispatchTypeInternal(TId&& id, std::size_t offset, THandler& handler, EmptyTag<TParams...>) 
    {
        static_cast<void>(id); 
        static_cast<void>(offset);
        static_cast<void>(handler);
        return false;
    }

    template <typename TAllMessages, typename TId, typename THandler, typename... TParams>
    static bool dispatchTypeInternal(TId&& id, THandler& handler, StrongTag<TParams...>) 
    {
        using FirstMsgType = typename std::tuple_element<0, TAllMessages>::type;
        static_assert(comms::isMessageBase<FirstMsgType>(), 
            "The type in the tuple are expected to be proper messages");
        static_assert(FirstMsgType::hasMsgIdType(), "The messages must define their ID type");
        using MsgIdParamType = typename FirstMsgType::MsgIdParamType;        

        return 
            DispatchMsgStrongLinearSwitchHelper<TAllMessages, std::tuple_size<TAllMessages>::value>::
                dispatchType(static_cast<MsgIdParamType>(id), handler);
    }

    template <typename TAllMessages, typename TId, typename THandler, typename... TParams>
    static bool dispatchTypeInternal(TId&& id, std::size_t offset, THandler& handler, StrongTag<TParams...>) 
    {
        if (offset != 0U) {
            return false;
        }

        return dispatchTypeInternal<TAllMessages>(std::forward<TId>(id), handler, StrongTag<TParams...>());
    }

    template <typename TAllMessages, typename TId, typename THandler, typename... TParams>
    static bool dispatchTypeInternal(TId&& id, THandler& handler, WeakTag<TParams...>) 
    {
        return dispatchTypeInternal<TAllMessages>(std::forward<TId>(id), 0U, handler, WeakTag<>());
    }

    template <typename TAllMessages, typename TId, typename THandler, typename... TParams>
    static bool dispatchTypeInternal(TId&& id, std::size_t offset, THandler& handler, WeakTag<TParams...>) 
    {
        using FirstMsgType = typename std::tuple_element<0, TAllMessages>::type;
        static_assert(comms::isMessageBase<FirstMsgType>(), 
            "The type in the tuple are expected to be proper messages");
        static_assert(FirstMsgType::hasMsgIdType(), "The messages must define their ID type");
        using MsgIdParamType = typename FirstMsgType::MsgIdParamType;        

        return 
            DispatchMsgWeakLinearSwitchHelper<TAllMessages, 0, std::tuple_size<TAllMessages>::value>::
                dispatchType(static_cast<MsgIdParamType>(id), offset, handler);
    }

};

} // namespace details

} // namespace comms
