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


#pragma once

#include <algorithm>
#include <iterator>
#include <string>

#include "comms/Assert.h"
#include "StaticVector.h"

namespace comms
{

namespace util
{

namespace details
{

template <typename TChar>
class StaticStringBase
{
    typedef StaticVectorBase<TChar> VecType;
    typedef typename VecType::CellType CellType;
protected:

    static const auto npos = std::string::npos;

    StaticStringBase(TChar* buf, std::size_t cap)
      : vec_(reinterpret_cast<CellType*>(buf), cap)
    {
        endString();
    }

    void assign(std::size_t count, TChar ch)
    {
        GASSERT(count <= capacity());
        auto countLimit = std::min(count, capacity());
        vec_.clear();
        std::fill_n(std::back_inserter(vec_), countLimit, ch);
        endString();
    }

    void assign(const StaticStringBase& other)
    {
        assign(other, 0, other.size());
    }

    void assign(const StaticStringBase& other, std::size_t pos, std::size_t count)
    {
        GASSERT(&other != this);
        auto updatedCount = std::min(other.size() - pos, count);
        auto countLimit = std::min(updatedCount, capacity());
        vec_.clear();
        std::copy_n(other.cbegin() + pos, countLimit, std::back_inserter(vec_));
        endString();
    }

    void assign(const TChar* str, std::size_t count)
    {
        vec_.clear();
        auto countLimit = std::min(count, capacity());
        while ((*str != Ends) && (vec_.size() < countLimit)) {
            vec_.push_back(*str);
            ++str;
        }
        endString();
    }

    void assign(const TChar* str)
    {
        assign(str, capacity());
    }

    template <typename TIter>
    void assign(TIter first, TIter last)
    {
        vec_.assign(first, last);
        endString();
    }

    TChar& at(std::size_t pos)
    {
        GASSERT(pos < size());
        return operator[](pos);
    }

    const TChar& at(std::size_t pos) const
    {
        GASSERT(pos < size());
        return operator[](pos);
    }

    TChar& operator[](std::size_t pos)
    {
        return vec_[pos];
    }

    const TChar& operator[](std::size_t pos) const
    {
        return vec_[pos];
    }

    TChar& front()
    {
        GASSERT(!empty());
        return vec_.front();
    }

    const TChar& front() const
    {
        GASSERT(!empty());
        return vec_.front();
    }

    TChar& back()
    {
        GASSERT(!empty());
        return vec_[size() - 1];
    }

    const TChar& back() const
    {
        GASSERT(!empty());
        return vec_[size() - 1];
    }

    const TChar* data() const
    {
        GASSERT(!vec_.empty());
        return vec_.data();
    }

    TChar* begin()
    {
        return vec_.begin();
    }

    const TChar* cbegin() const
    {
        return vec_.cbegin();
    }

    TChar* end()
    {
        return begin() + size();
    }

    const TChar* cend() const
    {
        return cbegin() + size();
    }

    bool empty() const
    {
        return size() == 0;
    }

    std::size_t size() const
    {
        GASSERT(!vec_.empty());
        return vec_.size() - 1;
    }

    std::size_t capacity() const
    {
        return vec_.capacity() - 1;
    }

    void clear()
    {
        vec_.clear();
        endString();
    }

    void insert(std::size_t idx, std::size_t count, TChar ch)
    {
        GASSERT(idx < size());
        vec_.insert(vec_.begin() + idx, count, ch);
    }

    void insert(std::size_t idx, const TChar* str)
    {
        auto endStr = str;
        while (*endStr != Ends) {
            ++endStr;
        }

        GASSERT(idx < size());
        vec_.insert(vec_.begin() + idx, str, endStr);
    }

    void insert(std::size_t idx, const TChar* str, std::size_t count)
    {
        GASSERT(idx < size());
        auto endStr = str + count;
        vec_.insert(vec_.begin() + idx, str, endStr);
    }

    void insert(std::size_t idx, const StaticStringBase& other)
    {
        GASSERT(idx < size());
        vec_.insert(vec_.begin() + idx, other.cbegin(), other.cend());
    }

    void insert(std::size_t idx, const StaticStringBase& str, std::size_t str_idx, std::size_t count)
    {
        GASSERT(idx < size());
        GASSERT(str_idx < str.size());
        auto begIter = str.cbegin() + str_idx;
        auto endIter = begIter + std::min((str.size() - str_idx), count);
        vec_.insert(vec_.begin() + idx, begIter, endIter);
    }

    TChar* insert(const TChar* pos, TChar ch)
    {
        return vec_.insert(pos, ch);
    }

    TChar* insert(const TChar* pos, std::size_t count, TChar ch)
    {
        return vec_.insert(pos, count, ch);
    }

    template <typename TIter>
    TChar* insert(const TChar* pos, TIter first, TIter last)
    {
        return vec_.insert(pos, first, last);
    }

    void erase(std::size_t idx, std::size_t count)
    {
        GASSERT(idx < size());
        auto begIter = begin() + idx;
        auto endIter = begIter + std::min(count, size() - idx);
        vec_.erase(begIter, endIter);
        GASSERT(!vec_.empty()); // Must contain '\0'
    }

    TChar* erase(const TChar* pos)
    {
        return vec_.erase(pos);
    }

    TChar* erase(const TChar* first, const TChar* last)
    {
        return vec_.erase(first, last);
    }

    void push_back(TChar ch)
    {
        GASSERT((size() < capacity()) && (!"The string is full."));
        vec_.insert(end(), ch);
    }

    void pop_back()
    {
        GASSERT((!empty()) && (!"The string is empty."));
        vec_.erase(end() - 1);
    }

    int compare(
        std::size_t pos1,
        std::size_t count1,
        const StaticStringBase& other,
        std::size_t pos2,
        std::size_t count2) const
    {
        GASSERT(pos1 <= size());
        GASSERT(pos2 <= other.size());
        count1 = std::min(count1, size() - pos1);
        count2 = std::min(count2, other.size() - pos2);
        auto minCount = std::min(count1, count2);
        for (auto idx = 0; idx < minCount; ++idx) {
            auto thisCh = (*this)[pos1 + idx];
            auto otherCh = other[pos2 + idx];
            auto diff = static_cast<int>(thisCh) - static_cast<int>(otherCh);
            if (diff != 0) {
                return diff;
            }
        }

        return static_cast<int>(count1) - static_cast<int>(count2);
    }

    int compare(std::size_t pos, std::size_t count, const TChar* str) const
    {
        GASSERT(pos <= size());
        count = std::min(count, size() - pos);
        for (auto idx = 0U; idx < count; ++idx) {
            auto ch = (*this)[pos + idx];
            auto diff = static_cast<int>(ch) - static_cast<int>(*str);
            if (diff != 0) {
                return diff;
            }

            if (*str == Ends) {
                return 1;
            }
            ++str;
        }

        if (*str != Ends) {
            return 0 - static_cast<int>(*str);
        }

        return 0;
    }

    int compare(
        std::size_t pos1,
        std::size_t count1,
        const char* str,
        std::size_t count2) const
    {
        GASSERT(pos1 <= size());
        count1 = std::min(count1, size() - pos1);
        auto minCount = std::min(count1, count2);
        for (auto idx = 0; idx < minCount; ++idx) {
            auto thisCh = (*this)[pos1 + idx];
            auto diff = static_cast<int>(thisCh) - static_cast<int>(*str);
            if (diff != 0) {
                return diff;
            }

            ++str;
        }

        return static_cast<int>(count1) - static_cast<int>(count2);
    }

    template <typename TIter>
    void replace(
        const TChar* first,
        const TChar* last,
        TIter first2,
        TIter last2)
    {
        GASSERT(first <= end());
        GASSERT(last <= end());
        GASSERT(first <= last);
        for (auto iter = first; iter != last; ++iter) {
            if (first2 == last2) {
                vec_.erase(iter, last);
                return;
            }

            *iter = static_cast<TChar>(*first2);
            ++first2;
        }

        vec_.insert(last, first2, last2);
    }

    void replace(
        const TChar* first,
        const TChar* last,
        const TChar* str)
    {
        GASSERT(first <= end());
        GASSERT(last <= end());
        GASSERT(first <= last);
        for (auto iter = first; iter != last; ++iter) {
            if (*str == Ends) {
                vec_.erase(iter, last);
                return;
            }

            *iter = *str;
            ++str;
        }

        auto remCapacity = capacity() - size();
        auto endStr = str + remCapacity;
        auto lastStrIter = std::find(str, endStr, Ends);
        vec_.insert(last, str, lastStrIter);
    }

    void replace(
        const TChar* first,
        const TChar* last,
        std::size_t count2,
        TChar ch)
    {
        GASSERT(first <= end());
        GASSERT(last <= end());
        GASSERT(first <= last);
        auto dist = static_cast<std::size_t>(std::distance(first, last));
        auto fillDist = std::min(dist, count2);
        std::fill_n(first, fillDist, ch);
        if (count2 <= dist) {
            vec_.erase(first + fillDist, last);
            return;
        }

        vec_.insert(last, count2 - fillDist, ch);
    }

    std::size_t copy(TChar* dest, std::size_t count, std::size_t pos) const
    {
        GASSERT(pos <= size());
        count = std::min(count, size() - pos);
        std::copy_n(cbegin() + pos, count, dest);
        return count;
    }

    void resize(std::size_t count)
    {
        resize(count, Ends);
    }

    void resize(std::size_t count, TChar ch)
    {
        if (count <= size()) {
            auto remCount = size() - count;
            vec_.erase(cbegin() + remCount, end());
            GASSERT(vec_[size()] == Ends);
            return;
        }

        vec_.insert(end(), count - size(), ch);
    }

    void swap(StaticStringBase& other)
    {
        vec_.swap(other.vec_);
    }

    std::size_t find(const TChar* str, std::size_t pos, std::size_t count) const
    {
        GASSERT(pos <= size());
        auto remCount = size() - pos;
        if (remCount < count) {
            return npos;
        }

        auto maxPos = size() - count;
        for (auto idx = pos; idx < maxPos; ++idx) {
            auto thisStrBeg = &vec_[idx];
            auto thisStrEnd = thisStrBeg + count;
            if (std::equal(thisStrBeg, thisStrEnd, str, str + count)) {
                return idx;
            }
        }
        return npos;
    }

    std::size_t find(const TChar* str, std::size_t pos) const
    {
        GASSERT(pos <= size());
        auto maxStrCount = size() - pos;
        auto maxStrEnd = str + maxStrCount;
        auto iter = std::find(str, maxStrEnd, Ends);
        if (iter == maxStrEnd) {
            return npos;
        }

        auto strCount = static_cast<std::size_t>(std::distance(str, iter));
        return find(str, pos, strCount);
    }

    std::size_t find(TChar ch, std::size_t pos) const
    {
        GASSERT(pos <= size());
        auto begIter = cbegin() + pos;
        auto iter = std::find(begIter, cend(), ch);
        if (iter == cend()) {
            return npos;
        }

        return static_cast<std::size_t>(std::distance(begIter, iter));
    }

    std::size_t rfind(const TChar* str, std::size_t pos, std::size_t count) const
    {
        pos = std::min(pos, size());
        if (pos < count) {
            return npos;
        }

        for (auto idx = static_cast<int>(pos - count); 0 <= idx; --idx) {
            auto thisStrBeg = &vec_[idx];
            auto thisStrEnd = thisStrBeg + count;
            if (std::equal(thisStrBeg, thisStrEnd, str, str + count)) {
                return static_cast<std::size_t>(idx);
            }
        }
        return npos;
    }

    std::size_t rfind(const TChar* str, std::size_t pos) const
    {
        pos = std::min(pos, size());
        auto maxStrCount = pos;
        auto maxStrEnd = str + maxStrCount;
        auto iter = std::find(str, maxStrEnd, Ends);
        if (iter == maxStrEnd) {
            return npos;
        }

        auto strCount = static_cast<std::size_t>(std::distance(str, iter));
        return rfind(str, pos, strCount);
    }

    std::size_t rfind(TChar ch, std::size_t pos) const
    {
        pos = std::min(pos, size());
        auto begIter = std::reverse_iterator<const TChar*>(cbegin() + pos);
        auto endIter = std::reverse_iterator<const TChar*>(cbegin());
        auto iter = std::find(begIter, endIter, ch);
        if (iter == endIter) {
            return npos;
        }

        return static_cast<std::size_t>(std::distance(iter, endIter));
    }

    std::size_t find_first_of(const TChar* str, std::size_t pos, std::size_t count) const
    {
        pos = std::min(pos, size());
        auto endStr = str + count;
        for (auto iter = str; iter != endStr; ++iter) {
            auto foundPos = find(*iter, pos);
            if (foundPos != npos) {
                return foundPos;
            }
        }

        return npos;
    }

    std::size_t find_first_of(const TChar* str, std::size_t pos) const
    {
        pos = std::min(pos, size());
        while (*str != Ends) {
            auto foundPos = find(*str, pos);
            if (foundPos != npos) {
                return foundPos;
            }
            ++str;
        }
        return npos;
    }

    std::size_t find_first_not_of(const TChar* str, std::size_t pos, std::size_t count) const
    {
        pos = std::min(pos, size());
        auto endStr = str + count;
        for (auto iter = cbegin() + pos; iter != cend(); ++iter) {
            auto found = std::none_of(str, endStr,
                [iter](TChar ch) -> bool
                {
                    return *iter == ch;
                });

            if (found) {
                return static_cast<std::size_t>(std::distance(cbegin(), iter));
            }
        }

        return npos;
    }

    std::size_t find_first_not_of(const TChar* str, std::size_t pos) const
    {
        auto* strTmp = str;
        while (*strTmp != Ends) {
            ++strTmp;
        }
        auto count = static_cast<std::size_t>(std::distance(str, strTmp));
        return find_first_not_of(str, pos, count);
    }

    std::size_t find_first_not_of(TChar ch, std::size_t pos) const
    {
        pos = std::min(pos, size());
        auto iter = std::find_if(cbegin() + pos, cend(),
            [ch](TChar nextCh) -> bool
            {
                return ch != nextCh;
            });

        if (iter == cend()) {
            return npos;
        }

        return static_cast<std::size_t>(std::distance(cbegin(), iter));
    }

    std::size_t find_last_of(const TChar* str, std::size_t pos, std::size_t count) const
    {
        pos = std::min(pos, size());
        auto endStr = str + count;
        for (auto iter = str; iter != endStr; ++iter) {
            auto foundPos = rfind(*iter, pos);
            if (foundPos != npos) {
                return foundPos;
            }
        }

        return npos;
    }

    std::size_t find_last_of(const TChar* str, std::size_t pos) const
    {
        pos = std::min(pos, size());
        while (*str != Ends) {
            auto foundPos = rfind(*str, pos);
            if (foundPos != npos) {
                return foundPos;
            }
            ++str;
        }
        return npos;
    }

    std::size_t find_last_not_of(const TChar* str, std::size_t pos, std::size_t count) const
    {
        pos = std::min(pos, size());
        auto endStr = str + count;
        auto begIter = std::reverse_iterator<const TChar*>(cbegin() + pos);
        auto endIter = std::reverse_iterator<const TChar*>(cbegin());
        for (auto iter = begIter; iter != endIter; ++iter) {
            auto found = std::none_of(str, endStr,
                [iter](TChar ch) -> bool
                {
                    return *iter == ch;
                });

            if (found) {
                return static_cast<std::size_t>(std::distance(iter, endIter));
            }
        }

        return npos;
    }

    std::size_t find_last_not_of(const TChar* str, std::size_t pos) const
    {
        auto* strTmp = str;
        while (*strTmp != Ends) {
            ++strTmp;
        }
        auto count = static_cast<std::size_t>(std::distance(str, strTmp));
        return find_last_not_of(str, pos, count);
    }

    std::size_t find_last_not_of(TChar ch, std::size_t pos) const
    {
        pos = std::min(pos, size());
        auto begIter = std::reverse_iterator<const TChar*>(cbegin() + pos);
        auto endIter = std::reverse_iterator<const TChar*>(cbegin());
        auto iter = std::find_if(begIter, endIter,
            [ch](TChar nextCh) -> bool
            {
                return ch != nextCh;
            });

        if (iter == endIter) {
            return npos;
        }

        return static_cast<std::size_t>(std::distance(iter, endIter));
    }


    // HERE

private:
    void endString()
    {
        vec_.push_back(TChar(Ends));
    }

    static const auto Ends = static_cast<TChar>('\0');
    StaticVectorBase<TChar> vec_;
};

template <typename TChar, std::size_t TSize>
struct StaticStringStorageBase
{
    typedef std::array<TChar, TSize> StorageType;
    StorageType data_;
};


}  // namespace details

template <std::size_t TSize, typename TChar = char>
class StaticString :
    public details::StaticStringStorageBase<TChar, TSize>,
    public details::StaticStringBase<TChar>
{
    typedef details::StaticStringStorageBase<TChar, TSize> StorageBase;
    typedef details::StaticStringBase<TChar> Base;

public:
    typedef TChar value_type;
    typedef std::size_t size_type;
    typedef typename StorageBase::StorageType::difference_type difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    static const auto npos = Base::npos;

    StaticString()
      : Base(&StorageBase::data_[0], StorageBase::data_.size())
    {
    }

    StaticString(size_type count, value_type ch)
      : Base(&StorageBase::data_[0], StorageBase::data_.size())
    {
        assign(count, ch);
    }

    template <std::size_t TOtherSize>
    StaticString(
        const StaticString<TOtherSize, TChar>& other,
        size_type pos,
        size_type count = npos)
      : Base(&StorageBase::data_[0], StorageBase::data_.size())
    {
        assign(other, pos, count);
    }

    StaticString(const_pointer str, size_type count)
      : Base(&StorageBase::data_[0], StorageBase::data_.size())
    {
        assign(str, count);
    }

    StaticString(const_pointer str)
      : Base(&StorageBase::data_[0], StorageBase::data_.size())
    {
        assign(str);
    }

    StaticString(const StaticString& other)
      : Base(&StorageBase::data_[0], StorageBase::data_.size())
    {
        assign(other);
    }

    template <std::size_t TOtherSize>
    explicit StaticString(const StaticString<TOtherSize, TChar>& other)
      : Base(&StorageBase::data_[0], StorageBase::data_.size())
    {
        assign(other);
    }

    StaticString& operator=(const StaticString& other)
    {
        return assign(other);
    }

    template <std::size_t TOtherSize>
    StaticString& operator=(const StaticString<TOtherSize, TChar>& other)
    {
        return assign(other);
    }

    StaticString& operator=(const_pointer str)
    {
        return assign(str);
    }

    StaticString& operator=(value_type ch)
    {
        return assign(1, ch);
    }

    StaticString& assign(size_type count, value_type ch)
    {
        Base::assign(count, ch);
        return *this;
    }

    template <typename TOtherSize>
    StaticString& assign(const StaticString& other)
    {
        if (&other != this) {
            Base::assign(other);
        }
        return *this;
    }

    template <std::size_t TOtherSize>
    StaticString& assign(const StaticString<TOtherSize, TChar>& other)
    {
        Base::assign(other);
        return *this;
    }

    template <std::size_t TOtherSize>
    StaticString& assign(
        const StaticString<TOtherSize, TChar>& other,
        size_type pos,
        size_type count = npos)
    {
        Base::assign(other, pos, count);
        return *this;
    }

    StaticString& assign(const_pointer str, size_type count)
    {
        Base::assign(str, count);
        return *this;
    }

    StaticString& assign(const_pointer str)
    {
        Base::assign(str);
        return *this;
    }

    template <typename TIter>
    StaticString& assign(TIter first, TIter last)
    {
        Base::assign(first, last);
        return *this;
    }

    reference at(size_type pos)
    {
        return Base::at(pos);
    }

    const_reference at(size_type pos) const
    {
        return Base::at(pos);
    }

    reference operator[](size_type pos)
    {
        return Base::operator[](pos);
    }

    const_reference operator[](size_type pos) const
    {
        return Base::operator[](pos);
    }

    reference front()
    {
        return Base::front();
    }

    const_reference front() const
    {
        return Base::front();
    }

    reference back()
    {
        return Base::back();
    }

    const_reference back() const
    {
        return Base::back();
    }

    const_pointer data() const
    {
        return Base::data();
    }

    const_pointer c_str() const
    {
        return data();
    }

    iterator begin()
    {
        return Base::begin();
    }

    const_iterator begin() const
    {
        return cbegin();
    }

    const_iterator cbegin() const
    {
        return Base::cbegin();
    }

    iterator end()
    {
        return Base::end();
    }

    const_iterator end() const
    {
        return cend();
    }

    const_iterator cend() const
    {
        return Base::cend();
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const
    {
        return crbegin();
    }

    const_reverse_iterator crbegin() const
    {
        return reverse_iterator(cend());
    }

    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const
    {
        return crend();
    }

    const_reverse_iterator crend() const
    {
        return reverse_iterator(cbegin());
    }

    bool empty() const
    {
        return Base::empty();
    }

    size_type size() const
    {
        return Base::size();
    }

    size_type length() const
    {
        return size();
    }

    size_type max_size() const
    {
        return capacity();
    }

    void reserve(size_type)
    {
    }

    size_type capacity() const
    {
        return Base::capacity();
    }

    void shrink_to_fit()
    {
    }

    void clear()
    {
        Base::clear();
    }

    StaticString& insert(size_type idx, size_type count, value_type ch)
    {
        Base::insert(idx, count, ch);
        return *this;
    }

    StaticString& insert(size_type idx, const_pointer str)
    {
        Base::insert(idx, str);
        return *this;
    }

    StaticString& insert(size_type idx, const_pointer str, size_type count)
    {
        Base::insert(idx, str, count);
        return *this;
    }

    template <std::size_t TAnySize>
    StaticString& insert(size_type idx, const StaticString<TAnySize, TChar>& str)
    {
        Base::insert(idx, str);
        return *this;
    }

    template <std::size_t TAnySize>
    StaticString& insert(
        size_type idx,
        const StaticString<TAnySize, TChar>& str,
        size_type str_idx,
        size_type count = npos)
    {
        Base::insert(idx, str, str_idx, count);
        return *this;
    }

    iterator insert(const_iterator pos, value_type ch)
    {
        return Base::insert(pos, ch);
    }

    iterator insert(const_iterator pos, size_type count, value_type ch)
    {
        return Base::insert(pos, count, ch);
    }

    template <typename TIter>
    iterator insert(const_iterator pos, TIter first, TIter last)
    {
        return Base::insert(pos, first, last);
    }

    StaticString& erase(std::size_t idx, std::size_t count = npos)
    {
        Base::erase(idx, count);
        return *this;
    }

    iterator erase(const_iterator pos)
    {
        return Base::erase(pos);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        return Base::erase(first, last);
    }


    void push_back(value_type ch)
    {
        Base::push_back(ch);
    }

    void pop_back()
    {
        Base::pop_back();
    }

    StaticString& append(size_type count, value_type ch)
    {
        return insert(size(), count, ch);
    }

    template <std::size_t TAnySize>
    StaticString& append(const StaticString<TAnySize, TChar>& other)
    {
        return insert(size(), other);
    }

    template <std::size_t TAnySize>
    StaticString& append(
        const StaticString<TAnySize, TChar>& other,
        size_type pos,
        size_type count = npos)
    {
        return insert(size(), other, pos, count);
    }

    StaticString& append(const TChar* str, size_type count = npos)
    {
        return insert(size(), str, count);
    }

    StaticString& append(const TChar* str)
    {
        return insert(size(), str);
    }

    template <typename TIter>
    StaticString& append(TIter first, TIter last)
    {
        insert(end(), first, last);
        return *this;
    }

    template <std::size_t TAnySize>
    StaticString& operator+=(const StaticString<TAnySize, TChar>& other)
    {
        return append(other);
    }

    StaticString& operator+=(value_type ch)
    {
        return append(1U, ch);
    }

    StaticString& operator+=(const_pointer str)
    {
        return append(str);
    }

    template <std::size_t TAnySize>
    int compare(const StaticString<TAnySize, TChar>& other) const
    {
        return compare(0, size(), other);
    }

    template <std::size_t TAnySize>
    int compare(
        size_type pos,
        size_type count,
        const StaticString<TAnySize, TChar>& other) const
    {
        return compare(pos, count, other, 0, other.size());
    }

    template <std::size_t TAnySize>
    int compare(
        size_type pos1,
        size_type count1,
        const StaticString<TAnySize, TChar>& other,
        size_type pos2,
        size_type count2 = npos) const
    {
        return Base::compare(pos1, count1, other, pos2, count2);
    }

    int compare(const_pointer str) const
    {
        return compare(0, size(), str);
    }

    int compare(size_type pos, size_type count, const_pointer str) const
    {
        return Base::compare(pos, count, str);
    }

    int compare(size_type pos, size_type count1, const_pointer str, size_type count2) const
    {
        return Base::compare(pos, count1, str, count2);
    }

    template <std::size_t TAnySize>
    StaticString& replace(
        size_type pos,
        size_type count,
        const StaticString<TAnySize, TChar>& other)
    {
        GASSERT(pos <= size());
        auto begIter = begin() + pos;
        auto remCount = static_cast<std::size_t>(std::distance(begIter, end()));
        auto endIter = begIter + std::min(count, remCount);
        return replace(begIter, endIter, other.begin(), other.end());
    }

    template <std::size_t TAnySize>
    StaticString& replace(
        const_iterator first,
        const_iterator last,
        const StaticString<TAnySize, TChar>& other)
    {
        return replace(first, last, other.begin(), other.end());
    }

    template <std::size_t TAnySize>
    StaticString& replace(
        size_type pos,
        size_type count,
        const StaticString<TAnySize, TChar>& other,
        size_type pos2,
        size_type count2 = npos)
    {
        GASSERT(pos <= size());
        auto begIter = begin() + pos;
        auto remCount = static_cast<std::size_t>(std::distance(begIter, end()));
        auto endIter = begIter + std::min(count, remCount);

        GASSERT(pos2 <= other.size());
        auto begIter2 = other.begin() + pos2;
        auto remCount2 = static_cast<std::size_t>(std::distance(begIter2, other.end()));
        auto endIter2 = begIter2 + std::min(count2, remCount2);

        return replace(begIter, endIter, begIter2, endIter2);
    }

    template <typename TIter>
    StaticString& replace(
        const_iterator first,
        const_iterator last,
        TIter first2,
        TIter last2)
    {
        Base::replace(first, last, first2, last2);
        return *this;
    }

    StaticString& replace(
        size_type pos,
        size_type count,
        const_pointer str,
        size_type count2)
    {
        GASSERT(pos <= size());
        auto begIter = cbegin() + pos;
        auto endIter = std::min(count, size() - pos);
        return replace(begIter, endIter, str, str + count2);
    }

    StaticString& replace(
        const_iterator first,
        const_iterator last,
        const_pointer str,
        size_type count2)
    {
        return replace(first, last, str, str + count2);
    }

    StaticString& replace(
        size_type pos,
        size_type count,
        const_pointer str)
    {
        GASSERT(pos <= size());
        auto begIter = cbegin() + pos;
        auto endIter = std::min(count, size() - pos);
        return replace(begIter, endIter, str);
    }

    StaticString& replace(
        const_iterator first,
        const_iterator last,
        const_pointer str)
    {
        Base::replace(first, last, str);
        return *this;
    }

    StaticString& replace(
        size_type pos,
        size_type count,
        size_type count2,
        value_type ch)
    {
        GASSERT(pos <= size());
        auto begIter = cbegin() + pos;
        auto endIter = std::min(count, size() - pos);
        return replace(begIter, endIter, count2, ch);
    }

    StaticString& replace(
        const_iterator first,
        const_iterator last,
        size_type count2,
        value_type ch)
    {
        Base::replace(first, last, count2, ch);
        return *this;
    }

    StaticString substr(size_type pos = 0, size_type count = npos) const
    {
        GASSERT(pos <= size());
        auto begIter = cbegin() + pos;
        auto endIter = begIter + std::min(count, size() - pos);
        return StaticString(cbegin() + pos, endIter);
    }

    size_type copy(pointer dest, size_type count, size_type pos = 0) const
    {
        return Base::copy(dest, count, pos);
    }

    void resize(size_type count)
    {
        Base::resize(count);
    }

    void resize(size_type count, value_type ch)
    {
        Base::resize(count, ch);
    }

    template <std::size_t TAnySize>
    void swap(StaticString<TAnySize, TChar>& other)
    {
        Base::swap(other);
    }

    template <std::size_t TAnySize>
    size_type find(const StaticString<TAnySize, TChar>& str, size_type pos = 0) const
    {
        GASSERT(pos <= size());
        return find(str.cbegin(), pos, str.size());
    }

    size_type find(const_pointer str, size_type pos, size_type count) const
    {
        return Base::find(str, pos, count);
    }

    size_type find(const_pointer str, size_type pos = 0) const
    {
        return Base::find(str, pos);
    }

    size_type find(value_type ch, size_type pos = 0) const
    {
        return Base::find(ch, pos);
    }

    template <std::size_t TAnySize>
    size_type rfind(const StaticString<TAnySize, TChar>& str, size_type pos = npos) const
    {
        GASSERT(pos <= size());
        return rfind(str.cbegin(), pos, str.size());
    }

    size_type rfind(const_pointer str, size_type pos, size_type count) const
    {
        return Base::rfind(str, pos, count);
    }

    size_type rfind(const_pointer str, size_type pos = npos) const
    {
        return Base::rfind(str, pos);
    }

    size_type rfind(value_type ch, size_type pos = npos) const
    {
        return Base::rfind(ch, pos);
    }

    template <std::size_t TAnySize>
    size_type find_first_of(const StaticString<TAnySize, TChar>& str, size_type pos = 0) const
    {
        GASSERT(pos <= size());
        return find_first_of(str.cbegin(), pos, str.size());
    }

    size_type find_first_of(const_pointer str, size_type pos, size_type count) const
    {
        return Base::find_first_of(str, pos, count);
    }

    size_type find_first_of(const_pointer str, size_type pos = 0) const
    {
        return Base::find_first_of(str, pos);
    }

    size_type find_first_of(value_type ch, size_type pos = 0) const
    {
        return find(ch, pos);
    }

    template <std::size_t TAnySize>
    size_type find_first_not_of(const StaticString<TAnySize, TChar>& str, size_type pos = 0) const
    {
        GASSERT(pos <= size());
        return find_first_not_of(str.cbegin(), pos, str.size());
    }

    size_type find_first_not_of(const_pointer str, size_type pos, size_type count) const
    {
        return Base::find_first_not_of(str, pos, count);
    }

    size_type find_first_not_of(const_pointer str, size_type pos = 0) const
    {
        return Base::find_first_not_of(str, pos);
    }

    size_type find_first_not_of(value_type ch, size_type pos = 0) const
    {
        return Base::find_first_not_of(ch, pos);
    }

    template <std::size_t TAnySize>
    size_type find_last_of(const StaticString<TAnySize, TChar>& str, size_type pos = npos) const
    {
        GASSERT(pos <= size());
        return find_last_of(str.cbegin(), pos, str.size());
    }

    size_type find_last_of(const_pointer str, size_type pos, size_type count) const
    {
        return Base::find_last_of(str, pos, count);
    }

    size_type find_last_of(const_pointer str, size_type pos = npos) const
    {
        return Base::find_last_of(str, pos);
    }

    size_type find_last_of(value_type ch, size_type pos = npos) const
    {
        return rfind(ch, pos);
    }

    template <std::size_t TAnySize>
    size_type find_last_not_of(const StaticString<TAnySize, TChar>& str, size_type pos = npos) const
    {
        GASSERT(pos <= size());
        return find_last_not_of(str.cbegin(), pos, str.size());
    }

    size_type find_last_not_of(const_pointer str, size_type pos, size_type count) const
    {
        return Base::find_last_not_of(str, pos, count);
    }

    size_type find_last_not_of(const_pointer str, size_type pos = npos) const
    {
        return Base::find_last_not_of(str, pos);
    }

    size_type find_last_not_of(value_type ch, size_type pos = npos) const
    {
        return Base::find_last_not_of(ch, pos);
    }

};

template <std::size_t TSize1, std::size_t TSize2, typename TChar>
bool operator<(const StaticString<TSize1, TChar>& str1, const StaticString<TSize2, TChar>& str2)
{
    return std::lexicographical_compare(str1.begin(), str1.end(), str2.begin(), str2.end());
}

template <std::size_t TSize1, std::size_t TSize2, typename TChar>
bool operator<=(const StaticString<TSize1, TChar>& str1, const StaticString<TSize2, TChar>& str2)
{
    return !(str2 < str1);
}

template <std::size_t TSize1, std::size_t TSize2, typename TChar>
bool operator>(const StaticString<TSize1, TChar>& str1, const StaticString<TSize2, TChar>& str2)
{
    return (str2 < str1);
}

template <std::size_t TSize1, std::size_t TSize2, typename TChar>
bool operator>=(const StaticString<TSize1, TChar>& str1, const StaticString<TSize2, TChar>& str2)
{
    return !(str1 < str2);
}

template <std::size_t TSize1, std::size_t TSize2, typename TChar>
bool operator==(const StaticString<TSize1, TChar>& str1, const StaticString<TSize2, TChar>& str2)
{
    return
        (str1.size() == str2.size()) &&
        std::equal(str1.begin(), str1.end(), str2.begin());
}

}  // namespace util

}  // namespace comms


