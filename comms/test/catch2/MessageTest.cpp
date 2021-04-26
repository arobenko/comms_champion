//
// Copyright 2014 - 2021 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstdint>
#include <cstddef>
#include <memory>
#include <iterator>

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "comms/comms.h"
#include "CommsTestCommonCatch2.h"

namespace
{

const std::string Message1Str("Message1");
const std::string Message3Str("Message3");
const std::string Message4Str("Message4");
const std::string Message9Str("Message9");

template <typename TMessage>
TMessage internalReadWriteTest(
    typename TMessage::ReadIterator const buf,
    std::size_t bufSize,
    comms::ErrorStatus expectedErrStatus = comms::ErrorStatus::Success)
{
    TMessage msg;

    auto readIter = buf;
    auto es = msg.read(readIter, bufSize);
    REQUIRE(es == expectedErrStatus);

    if (es == comms::ErrorStatus::Success) {
        auto diff = static_cast<unsigned>(std::distance(buf, readIter));
        REQUIRE(0 < diff);

        typedef typename TMessage::WriteIterator WriteIterator;
        typedef typename std::decay<decltype(*(std::declval<WriteIterator>()))>::type CharType;

        std::unique_ptr<CharType[]> outDataBuf(new CharType[diff]);
        auto writeIter = &outDataBuf[0];
        auto writeES = msg.write(writeIter, diff);
        REQUIRE(writeES == comms::ErrorStatus::Success);
        REQUIRE(std::equal(buf, buf + diff, static_cast<const CharType*>(&outDataBuf[0])));
    }
    return msg;
}

template <typename TMessage>
void internalWriteReadTest(
    TMessage& msg,
    typename TMessage::WriteIterator const buf,
    std::size_t bufSize,
    comms::ErrorStatus expectedErrStatus = comms::ErrorStatus::Success)
{
    auto writeIter = buf;
    auto es = msg.write(writeIter, bufSize);
    REQUIRE(es == expectedErrStatus);

    if (es == comms::ErrorStatus::Success) {
        auto diff = static_cast<std::size_t>(std::distance(buf, writeIter));
        TMessage readMsg;
        auto readIter = static_cast<const std::uint8_t*>(buf);
        auto readES = readMsg.read(readIter, diff);
        REQUIRE(readES == comms::ErrorStatus::Success);
        REQUIRE(msg == readMsg);
    }
}

typedef std::tuple<
    comms::option::MsgIdType<MessageType>,
    comms::option::IdInfoInterface,
    comms::option::ReadIterator<const std::uint8_t*>,
    comms::option::WriteIterator<std::uint8_t*>,
    comms::option::ValidCheckInterface,
    comms::option::LengthInfoInterface,
    comms::option::Handler<comms::EmptyHandler>,
    comms::option::NameInterface
> CommonOptions;

typedef
    comms::Message<
        comms::option::BigEndian,
        CommonOptions
    > BeMessageBase;
typedef
    comms::Message<
        comms::option::LittleEndian,
        CommonOptions
    > LeMessageBase;

static_assert(std::has_virtual_destructor<BeMessageBase>::value,
    "BeMessageBase is expected to have virtual destructor");

static_assert(std::has_virtual_destructor<LeMessageBase>::value,
    "LeMessageBase is expected to have virtual destructor");

static_assert(BeMessageBase::hasMsgIdType(), "Wrong interface");
static_assert(BeMessageBase::hasEndian(), "Wrong interface");
static_assert(BeMessageBase::hasGetId(), "Wrong interface");
static_assert(BeMessageBase::hasRead(), "Wrong interface");
static_assert(BeMessageBase::hasWrite(), "Wrong interface");
static_assert(BeMessageBase::hasValid(), "Wrong interface");
static_assert(BeMessageBase::hasLength(), "Wrong interface");
static_assert(!BeMessageBase::hasRefresh(), "Wrong interface");
static_assert(BeMessageBase::hasDispatch(), "Wrong interface");
static_assert(!BeMessageBase::hasTransportFields(), "Wrong interface");
static_assert(!BeMessageBase::hasVersionInTransportFields(), "Wrong interface");


typedef std::tuple<
    comms::option::MsgIdType<MessageType>
> BasicCommonOptions;

typedef
    comms::Message<
        comms::option::BigEndian,
        BasicCommonOptions
    > BeBasicMessageBase;

static_assert(!std::has_virtual_destructor<BeBasicMessageBase>::value,
    "BeBasicMessageBase is expected to NOT have virtual destructor");

static_assert(BeBasicMessageBase::hasMsgIdType(), "Wrong interface");
static_assert(BeBasicMessageBase::hasEndian(), "Wrong interface");
static_assert(!BeBasicMessageBase::hasGetId(), "Wrong interface");
static_assert(!BeBasicMessageBase::hasRead(), "Wrong interface");
static_assert(!BeBasicMessageBase::hasWrite(), "Wrong interface");
static_assert(!BeBasicMessageBase::hasValid(), "Wrong interface");
static_assert(!BeBasicMessageBase::hasLength(), "Wrong interface");
static_assert(!BeBasicMessageBase::hasRefresh(), "Wrong interface");
static_assert(!BeBasicMessageBase::hasDispatch(), "Wrong interface");
static_assert(!BeBasicMessageBase::hasTransportFields(), "Wrong interface");
static_assert(!BeBasicMessageBase::hasVersionInTransportFields(), "Wrong interface");

typedef std::tuple<
    comms::option::MsgIdType<MessageType>,
    comms::option::ReadIterator<const std::uint8_t*>
> ReadOnlyCommonOptions;

typedef
    comms::Message<
        comms::option::BigEndian,
        ReadOnlyCommonOptions
    > BeReadOnlyMessageBase;

static_assert(std::has_virtual_destructor<BeReadOnlyMessageBase>::value,
    "BeReadOnlyMessageBase is expected to have virtual destructor");


typedef std::tuple<
    comms::option::MsgIdType<MessageType>,
    comms::option::WriteIterator<std::back_insert_iterator<std::vector<std::uint8_t> > >
> WriteOnlyCommonOptions;

typedef
    comms::Message<
        comms::option::BigEndian,
        WriteOnlyCommonOptions
    > BeWriteOnlyMessageBase;

static_assert(BeWriteOnlyMessageBase::hasMsgIdType(), "Wrong interface");
static_assert(BeWriteOnlyMessageBase::hasEndian(), "Wrong interface");
static_assert(!BeWriteOnlyMessageBase::hasGetId(), "Wrong interface");
static_assert(!BeWriteOnlyMessageBase::hasRead(), "Wrong interface");
static_assert(BeWriteOnlyMessageBase::hasWrite(), "Wrong interface");
static_assert(!BeWriteOnlyMessageBase::hasValid(), "Wrong interface");
static_assert(!BeWriteOnlyMessageBase::hasLength(), "Wrong interface");
static_assert(!BeWriteOnlyMessageBase::hasRefresh(), "Wrong interface");
static_assert(!BeWriteOnlyMessageBase::hasDispatch(), "Wrong interface");
static_assert(!BeWriteOnlyMessageBase::hasTransportFields(), "Wrong interface");
static_assert(!BeWriteOnlyMessageBase::hasVersionInTransportFields(), "Wrong interface");

static_assert(std::has_virtual_destructor<BeWriteOnlyMessageBase>::value,
    "BeWriteOnlyMessageBase is expected to have virtual destructor");


typedef std::tuple<
    comms::option::MsgIdType<MessageType>,
    comms::option::LengthInfoInterface
> LengthOnlyCommonOptions;

typedef
    comms::Message<
        comms::option::BigEndian,
        LengthOnlyCommonOptions
    > BeLengthOnlyMessageBase;

static_assert(BeLengthOnlyMessageBase::hasMsgIdType(), "Wrong interface");
static_assert(BeLengthOnlyMessageBase::hasEndian(), "Wrong interface");
static_assert(!BeLengthOnlyMessageBase::hasGetId(), "Wrong interface");
static_assert(!BeLengthOnlyMessageBase::hasRead(), "Wrong interface");
static_assert(!BeLengthOnlyMessageBase::hasWrite(), "Wrong interface");
static_assert(!BeLengthOnlyMessageBase::hasValid(), "Wrong interface");
static_assert(BeLengthOnlyMessageBase::hasLength(), "Wrong interface");
static_assert(!BeLengthOnlyMessageBase::hasRefresh(), "Wrong interface");
static_assert(!BeLengthOnlyMessageBase::hasDispatch(), "Wrong interface");
static_assert(!BeLengthOnlyMessageBase::hasTransportFields(), "Wrong interface");
static_assert(!BeLengthOnlyMessageBase::hasVersionInTransportFields(), "Wrong interface");

static_assert(std::has_virtual_destructor<BeLengthOnlyMessageBase>::value,
    "BeLengthOnlyMessageBase is expected to have virtual destructor");


typedef Message1<BeMessageBase> BeMsg1;
typedef Message1<LeMessageBase> LeMsg1;

static_assert(std::is_same<BeMsg1::Field_value1::ValueType, std::uint16_t>::value, "Check failed");

typedef Message2<BeMessageBase> BeMsg2;

typedef Message1<BeBasicMessageBase> BeBasicMsg1;
typedef Message2<BeBasicMessageBase> BeBasicMsg2;
typedef Message3<BeBasicMessageBase> BeBasicMsg3;
typedef Message90_1<BeBasicMessageBase> BeBasicMsg90_1;
typedef Message90_2<BeBasicMessageBase> BeBasicMsg90_2;

typedef Message1<BeReadOnlyMessageBase> BeReadOnlyMsg1;
typedef Message1<BeWriteOnlyMessageBase> BeWriteOnlyMsg1;
typedef Message1<BeLengthOnlyMessageBase> BeLengthOnlyMsg1;

typedef Message3<BeMessageBase> BeMsg3;
typedef Message3<LeMessageBase> LeMsg3;

typedef Message90_1<BeMessageBase> BeMsg90_1;
typedef Message90_2<BeMessageBase> BeMsg90_2;    

typedef
    comms::Message<
        CommonOptions,
        comms::option::BigEndian,
        comms::option::RefreshInterface
    > BeRefreshableMessageBase;

static_assert(BeRefreshableMessageBase::hasMsgIdType(), "Wrong interface");
static_assert(BeRefreshableMessageBase::hasEndian(), "Wrong interface");
static_assert(BeRefreshableMessageBase::hasGetId(), "Wrong interface");
static_assert(BeRefreshableMessageBase::hasRead(), "Wrong interface");
static_assert(BeRefreshableMessageBase::hasWrite(), "Wrong interface");
static_assert(BeRefreshableMessageBase::hasValid(), "Wrong interface");
static_assert(BeRefreshableMessageBase::hasLength(), "Wrong interface");
static_assert(BeRefreshableMessageBase::hasRefresh(), "Wrong interface");
static_assert(BeRefreshableMessageBase::hasDispatch(), "Wrong interface");
static_assert(!BeRefreshableMessageBase::hasTransportFields(), "Wrong interface");
static_assert(!BeRefreshableMessageBase::hasVersionInTransportFields(), "Wrong interface");

static_assert(std::has_virtual_destructor<BeRefreshableMessageBase>::value,
    "BeRefreshableMessageBase is expected to have virtual destructor");


typedef Message4<BeRefreshableMessageBase> BeRefreshableMsg4;
typedef Message9<BeRefreshableMessageBase> BeRefreshableMsg9;

typedef
    comms::Message<
        comms::option::BigEndian,
        comms::option::ReadIterator<const std::uint8_t*>,
        comms::option::WriteIterator<std::uint8_t*>
    > NoIdMsgBase;

static_assert(!NoIdMsgBase::hasMsgIdType(), "Wrong interface");
static_assert(NoIdMsgBase::hasEndian(), "Wrong interface");
static_assert(!NoIdMsgBase::hasGetId(), "Wrong interface");
static_assert(NoIdMsgBase::hasRead(), "Wrong interface");
static_assert(NoIdMsgBase::hasWrite(), "Wrong interface");
static_assert(!NoIdMsgBase::hasValid(), "Wrong interface");
static_assert(!NoIdMsgBase::hasLength(), "Wrong interface");
static_assert(!NoIdMsgBase::hasRefresh(), "Wrong interface");
static_assert(!NoIdMsgBase::hasDispatch(), "Wrong interface");
static_assert(!NoIdMsgBase::hasTransportFields(), "Wrong interface");
static_assert(!NoIdMsgBase::hasVersionInTransportFields(), "Wrong interface");

static_assert(std::has_virtual_destructor<NoIdMsgBase>::value,
    "NoIdMsgBase is expected to have virtual destructor");

typedef Message1<NoIdMsgBase> NoIdMsg1;

typedef
    comms::Message<
        comms::option::ReadIterator<const std::uint8_t*>,
        comms::option::WriteIterator<std::uint8_t*>
    > NoEndianMsgBase;

static_assert(!NoEndianMsgBase::hasMsgIdType(), "Wrong interface");
static_assert(!NoEndianMsgBase::hasEndian(), "Wrong interface");
static_assert(!NoEndianMsgBase::hasGetId(), "Wrong interface");
static_assert(NoEndianMsgBase::hasRead(), "Wrong interface");
static_assert(NoEndianMsgBase::hasWrite(), "Wrong interface");
static_assert(!NoEndianMsgBase::hasValid(), "Wrong interface");
static_assert(!NoEndianMsgBase::hasLength(), "Wrong interface");
static_assert(!NoEndianMsgBase::hasRefresh(), "Wrong interface");
static_assert(!NoEndianMsgBase::hasDispatch(), "Wrong interface");
static_assert(!NoEndianMsgBase::hasTransportFields(), "Wrong interface");
static_assert(!NoEndianMsgBase::hasVersionInTransportFields(), "Wrong interface");

typedef Message5<NoIdMsgBase> NoEndianMsg5;

class BoolHandler;

typedef
    comms::Message<
        comms::option::MsgIdType<MessageType>,
        comms::option::Handler<BoolHandler>,
        comms::option::IdInfoInterface,
        comms::option::BigEndian
    > BoolHandlerMsgBase;

typedef Message1<BoolHandlerMsgBase> BoolHandlerMsg1;
typedef Message2<BoolHandlerMsgBase> BoolHandlerMsg2;
typedef Message3<BoolHandlerMsgBase> BoolHandlerMsg3;

typedef std::tuple<
    BoolHandlerMsg1,
    BoolHandlerMsg2,
    BoolHandlerMsg3
> BoolHandlerAllMessages;

class BoolHandler : public comms::GenericHandler<BoolHandlerMsgBase, BoolHandlerAllMessages, bool>
{
    using Base = comms::GenericHandler<BoolHandlerMsgBase, BoolHandlerAllMessages, bool>;
public:
    using Base::handle;
    virtual bool handle(BoolHandlerMsgBase& msg) override
    {
        m_lastId = msg.getId();
        return true;
    }

    BoolHandlerMsgBase::MsgIdType getLastId() const
    {
        return m_lastId;
    }

private:
    BoolHandlerMsgBase::MsgIdType m_lastId = BoolHandlerMsgBase::MsgIdType();
};

static_assert(BoolHandlerMsgBase::hasMsgIdType(), "Wrong interface");
static_assert(BoolHandlerMsgBase::hasEndian(), "Wrong interface");
static_assert(BoolHandlerMsgBase::hasGetId(), "Wrong interface");
static_assert(!BoolHandlerMsgBase::hasRead(), "Wrong interface");
static_assert(!BoolHandlerMsgBase::hasWrite(), "Wrong interface");
static_assert(!BoolHandlerMsgBase::hasValid(), "Wrong interface");
static_assert(!BoolHandlerMsgBase::hasLength(), "Wrong interface");
static_assert(!BoolHandlerMsgBase::hasRefresh(), "Wrong interface");
static_assert(BoolHandlerMsgBase::hasDispatch(), "Wrong interface");
static_assert(!BoolHandlerMsgBase::hasTransportFields(), "Wrong interface");
static_assert(!BoolHandlerMsgBase::hasVersionInTransportFields(), "Wrong interface");

class SomeDummyBitfield : public
    comms::field::Bitfield<
        comms::Field<comms::option::BigEndian>,
        std::tuple<
            comms::field::IntValue<comms::Field<comms::option::BigEndian>, uint8_t, comms::option::FixedBitLength<4> >,
            comms::field::IntValue<comms::Field<comms::option::BigEndian>, uint8_t, comms::option::FixedBitLength<4> >
        >
    >
{
    using Base =
        comms::field::Bitfield<
            comms::Field<comms::option::BigEndian>,
            std::tuple<
                comms::field::IntValue<comms::Field<comms::option::BigEndian>, uint8_t, comms::option::FixedBitLength<4> >,
                comms::field::IntValue<comms::Field<comms::option::BigEndian>, uint8_t, comms::option::FixedBitLength<4> >
            >
        >;

public:
    COMMS_FIELD_MEMBERS_NAMES(mem1, mem2);
};


typedef std::tuple<
    comms::field::IntValue<
        comms::Field<comms::option::BigEndian>,
        std::uint16_t,
        comms::option::DefaultNumValue<5>
    >,
    SomeDummyBitfield
> ExtraVersionTransport;

struct ExtraTransportMessageBase : public
    comms::Message<
        comms::option::BigEndian,
        CommonOptions,
        comms::option::ExtraTransportFields<ExtraVersionTransport>,
        comms::option::VersionInExtraTransportFields<0>,
        comms::option::RefreshInterface
    >
{
    using Base =
        comms::Message<
            comms::option::BigEndian,
            CommonOptions,
            comms::option::ExtraTransportFields<ExtraVersionTransport>,
            comms::option::VersionInExtraTransportFields<0>,
            comms::option::RefreshInterface
        >;
public:
    COMMS_MSG_TRANSPORT_FIELDS_NAMES(version, dummy);
    COMMS_MSG_TRANSPORT_FIELD_ALIAS_NOTEMPLATE(v, version);
    COMMS_MSG_TRANSPORT_FIELD_ALIAS_NOTEMPLATE(mem1, dummy, mem1);
    COMMS_MSG_TRANSPORT_FIELD_ALIAS_NOTEMPLATE(mem2, dummy, mem2);
};

static_assert(ExtraTransportMessageBase::hasMsgIdType(), "Wrong interface");
static_assert(ExtraTransportMessageBase::hasEndian(), "Wrong interface");
static_assert(ExtraTransportMessageBase::hasGetId(), "Wrong interface");
static_assert(ExtraTransportMessageBase::hasRead(), "Wrong interface");
static_assert(ExtraTransportMessageBase::hasWrite(), "Wrong interface");
static_assert(ExtraTransportMessageBase::hasValid(), "Wrong interface");
static_assert(ExtraTransportMessageBase::hasLength(), "Wrong interface");
static_assert(ExtraTransportMessageBase::hasRefresh(), "Wrong interface");
static_assert(ExtraTransportMessageBase::hasDispatch(), "Wrong interface");
static_assert(ExtraTransportMessageBase::hasTransportFields(), "Wrong interface");
static_assert(ExtraTransportMessageBase::hasVersionInTransportFields(), "Wrong interface");

template <typename TMsgPtr>
class MsgCreationHandler
{
public:

    TMsgPtr getMsg()
    {
        return std::move(m_ptr);
    }

    template <typename TMsg>
    void handle()
    {
        m_ptr.reset(new TMsg);
    }

private:
    TMsgPtr m_ptr;        
};    

} // namespace

TEST_CASE("Test1", "test1") 
{
    static const std::uint8_t Buf[] = {
        0x01, 0x02
    };
    static const std::size_t BufSize =
        std::extent<decltype(Buf)>::value;

    auto beMsg = internalReadWriteTest<BeMsg1>(&Buf[0], BufSize);
    auto value = std::get<0>(beMsg.fields()).value();
    REQUIRE(value == 0x0102);
    REQUIRE(&beMsg.field_value1() == &beMsg.field_f1()); // check alias works

    REQUIRE(beMsg.valid());
    REQUIRE(beMsg.name() == Message1Str);

    auto leMsg = internalReadWriteTest<LeMsg1>(&Buf[0], BufSize);
    value = std::get<0>(leMsg.fields()).value();
    REQUIRE(value == 0x0201);
    REQUIRE(leMsg.valid());
    REQUIRE(leMsg.name() == Message1Str);
}

TEST_CASE("Test2", "test2") 
{
    static const std::uint8_t ExpectecedBeBuf[] = {
        0x01, 0x02
    };

    BeMsg1 beMsg;
    REQUIRE(beMsg.valid());
    std::get<0>(beMsg.fields()).value() = 0x0102;
    std::uint8_t beBuf[2] = {0};
    static const std::size_t BeBufSize = std::extent<decltype(beBuf)>::value;
    internalWriteReadTest(beMsg, &beBuf[0], BeBufSize);
    REQUIRE(
        std::equal(
            &ExpectecedBeBuf[0], &ExpectecedBeBuf[0] + BeBufSize,
            static_cast<const std::uint8_t*>(&beBuf[0])));

    static const std::uint8_t ExpectecedLeBuf[] = {
        0x02, 0x01
    };

    LeMsg1 leMsg;
    std::get<0>(leMsg.fields()).value() = 0x0102;
    REQUIRE(leMsg.valid());
    std::uint8_t leBuf[2] = {0};
    static const std::size_t LeBufSize = std::extent<decltype(leBuf)>::value;
    internalWriteReadTest(leMsg, leBuf, LeBufSize);
    REQUIRE(
        std::equal(
            &ExpectecedLeBuf[0], &ExpectecedLeBuf[0] + LeBufSize,
            static_cast<const std::uint8_t*>(&leBuf[0])));
}

TEST_CASE("Test3", "test3") 
{
    static const std::uint8_t Buf[] = {
        0x01
    };

    static const std::size_t BufSize = std::extent<decltype(Buf)>::value;

    auto beMsg =
        internalReadWriteTest<BeMsg1>(
            Buf,
            BufSize,
            comms::ErrorStatus::NotEnoughData);

    auto leMsg =
        internalReadWriteTest<LeMsg1>(
            Buf,
            BufSize,
            comms::ErrorStatus::NotEnoughData);
}

TEST_CASE("Test4", "test4") 
{
    std::uint8_t buf[1] = {0};

    static const std::size_t BufSize = std::extent<decltype(buf)>::value;

    BeMsg1 beMsg;
    std::get<0>(beMsg.fields()).value() = 0x0102;
    internalWriteReadTest(beMsg, buf, BufSize, comms::ErrorStatus::BufferOverflow);

    LeMsg1 leMsg;
    std::get<0>(leMsg.fields()).value() = 0x0102;
    internalWriteReadTest(leMsg, buf, BufSize, comms::ErrorStatus::BufferOverflow);
}

TEST_CASE("Test5", "test5") 
{
    static const std::uint8_t Buf[] = {
        0x01, 0x02, 0x3, 0x4, (std::uint8_t)-5, 0xde, 0xad, 0x00, 0xaa, 0xff
    };

    static const std::size_t BufSize = std::extent<decltype(Buf)>::value;

    BeMsg3 beMsg;
    REQUIRE(!beMsg.valid()); // there is invalid field
    beMsg = internalReadWriteTest<BeMsg3>(Buf, BufSize);

    REQUIRE(std::get<0>(beMsg.fields()).value() == 0x01020304);
    REQUIRE(std::get<1>(beMsg.fields()).value() == -5);
    REQUIRE(std::get<2>(beMsg.fields()).value() == 0xdead);
    REQUIRE(std::get<3>(beMsg.fields()).value() == 0xaaff);
    REQUIRE(beMsg.length() == 10);
    REQUIRE(beMsg.valid());
    REQUIRE(beMsg.name() == Message3Str);
    REQUIRE(beMsg.doName() == Message3Str);

    LeMsg3 leMsg;
    REQUIRE(!leMsg.valid());
    leMsg = internalReadWriteTest<LeMsg3>(Buf, BufSize);

    REQUIRE(std::get<0>(leMsg.fields()).value() == 0x04030201);
    REQUIRE(std::get<1>(leMsg.fields()).value() == -5);
    REQUIRE(std::get<2>(leMsg.fields()).value() == 0xadde);
    REQUIRE(std::get<3>(leMsg.fields()).value() == 0xffaa00);
    REQUIRE(leMsg.length() == 10);
    REQUIRE(leMsg.valid());
    REQUIRE(leMsg.name() == Message3Str);
}

TEST_CASE("Test6", "test6") 
{
    std::uint8_t buf[4] = {0};
    static const std::size_t BufSize = std::extent<decltype(buf)>::value;

    BeMsg3 beMsg;
    internalWriteReadTest(beMsg, buf, BufSize, comms::ErrorStatus::BufferOverflow);

    LeMsg3 leMsg;
    internalWriteReadTest(leMsg, buf, BufSize, comms::ErrorStatus::BufferOverflow);
}

TEST_CASE("Test7", "test7") 
{
    BeBasicMsg1 msg1;
    REQUIRE(msg1.doGetId() == MessageType1);

    BeReadOnlyMsg1 msg2;
    REQUIRE(msg2.doGetId() == MessageType1);
    static const std::uint8_t Data1[] = { 0x1, 0x2 };
    static const auto Data1Size = std::extent<decltype(Data1)>::value;
    BeReadOnlyMsg1::ReadIterator readIter = &Data1[0];
    auto es = msg2.read(readIter, Data1Size);
    REQUIRE(es == comms::ErrorStatus::Success);
    REQUIRE(std::get<0>(msg2.fields()).value() == 0x0102);

    BeWriteOnlyMsg1 msg3;
    REQUIRE(msg3.doGetId() == MessageType1);
    std::get<0>(msg3.fields()).value() = 0x0102;
    std::vector<std::uint8_t> outData;
    BeWriteOnlyMsg1::WriteIterator writeIter = std::back_inserter(outData);
    es = msg3.write(writeIter, outData.max_size());
    REQUIRE(es == comms::ErrorStatus::Success);
    REQUIRE(outData.size() == 2U);
    REQUIRE(outData[0] == 0x1);
    REQUIRE(outData[1] == 0x2);

    BeLengthOnlyMsg1 msg4;
    REQUIRE(msg4.doGetId() == MessageType1);
    REQUIRE(msg4.length() == 2U);
}

TEST_CASE("Test8", "test8") 
{
    BeRefreshableMsg4 msg;
    REQUIRE(msg.doGetId() == MessageType4);
    REQUIRE(msg.length() == 1U);
    REQUIRE(msg.name() == Message4Str);

    std::vector<std::uint8_t> outData;
    outData.resize(msg.length());
    BeRefreshableMsg4::WriteIterator writeIter = &outData[0];

    internalWriteReadTest(msg, writeIter, outData.size());

    auto& mask = std::get<0>(msg.fields());
    mask.value() = 0x1;
    bool refreshResult = msg.refresh();
    REQUIRE(refreshResult);
    REQUIRE(msg.length() == 3U);
    REQUIRE(msg.field_value2().getMode() == comms::field::OptionalMode::Exists);

    outData.clear();
    outData.resize(msg.length());
    writeIter = &outData[0];
    internalWriteReadTest(msg, writeIter, outData.size());
}

TEST_CASE("Test9", "test9") 
{
    static const std::uint8_t Buf[] = {
        0x12, 0x34
    };
    static const std::size_t BufSize =
        std::extent<decltype(Buf)>::value;

    auto msg1 = internalReadWriteTest<NoIdMsg1>(&Buf[0], BufSize);
    auto value = std::get<0>(msg1.fields()).value();
    REQUIRE(value == 0x1234);
}

TEST_CASE("Test10", "test10") 
{
    static const std::uint8_t Buf[] = {
        0x12, 0x34, 0xff
    };
    static const std::size_t BufSize =
        std::extent<decltype(Buf)>::value;

    auto msg = internalReadWriteTest<NoEndianMsg5>(&Buf[0], BufSize);
    auto value1 = std::get<0>(msg.fields()).value();
    auto value2 = std::get<1>(msg.fields()).value();
    REQUIRE(value1 == 0x1234);
    REQUIRE(value2 == -1);
}

TEST_CASE("Test11", "test11") 
{
    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>
        > Msg1;

    static_assert(!std::has_virtual_destructor<Msg1>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::ReadIterator<const char*>
        > Msg2;

    static_assert(std::has_virtual_destructor<Msg2>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::ReadIterator<const char*>,
            comms::option::NoVirtualDestructor
        > Msg3;

    static_assert(!std::has_virtual_destructor<Msg3>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::WriteIterator<char*>
        > Msg4;

    static_assert(std::has_virtual_destructor<Msg4>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::WriteIterator<char*>,
            comms::option::NoVirtualDestructor
        > Msg5;

    static_assert(!std::has_virtual_destructor<Msg5>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::IdInfoInterface
        > Msg6;

    static_assert(std::has_virtual_destructor<Msg6>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::IdInfoInterface,
            comms::option::NoVirtualDestructor
        > Msg7;

    static_assert(!std::has_virtual_destructor<Msg7>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::ValidCheckInterface
        > Msg8;

    static_assert(std::has_virtual_destructor<Msg8>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::ValidCheckInterface,
            comms::option::NoVirtualDestructor
        > Msg9;

    static_assert(!std::has_virtual_destructor<Msg9>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::LengthInfoInterface
        > Msg10;

    static_assert(std::has_virtual_destructor<Msg10>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::LengthInfoInterface,
            comms::option::NoVirtualDestructor
        > Msg11;

    static_assert(!std::has_virtual_destructor<Msg11>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::RefreshInterface
        > Msg12;

    static_assert(std::has_virtual_destructor<Msg12>::value, "Error");

    typedef
        comms::Message<
            comms::option::BigEndian,
            comms::option::MsgIdType<MessageType>,
            comms::option::RefreshInterface,
            comms::option::NoVirtualDestructor
        > Msg13;

    static_assert(!std::has_virtual_destructor<Msg13>::value, "Error");
}

TEST_CASE("Test12", "test12") 
{
    using Message = comms::GenericMessage<BeMessageBase>;
    Message msg(MessageType1);

    BeMessageBase& interface = msg;
    REQUIRE(interface.getId() == MessageType1);
    REQUIRE(interface.valid());
    REQUIRE(interface.length() == 0);

    static const std::uint8_t Buf[] = {
        0x12, 0x34, 0x56
    };
    static const std::size_t BufSize =
        std::extent<decltype(Buf)>::value;

    auto readIter = comms::readIteratorFor(interface, Buf);
    auto es = interface.read(readIter, BufSize);
    REQUIRE(es == comms::ErrorStatus::Success);
    REQUIRE(interface.length() == BufSize);
    REQUIRE(msg.field_data().value().size() == BufSize);

    bool equal =
        std::equal(
            std::begin(Buf), std::end(Buf), msg.field_data().value().begin());
    REQUIRE(equal);

    std::vector<std::uint8_t> outBuf;
    outBuf.resize(BufSize);
    auto writeIter = comms::writeIteratorFor<BeMessageBase>(&outBuf[0]);
    es = interface.write(writeIter, outBuf.size());
    REQUIRE(es == comms::ErrorStatus::Success);

    equal =
        std::equal(
            std::begin(Buf), std::end(Buf), outBuf.begin());
    REQUIRE(equal);
}

TEST_CASE("Test13", "test13") 
{
    BoolHandlerMsg1 msg1;
    BoolHandlerMsg2 msg2;
    BoolHandlerMsg3 msg3;

    BoolHandler handler;
    REQUIRE(msg1.dispatch(handler));
    REQUIRE(handler.getLastId() == msg1.getId());
    REQUIRE(msg2.dispatch(handler));
    REQUIRE(handler.getLastId() == msg2.getId());
    REQUIRE(msg3.dispatch(handler));
    REQUIRE(handler.getLastId() == msg3.getId());
}

TEST_CASE("Test14", "test14") 
{
    static const std::uint8_t Buf[] = {
        0x12, 0x34, 0xff
    };
    static const std::size_t BufSize =
        std::extent<decltype(Buf)>::value;

    using Msg5 = Message5<ExtraTransportMessageBase>;

    auto msg = internalReadWriteTest<Msg5>(&Buf[0], BufSize);
    auto value1 = std::get<0>(msg.fields()).value();
    auto value2 = std::get<1>(msg.fields()).value();
    REQUIRE(value1 == 0x1234);
    REQUIRE(value2 == -1);
    REQUIRE(msg.transportField_version().value() == 5U);
    REQUIRE(&msg.transportField_version() == &msg.transportField_v());
    REQUIRE(&msg.transportField_mem1() == &msg.transportField_dummy().field_mem1());
    REQUIRE(&msg.transportField_mem2() == &msg.transportField_dummy().field_mem2());
}

TEST_CASE("Test15", "test15") 
{
    static const std::uint8_t Buf[] = {
        0x1, 0x02, 0x03
    };
    static const std::size_t BufSize =
        std::extent<decltype(Buf)>::value;

    using Msg6 = Message6<BeRefreshableMessageBase>;

    auto msg = internalReadWriteTest<Msg6>(&Buf[0], BufSize);
    REQUIRE(msg.doLength() == 3U);
    REQUIRE(msg.field_value1().field_mask().value() == 1U);
    REQUIRE(msg.field_value1().field_val().field().value() == 0x0203);
    REQUIRE(&msg.field_value1().field_mask() == &msg.field_mask()); // check alias works
    REQUIRE(&msg.field_value1().field_val() == &msg.field_val()); // check alias works

    msg.field_value1().field_mask().value() = 0U;

    BeRefreshableMessageBase& interface = msg;
    REQUIRE(interface.refresh());
    REQUIRE(msg.length() == 1U);
    REQUIRE(!interface.refresh());

    msg.field_value1().field_mask().value() = 1U;
    REQUIRE(interface.refresh());
    REQUIRE(msg.length() == 3U);
}

TEST_CASE("Test16", "test16") 
{
    using Msg7 = Message7<ExtraTransportMessageBase>;
    Msg7 msg;
    REQUIRE(msg.version() == 5U);
    REQUIRE(msg.length() == 4U);

    static const std::uint8_t Buf[] = {
        0x12, 0x34, 0x56, 0x78
    };
    static const std::size_t BufSize =
        std::extent<decltype(Buf)>::value;

    msg = internalReadWriteTest<Msg7>(&Buf[0], BufSize);
    REQUIRE(msg.field_value1().value() == 0x1234);
    REQUIRE(msg.field_value2().doesExist());
    REQUIRE(msg.field_value2().field().value() == 0x5678);

    msg.version() = 4U;
    auto readIter = comms::readIteratorFor(msg, &Buf[0]);
    auto es = msg.read(readIter, BufSize);
    REQUIRE(es == comms::ErrorStatus::Success);
    REQUIRE(msg.length() == 2U);
    REQUIRE(msg.field_value2().isMissing());

    msg.version() = 10;
    REQUIRE(msg.refresh());
    REQUIRE(msg.length() == 4U);
    REQUIRE(msg.field_value2().doesExist());

    msg.version() = 11;
    REQUIRE(msg.refresh());
    REQUIRE(msg.length() == 2U);
    REQUIRE(msg.field_value2().isMissing());
}

TEST_CASE("Test17", "test17") 
{
    using Msg8 = Message8<BeMessageBase>;
    Msg8 msg;
    REQUIRE(msg.length() == 1U);
}

TEST_CASE("Test18", "test18") 
{
    BoolHandlerMsg1 msg1;
    BoolHandlerMsg2 msg2;
    BoolHandlerMsg3 msg3;

    BoolHandler handler;
    REQUIRE(comms::dispatchMsgPolymorphic<BoolHandlerAllMessages>(msg1, handler));
    REQUIRE(handler.getLastId() == msg1.getId());
    REQUIRE(comms::dispatchMsgPolymorphic<BoolHandlerAllMessages>(msg2, handler));
    REQUIRE(handler.getLastId() == msg2.getId());
    REQUIRE(comms::dispatchMsgPolymorphic<BoolHandlerAllMessages>(msg3, handler));
    REQUIRE(handler.getLastId() == msg3.getId());
}

TEST_CASE("Test19", "test19") 
{
    BeBasicMsg1 msg1;
    BeBasicMsg2 msg2;
    BeBasicMsg3 msg3;

    using AllMessages = 
        std::tuple<
            BeBasicMsg1,
            BeBasicMsg2
        >;

    CountHandler<BeBasicMessageBase> handler;
    auto* interface = static_cast<BeBasicMessageBase*>(&msg1);
    comms::dispatchMsgStaticBinSearch<AllMessages>(msg1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg2);
    comms::dispatchMsgStaticBinSearch<AllMessages>(msg2.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsgStaticBinSearch<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 1U);    
}

TEST_CASE("Test20", "test20") 
{
    BeBasicMsg1 msg1;
    BeBasicMsg2 msg2;
    BeBasicMsg3 msg3;
    BeBasicMsg90_1 msg90;

    using AllMessages = 
        std::tuple<
            BeBasicMsg1,
            BeBasicMsg2,
            BeBasicMsg3
        >;

    CountHandler<BeBasicMessageBase> handler;
    auto* interface = static_cast<BeBasicMessageBase*>(&msg1);
    comms::dispatchMsgPolymorphic<AllMessages>((int)msg1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg2);
    comms::dispatchMsgPolymorphic<AllMessages>((unsigned)msg2.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsgPolymorphic<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 0U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90);
    comms::dispatchMsgPolymorphic<AllMessages>(msg90.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 1U);    

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsg<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 4U);
    REQUIRE(handler.getBaseCount() == 1U);    

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsg<AllMessages>(msg3.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 4U);
    REQUIRE(handler.getBaseCount() == 2U);    
}

TEST_CASE("Test21", "test21") 
{
    BeBasicMsg1 msg1;
    BeBasicMsg2 msg2;
    BeBasicMsg3 msg3;
    BeBasicMsg90_1 msg90;

    using AllMessages = 
        std::tuple<
            BeBasicMsg1,
            BeBasicMsg2,
            BeBasicMsg90_1
        >;

    static_assert(comms::details::allMessagesAreStrongSorted<AllMessages>(), "Messages are not sorted");

    CountHandler<BeBasicMessageBase> handler;
    auto* interface = static_cast<BeBasicMessageBase*>(&msg1);
    comms::dispatchMsgPolymorphic<AllMessages>((int)msg1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg2);
    comms::dispatchMsgPolymorphic<AllMessages>((unsigned)msg2.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsgPolymorphic<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 1U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90);
    comms::dispatchMsgPolymorphic<AllMessages>(msg90.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 1U);    
}

TEST_CASE("Test22", "test22") 
{
    BeBasicMsg1 msg1;
    BeBasicMsg2 msg2;
    BeBasicMsg3 msg3;
    BeBasicMsg90_1 msg90_1;
    BeBasicMsg90_2 msg90_2;

    using AllMessages = 
        std::tuple<
            BeBasicMsg1,
            BeBasicMsg2,
            BeBasicMsg90_1,
            BeBasicMsg90_2
        >;

    static_assert(!comms::details::allMessagesAreStrongSorted<AllMessages>(), "Messages are strong sorted, should not be");
    static_assert(comms::details::allMessagesAreWeakSorted<AllMessages>(), "Messages are not sorted");
    REQUIRE(msg90_1.doGetId() == msg90_2.doGetId());

    CountHandler<BeBasicMessageBase> handler;
    auto* interface = static_cast<BeBasicMessageBase*>(&msg1);
    comms::dispatchMsgPolymorphic<AllMessages>((int)msg1.doGetId(), 0U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 0U);

    comms::dispatchMsgPolymorphic<AllMessages>((unsigned)msg1.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 1U);

    interface = static_cast<BeBasicMessageBase*>(&msg2);
    comms::dispatchMsgPolymorphic<AllMessages>(msg2.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 1U);

    comms::dispatchMsgPolymorphic<AllMessages>(msg2.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 2U);

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsgPolymorphic<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 3U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90_1);
    comms::dispatchMsgPolymorphic<AllMessages>(msg90_1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 3U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90_2);
    comms::dispatchMsgPolymorphic<AllMessages>(msg90_2.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 4U);
    REQUIRE(handler.getBaseCount() == 3U);    

    comms::dispatchMsgPolymorphic<AllMessages>(msg90_2.doGetId(), 2U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 4U);
    REQUIRE(handler.getBaseCount() == 4U);    
}

TEST_CASE("Test23", "test23") 
{
    BeBasicMsg1 msg1;
    BeBasicMsg2 msg2;
    BeBasicMsg3 msg3;

    using AllMessages = 
        std::tuple<
            BeBasicMsg1,
            BeBasicMsg2,
            BeBasicMsg3
        >;

    CountHandler<BeBasicMessageBase> handler;
    auto* interface = static_cast<BeBasicMessageBase*>(&msg1);
    comms::dispatchMsgStaticBinSearch<AllMessages>((int)msg1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg2);
    comms::dispatchMsgStaticBinSearch<AllMessages>((unsigned)msg2.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsgStaticBinSearch<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 0U);    

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsgStaticBinSearch<AllMessages>(msg3.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 1U);    
}

TEST_CASE("Test24", "test24") 
{
    BeBasicMsg1 msg1;
    BeBasicMsg2 msg2;
    BeBasicMsg3 msg3;
    BeBasicMsg90_1 msg90;

    using AllMessages = 
        std::tuple<
            BeBasicMsg1,
            BeBasicMsg2,
            BeBasicMsg90_1
        >;

    static_assert(comms::details::allMessagesAreStrongSorted<AllMessages>(), "Messages are not sorted");

    CountHandler<BeBasicMessageBase> handler;
    auto* interface = static_cast<BeBasicMessageBase*>(&msg1);
    comms::dispatchMsgStaticBinSearch<AllMessages>((int)msg1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg2);
    comms::dispatchMsgStaticBinSearch<AllMessages>((unsigned)msg2.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsgStaticBinSearch<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 1U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90);
    comms::dispatchMsgStaticBinSearch<AllMessages>(msg90.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 1U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90);
    comms::dispatchMsg<AllMessages>(msg90.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 4U);
    REQUIRE(handler.getBaseCount() == 1U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90);
    comms::dispatchMsg<AllMessages>(msg90.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 4U);
    REQUIRE(handler.getBaseCount() == 2U);    
}

TEST_CASE("Test25", "test25") 
{
    BeBasicMsg1 msg1;
    BeBasicMsg2 msg2;
    BeBasicMsg3 msg3;
    BeBasicMsg90_1 msg90_1;
    BeBasicMsg90_2 msg90_2;

    using AllMessages = 
        std::tuple<
            BeBasicMsg1,
            BeBasicMsg2,
            BeBasicMsg90_1,
            BeBasicMsg90_2
        >;

    static_assert(!comms::details::allMessagesAreStrongSorted<AllMessages>(), "Messages are strong sorted, should not be");
    static_assert(comms::details::allMessagesAreWeakSorted<AllMessages>(), "Messages are not sorted");
    REQUIRE(msg90_1.doGetId() == msg90_2.doGetId());

    CountHandler<BeBasicMessageBase> handler;
    auto* interface = static_cast<BeBasicMessageBase*>(&msg1);
    comms::dispatchMsgStaticBinSearch<AllMessages>((int)msg1.doGetId(), 0U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 0U);

    comms::dispatchMsgStaticBinSearch<AllMessages>((unsigned)msg1.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 1U);

    interface = static_cast<BeBasicMessageBase*>(&msg2);
    comms::dispatchMsgStaticBinSearch<AllMessages>(msg2.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 1U);

    comms::dispatchMsgStaticBinSearch<AllMessages>(msg2.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 2U);

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsgStaticBinSearch<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 3U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90_1);
    comms::dispatchMsgStaticBinSearch<AllMessages>(msg90_1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 3U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90_2);
    comms::dispatchMsgStaticBinSearch<AllMessages>(msg90_2.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 4U);
    REQUIRE(handler.getBaseCount() == 3U);    

    comms::dispatchMsgStaticBinSearch<AllMessages>(msg90_2.doGetId(), 2U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 4U);
    REQUIRE(handler.getBaseCount() == 4U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90_1);
    comms::dispatchMsg<AllMessages>(msg90_1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 5U);
    REQUIRE(handler.getBaseCount() == 4U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90_2);
    comms::dispatchMsg<AllMessages>(msg90_2.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 6U);
    REQUIRE(handler.getBaseCount() == 4U);    

    comms::dispatchMsg<AllMessages>(msg90_2.doGetId(), 2U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 6U);
    REQUIRE(handler.getBaseCount() == 5U);    
}

TEST_CASE("Test26", "test26") 
{
    BeBasicMsg1 msg1;
    BeBasicMsg2 msg2;
    BeBasicMsg3 msg3;

    using AllMessages = 
        std::tuple<
            BeBasicMsg1,
            BeBasicMsg2,
            BeBasicMsg3
        >;

    CountHandler<BeBasicMessageBase> handler;
    auto* interface = static_cast<BeBasicMessageBase*>(&msg1);
    comms::dispatchMsgLinearSwitch<AllMessages>((int)msg1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg2);
    comms::dispatchMsgLinearSwitch<AllMessages>((unsigned)msg2.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsgLinearSwitch<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 0U);    
}

TEST_CASE("Test27", "test27") 
{
    BeBasicMsg1 msg1;
    BeBasicMsg2 msg2;
    BeBasicMsg3 msg3;
    BeBasicMsg90_1 msg90;

    using AllMessages = 
        std::tuple<
            BeBasicMsg1,
            BeBasicMsg2,
            BeBasicMsg90_1
        >;

    static_assert(comms::details::allMessagesAreStrongSorted<AllMessages>(), "Messages are not sorted");

    CountHandler<BeBasicMessageBase> handler;
    auto* interface = static_cast<BeBasicMessageBase*>(&msg1);
    comms::dispatchMsgLinearSwitch<AllMessages>((int)msg1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg2);
    comms::dispatchMsgLinearSwitch<AllMessages>((unsigned)msg2.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 0U);

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsgLinearSwitch<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 1U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90);
    comms::dispatchMsgLinearSwitch<AllMessages>(msg90.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 1U);    
}

TEST_CASE("Test28", "test28") 
{
    BeBasicMsg1 msg1;
    BeBasicMsg2 msg2;
    BeBasicMsg3 msg3;
    BeBasicMsg90_1 msg90_1;
    BeBasicMsg90_2 msg90_2;

    using AllMessages = 
        std::tuple<
            BeBasicMsg1,
            BeBasicMsg2,
            BeBasicMsg90_1,
            BeBasicMsg90_2
        >;

    static_assert(!comms::details::allMessagesAreStrongSorted<AllMessages>(), "Messages are strong sorted, should not be");
    static_assert(comms::details::allMessagesAreWeakSorted<AllMessages>(), "Messages are not sorted");
    REQUIRE(msg90_1.doGetId() == msg90_2.doGetId());

    CountHandler<BeBasicMessageBase> handler;
    auto* interface = static_cast<BeBasicMessageBase*>(&msg1);
    comms::dispatchMsgLinearSwitch<AllMessages>((int)msg1.doGetId(), 0U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 0U);

    comms::dispatchMsgLinearSwitch<AllMessages>((unsigned)msg1.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 1U);

    interface = static_cast<BeBasicMessageBase*>(&msg2);
    comms::dispatchMsgLinearSwitch<AllMessages>(msg2.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 1U);

    comms::dispatchMsgLinearSwitch<AllMessages>(msg2.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 2U);

    interface = static_cast<BeBasicMessageBase*>(&msg3);
    comms::dispatchMsgLinearSwitch<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 3U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90_1);
    comms::dispatchMsgLinearSwitch<AllMessages>(msg90_1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 3U);    

    interface = static_cast<BeBasicMessageBase*>(&msg90_2);
    comms::dispatchMsgLinearSwitch<AllMessages>(msg90_2.doGetId(), 1U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 4U);
    REQUIRE(handler.getBaseCount() == 3U);    

    comms::dispatchMsgLinearSwitch<AllMessages>(msg90_2.doGetId(), 2U, *interface, handler);
    REQUIRE(handler.getCustomCount() == 4U);
    REQUIRE(handler.getBaseCount() == 4U);    
}

TEST_CASE("Test29", "test29") 
{
    using AllMessages = 
        std::tuple<
            BeMsg1,
            BeMsg2,
            BeMsg3
        >;

    using MsgPtr = std::unique_ptr<BeMessageBase>;
    MsgCreationHandler<MsgPtr> handler;
    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType1, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgType<AllMessages>(MessageType1, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgType<AllMessages>(MessageType1, 0U, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgType<AllMessages>(MessageType1, 1U, handler);
        REQUIRE(!result);
    }
    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType2, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg2*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType3, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg3*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }

    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType5, handler);
        REQUIRE(!result);
    }    
}

TEST_CASE("Test30", "test30") 
{
    using AllMessages = 
        std::tuple<
            BeMsg1,
            BeMsg2,
            BeMsg90_1
        >;

    using MsgPtr = std::unique_ptr<BeMessageBase>;
    MsgCreationHandler<MsgPtr> handler;
    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType1, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType2, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg2*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType90, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg90_1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType90, 1U, handler);
        REQUIRE(!result);
    }
    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType3, handler);
        REQUIRE(!result);
    }    
}

TEST_CASE("Test31", "test31") 
{
    using AllMessages = 
        std::tuple<
            BeMsg1,
            BeMsg2,
            BeMsg90_1,
            BeMsg90_2
        >;

    using MsgPtr = std::unique_ptr<BeMessageBase>;
    MsgCreationHandler<MsgPtr> handler;
    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType1, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType2, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg2*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType90, 0U, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg90_1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }

    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType90, 1U, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg90_1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }    

    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType90, 2U, handler);
        REQUIRE(!result);
    }        

    {
        bool result = comms::dispatchMsgTypePolymorphic<AllMessages>(MessageType3, handler);
        REQUIRE(!result);
    }    
}

TEST_CASE("Test32", "test32") 
{
    using AllMessages = 
        std::tuple<
            BeMsg1,
            BeMsg2,
            BeMsg3
        >;

    using MsgPtr = std::unique_ptr<BeMessageBase>;
    MsgCreationHandler<MsgPtr> handler;
    REQUIRE(comms::dispatchMsgTypeCountStaticBinSearch<AllMessages>(MessageType1) == 1U);
    REQUIRE(comms::dispatchMsgTypeCountStaticBinSearch<AllMessages>(MessageType2) == 1U);
    REQUIRE(comms::dispatchMsgTypeCountStaticBinSearch<AllMessages>(MessageType5) == 0U);

    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType1, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType2, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg2*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType3, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg3*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }

    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType5, handler);
        REQUIRE(!result);
    }    
}

TEST_CASE("Test33", "test33") 
{
    using AllMessages = 
        std::tuple<
            BeMsg1,
            BeMsg2,
            BeMsg90_1
        >;
    REQUIRE(comms::dispatchMsgTypeCountStaticBinSearch<AllMessages>(MessageType1) == 1U);
    REQUIRE(comms::dispatchMsgTypeCountStaticBinSearch<AllMessages>(MessageType2) == 1U);
    REQUIRE(comms::dispatchMsgTypeCountStaticBinSearch<AllMessages>(MessageType3) == 0U);
    REQUIRE(comms::dispatchMsgTypeCountStaticBinSearch<AllMessages>(MessageType90) == 1U);

    using MsgPtr = std::unique_ptr<BeMessageBase>;
    MsgCreationHandler<MsgPtr> handler;
    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType1, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgType<AllMessages>(MessageType1, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgType<AllMessages>(MessageType1, 0U, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgType<AllMessages>(MessageType1, 1U, handler);
        REQUIRE(!result);
    }    
    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType2, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg2*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType90, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg90_1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType90, 1U, handler);
        REQUIRE(!result);
    }
    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType3, handler);
        REQUIRE(!result);
    }    

    {
        bool result = comms::dispatchMsgType<AllMessages>(MessageType5, handler);
        REQUIRE(!result);
    }    
}

TEST_CASE("Test34", "test34") 
{
    using AllMessages = 
        std::tuple<
            BeMsg1,
            BeMsg2,
            BeMsg90_1,
            BeMsg90_2
        >;

    REQUIRE(comms::dispatchMsgTypeCountStaticBinSearch<AllMessages>(MessageType1) == 1U);
    REQUIRE(comms::dispatchMsgTypeCountStaticBinSearch<AllMessages>(MessageType2) == 1U);
    REQUIRE(comms::dispatchMsgTypeCountStaticBinSearch<AllMessages>(MessageType3) == 0U);
    REQUIRE(comms::dispatchMsgTypeCountStaticBinSearch<AllMessages>(MessageType90) == 2U);

    using MsgPtr = std::unique_ptr<BeMessageBase>;
    MsgCreationHandler<MsgPtr> handler;
    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType1, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType2, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg2*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType90, 0U, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg90_1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }

    {
        bool result = comms::dispatchMsgType<AllMessages>(MessageType90, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg90_1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }

    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType90, 1U, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg90_1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }    

    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType90, 2U, handler);
        REQUIRE(!result);
    }        

    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType90, 3U, handler);
        REQUIRE(!result);
    }        

    {
        bool result = comms::dispatchMsgTypeStaticBinSearch<AllMessages>(MessageType3, handler);
        REQUIRE(!result);
    }    

    {
        bool result = comms::dispatchMsgType<AllMessages>(MessageType5, handler);
        REQUIRE(!result);
    }    
}

TEST_CASE("Test35", "test35") 
{
    using AllMessages = 
        std::tuple<
            BeMsg1,
            BeMsg2,
            BeMsg3
        >;

    using MsgPtr = std::unique_ptr<BeMessageBase>;
    MsgCreationHandler<MsgPtr> handler;
    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType1, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType2, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg2*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType3, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg3*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }

    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType5, handler);
        REQUIRE(!result);
    }    
}

TEST_CASE("Test36", "test36") 
{
    using AllMessages = 
        std::tuple<
            BeMsg1,
            BeMsg2,
            BeMsg90_1
        >;

    using MsgPtr = std::unique_ptr<BeMessageBase>;
    MsgCreationHandler<MsgPtr> handler;
    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType1, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType2, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg2*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType90, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg90_1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType90, 1U, handler);
        REQUIRE(!result);
    }
    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType3, handler);
        REQUIRE(!result);
    }    
}

TEST_CASE("Test37", "test37") 
{
    using AllMessages = 
        std::tuple<
            BeMsg1,
            BeMsg2,
            BeMsg90_1,
            BeMsg90_2
        >;

    using MsgPtr = std::unique_ptr<BeMessageBase>;
    MsgCreationHandler<MsgPtr> handler;
    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType1, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType2, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg2*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }
    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType90, 0U, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg90_1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }

    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType90, 1U, handler);
        REQUIRE(result);
        auto msg = handler.getMsg();
        auto* castedMsg = reinterpret_cast<BeMsg90_1*>(msg.get());
        REQUIRE(castedMsg != nullptr);
    }    

    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType90, 2U, handler);
        REQUIRE(!result);
    }        

    {
        bool result = comms::dispatchMsgTypeLinearSwitch<AllMessages>(MessageType3, handler);
        REQUIRE(!result);
    }    
}

TEST_CASE("Test38", "test38") 
{
    BeRefreshableMsg9 msg;
    REQUIRE(msg.doGetId() == MessageType9);
    REQUIRE(msg.length() == 1U);
    REQUIRE(msg.name() == Message9Str);

    msg.field_f1().field_str().value() = "hello";
    REQUIRE(msg.refresh());
    REQUIRE(msg.length() == 6U);

    std::vector<std::uint8_t> outData;
    outData.resize(msg.length());
    BeRefreshableMsg4::WriteIterator writeIter = &outData[0];

    internalWriteReadTest(msg, writeIter, outData.size());

    static const std::uint8_t ExpBuf[] =  {
        5, 'h', 'e', 'l', 'l', 'o'
    };
    static const std::size_t ExpBufSize = std::extent<decltype(ExpBuf)>::value;
    REQUIRE(outData.size() == ExpBufSize);
    REQUIRE(std::equal(outData.begin(), outData.end(), std::begin(ExpBuf)));
}

TEST_CASE("Test39", "test39") 
{
    BeBasicMsg1 msg1;
    BeBasicMsg2 msg2;
    BeBasicMsg3 msg3;
    BeBasicMsg90_1 msg90;

    using AllMessages =
        std::tuple<
            BeBasicMsg1,
            BeBasicMsg2,
            BeBasicMsg3
        >;

    CountHandler<BeBasicMessageBase> handler;
    using Dispatcher1 = comms::MsgDispatcher<>;
    static_assert(Dispatcher1::isDispatchPolymorphic<AllMessages>(), "Invalid dispatch");
    auto* interface = static_cast<BeBasicMessageBase*>(&msg1);
    Dispatcher1::dispatch<AllMessages>(msg1.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 1U);
    REQUIRE(handler.getBaseCount() == 0U);

    using Dispatcher2 = comms::MsgDispatcher<comms::option::ForceDispatchPolymorphic>;
    static_assert(Dispatcher2::isDispatchPolymorphic<AllMessages>(), "Invalid dispatch");
    interface = static_cast<BeBasicMessageBase*>(&msg2);
    Dispatcher2::dispatch<AllMessages>(msg2.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 2U);
    REQUIRE(handler.getBaseCount() == 0U);

    using Dispatcher3 = comms::MsgDispatcher<comms::option::ForceDispatchStaticBinSearch>;
    static_assert(Dispatcher3::isDispatchStaticBinSearch<AllMessages>(), "Invalid dispatch");
    interface = static_cast<BeBasicMessageBase*>(&msg3);
    Dispatcher3::dispatch<AllMessages>(msg3.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 0U);

    using Dispatcher4 = comms::MsgDispatcher<comms::option::ForceDispatchLinearSwitch>;
    static_assert(Dispatcher4::isDispatchLinearSwitch<AllMessages>(), "Invalid dispatch");
    interface = static_cast<BeBasicMessageBase*>(&msg90);
    Dispatcher4::dispatch<AllMessages>(msg90.doGetId(), *interface, handler);
    REQUIRE(handler.getCustomCount() == 3U);
    REQUIRE(handler.getBaseCount() == 1U);
}
