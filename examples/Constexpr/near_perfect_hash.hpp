#pragma once

#include <initializer_list>
#include <limits.h>
#include <utility>
#include <algorithm>
#include <array>
#include <stdint.h>
#include <string.h>
#include <stdexcept>
#include "cx_helpers.hpp"

#ifdef USE_CONSTEXPR_FOR_HASH
#define CONSTEXPR_FOR_HASH constexpr
#define CONSTEXPR_OR_CONST_FOR_HASH constexpr
#else
#define CONSTEXPR_FOR_HASH
#define CONSTEXPR_OR_CONST_FOR_HASH const
#endif

template <typename ENUM, size_t S>
struct EnumValueNameConstruct
{
    cx_string<S> string;
    const char * stringPtr;
    ENUM value;

    CONSTEXPR_FOR_HASH EnumValueNameConstruct(const char (&s)[S], ENUM value): string(s), stringPtr(s), value(value)
    {
    }
};

template <typename ENUM>
struct  EnumValueName {
    const char * enumName_;
    size_t strLen_;
    ENUM enumVal_;

    template <size_t S>
    CONSTEXPR_FOR_HASH EnumValueName(const EnumValueNameConstruct<ENUM, S> c) :
        enumName_(c.stringPtr), strLen_(c.string.size()), enumVal_(c.value)
    {
    }

    CONSTEXPR_FOR_HASH EnumValueName(const char * enumName_, const size_t strLen_, const ENUM enumVal_):
        enumName_(enumName_),
        strLen_(strLen_),
        enumVal_(enumVal_)
    {
    }

    template <size_t strLen_>
    CONSTEXPR_FOR_HASH EnumValueName(const char (&enumName_)[strLen_], const ENUM enumVal_):
        enumName_(enumName_),
        strLen_(strLen_-1),
        enumVal_(enumVal_)
    {
    }


    template <size_t strLen_>
    CONSTEXPR_FOR_HASH EnumValueName(const ENUM enumVal_, const char (&enumName_)[strLen_]):
        enumName_(enumName_),
        strLen_(strLen_-1),
        enumVal_(enumVal_)
    {
    }


    CONSTEXPR_FOR_HASH EnumValueName() : enumName_(""), strLen_(0), enumVal_(static_cast<ENUM>(0))
    {
    }

    CONSTEXPR_FOR_HASH EnumValueName(const EnumValueName& o) : enumName_(o.enumName_), strLen_(o.strLen_), enumVal_(o.enumVal_)
    {
    }

    CONSTEXPR_FOR_HASH EnumValueName(EnumValueName&& o) : enumName_(std::move(o.enumName_)), strLen_(std::move(o.strLen_)),enumVal_(std::move(o.enumVal_))
    {
    }

    CONSTEXPR_FOR_HASH EnumValueName& operator=(const EnumValueName& o)
    {
        enumName_ = o.enumName_;
        strLen_ = o.strLen_;
        enumVal_ = o.enumVal_;

        return *this;
    }

    CONSTEXPR_FOR_HASH EnumValueName& operator=(EnumValueName&& o)
    {
        enumName_ = std::move(o.enumName_);
        strLen_ = std::move(o.strLen_);
        enumVal_ = std::move(o.enumVal_);

        return *this;
    }
};




template <typename ENUM, size_t STRLENGTH>
CONSTEXPR_FOR_HASH inline EnumValueNameConstruct<ENUM,STRLENGTH> make_enum_name(ENUM enumVal_,
        const char (&enumName_)[STRLENGTH])
{
    static_assert(STRLENGTH > 1, "Minimum String Length of 1 required");

    return { enumName_, enumVal_ };
}


template <typename ENUM, size_t SIZE>
class NearPerfectHash
{
    using initializer_iter = const EnumValueName<ENUM>*;
    using association  = int16_t;
    static CONSTEXPR_OR_CONST_FOR_HASH int chosen_modulo {((SIZE << 4) + 1)};
    static CONSTEXPR_OR_CONST_FOR_HASH int chosen_size {(SIZE << 4) + 2};
    using  selector_type = typename std::conditional<chosen_size < 256, uint8_t, uint16_t>::type;
    using hashitem = cx_pair<selector_type, association>;



protected:
    const uint32_t uiFnvOffsetBasis = 0x811c9dc5;
    const uint32_t uiFnvPrime = 16777219;
    const uint16_t NoSuchBucket = 65535;

    cx_hash hashCalcs;
    const EnumValueName<ENUM> array[SIZE];
    const initializer_iter iiBegin, iiEnd;
    size_t collisions {0}, uniqueBuckets{0}, reusedBuckets{0};
    uint8_t selI{0}, selJ{0};
    cx_array<cx_pair<uint16_t,uint8_t>, chosen_size> cheatTable;
    cx_array<hashitem, SIZE> hashTable;
public:
    template <typename...C>
    CONSTEXPR_FOR_HASH NearPerfectHash(const C&... w):
        hashCalcs(w.string...),
        array{w...},
        iiBegin(std::begin(array)),
        iiEnd(std::end(array)),
        cheatTable( {
        cx_pair<uint16_t, uint8_t>(0,0)
    }),
    hashTable(ComputeHash())
    {
        static_assert(SIZE == sizeof...(w), "Mismatched argument count");
    }


    CONSTEXPR_FOR_HASH auto GetCollisions() const
    {
        return collisions;
    }

    CONSTEXPR_FOR_HASH auto GetUniqueBuckets() const
    {
        return uniqueBuckets;
    }

    CONSTEXPR_FOR_HASH auto GetReusedBuckets() const
    {
        return reusedBuckets;
    }

    const auto& GetValues() const
    {
        return array;
    }

    template <size_t S>
    CONSTEXPR_FOR_HASH ENUM Convert(const char (&name)[S], ENUM defaultValue) const
    {
        size_t strLen = S - 1;
        uint32_t hash = uiFnvOffsetBasis;

        for (int i = 0; i < hashCalcs.hash_size; ++i)
        {
            auto pos = hashCalcs.scounts[i].second;
            auto ch = (pos < strLen ? name[hashCalcs.scounts[i].second] : 0);

            hash ^= ch;
            hash *= uiFnvPrime;
        }

        hash ^= name[strLen - 1];
        hash *= uiFnvPrime;
        hash ^= strLen;
        hash *= uiFnvPrime;
        hash = hash % chosen_modulo;

        auto& u = cheatTable[hash];
        auto  ul = u.first;
        auto  uh = u.first + u.second;

        if (ul != NoSuchBucket)
        {
            while (ul < uh)
            {
                if (strcmp(array[hashTable[ul].second].enumName_, name) == 0)
                    return array[hashTable[ul].second].enumVal_;
                ++ul;
            }
        }

        return defaultValue;

    }

    ENUM Convert(const std::string& name, ENUM defaultValue) const
    {
        uint32_t hash = uiFnvOffsetBasis;

        for (int i = 0; i < hashCalcs.hash_size; ++i)
        {
            auto pos = hashCalcs.scounts[i].second;
            auto ch = (pos < name.size() ? name[hashCalcs.scounts[i].second] : 0);

            hash ^= ch;
            hash *= uiFnvPrime;
        }

        hash ^= name[name.size()-1];
        hash *= uiFnvPrime;
        hash ^= name.size();
        hash *= uiFnvPrime;
        hash = hash % chosen_modulo;

        auto& u = cheatTable[hash];
        auto  ul = u.first;
        auto  uh = u.first + u.second;

        if (ul != NoSuchBucket)
        {
            while (ul < uh)
            {
                if (strcmp(array[hashTable[ul].second].enumName_, name.c_str()) == 0)
                    return array[hashTable[ul].second].enumVal_;
                ++ul;
            }
        }

        return defaultValue;
    }

private:
    CONSTEXPR_FOR_HASH cx_array<hashitem, SIZE>  ComputeHash()
    {
        cx_array<hashitem, SIZE> hashTable {};

        int hashTableSize = 0;

        for (auto hi = iiBegin; hi != iiEnd; ++hi)
        {
            uint32_t hash = uiFnvOffsetBasis;

            for (int i = 0; i < hashCalcs.hash_size; ++i)
            {
                auto pos = hashCalcs.scounts[i].second;
                auto ch = (pos < hi->strLen_ ? hi->enumName_[hashCalcs.scounts[i].second] : 0);
                hash ^= ch;
                hash *= uiFnvPrime;
            }

            hash ^= hi->enumName_[hi->strLen_-1];
            hash *= uiFnvPrime;
            hash ^= hi->strLen_;
            hash *= uiFnvPrime;
            hash = hash % chosen_modulo;

            hashTable[hashTableSize++] = hashitem(hash, hi-iiBegin);
        }

        struct
        {
            CONSTEXPR_FOR_HASH int operator()(const hashitem& a, const hashitem& b) const
            {
                return a.first - b.first;
            }
        } sorter{};


        cx_sort(hashTable, 0, hashTableSize - 1, sorter);

        cx_fill(cheatTable, cx_pair<uint16_t, uint8_t>(NoSuchBucket, 0));

        int counter = 0;
        for (const auto& hte : hashTable)
        {
            if (cheatTable[hte.first].first==NoSuchBucket)
            {
                cheatTable[hte.first] = cx_pair<uint16_t,uint8_t>(counter, 1);
                ++uniqueBuckets;
            } else {
                if (cheatTable[hte.first].second==1)
                    ++reusedBuckets;
                ++cheatTable[hte.first].second;
                ++collisions;
            }

            ++counter;
        }

        return hashTable;
    }


};

