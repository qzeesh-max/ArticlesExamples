#pragma once

#ifdef NO_CONSTEXPR
#define CX_CONSTEXPR
#else
#define CX_CONSTEXPR constexpr
#endif

template <typename T, size_t S>
struct cx_array_impl
{
private:
    T value[S] {};

public:
    CX_CONSTEXPR T* operator+(intptr_t offset)
    {
        return value + offset;
    }

    CX_CONSTEXPR const T* operator+(intptr_t offset) const
    {
        return value + offset;
    }

    CX_CONSTEXPR cx_array_impl()
    {
    }
};

template <typename T, size_t S>
struct cx_array
{
    cx_array_impl<T, S> elems;


public:
    CX_CONSTEXPR cx_array(std::initializer_list<T> a)
    {
        auto i = 0;

        for (auto& b : a)
        {
            *(elems + i) = b;

            ++i;
        }
    }

    CX_CONSTEXPR cx_array(const cx_array<T,S>& a)
    {
        auto i = 0;

        for (auto b : a)
        {
            *(elems + i) = b;

            ++i;
        }
    }

    CX_CONSTEXPR cx_array()
    {
    }

    CX_CONSTEXPR T* begin()
    {
        return elems + 0;
    }

    CX_CONSTEXPR T* end()
    {
        return elems + S;
    }

    CX_CONSTEXPR const T* begin() const
    {
        return elems + 0;
    }

    CX_CONSTEXPR const T* end() const
    {
        return elems + S;
    }


    CX_CONSTEXPR T& operator[](size_t i)
    {
        return *(elems + i);
    }

    CX_CONSTEXPR const T& operator[](size_t i) const
    {
        return *(elems + i);
    }

    CX_CONSTEXPR T* operator+(int i)
    {
        return elems +  i;
    }

    CX_CONSTEXPR const T* operator+(int i) const
    {
        return elems + i;
    }
};

template <typename K, typename V>
struct cx_pair
{
    K first;
    V second;

    CX_CONSTEXPR cx_pair() : first(K()), second(V())
    {
    }

    CX_CONSTEXPR cx_pair(K k, V v): first(k), second(v)
    {
    }

    CX_CONSTEXPR cx_pair(const cx_pair& o) : first(o.first), second(o.second)
    {
    }

    CX_CONSTEXPR cx_pair(cx_pair&& o) : first(std::move(o.first)), second(std::move(o.second))
    {
    }

    CX_CONSTEXPR cx_pair& operator=(const cx_pair& o)
    {
        first = o.first;
        second = o.second;

        return *this;
    }

    CX_CONSTEXPR cx_pair& operator=(cx_pair&& o)
    {
        first = std::move(o.first);
        second = std::move(o.second);

        return *this;
    }
};



template <typename T>
inline CX_CONSTEXPR void cx_swap(T& a, T& b)
{
    T temp(a);

    a = b;
    b = temp;
}

template <typename T, typename U>
inline CX_CONSTEXPR void cx_fill(T& a, const U& b)
{
    for (auto& i : a)
    {
        i = b;
    }
}


template<typename T, size_t N, typename LAMBDA>
inline CX_CONSTEXPR void cx_sort(cx_array<T, N>& arr, int left, int right, const LAMBDA& compare)
{
    int i = left, j = right;
    auto pivot = arr[(left + right) / 2];

    while (i <= j) {
        while (compare(arr[i], pivot) < 0)
            i++;
        while (compare(arr[j], pivot) > 0)
            j--;
        if (i <= j)
        {
            cx_swap(arr[i], arr[j]);
            i++;
            j--;
        }
    };

    if (left < j)
        cx_sort(arr, left, j, compare);
    if (i < right)
        cx_sort(arr, i, right, compare);
}


template <typename STATE, typename LAMBDA>
struct cx_continuation
{
    STATE& state_ {};
    const LAMBDA& lambda_ {};
    bool done_ {false};

    CX_CONSTEXPR cx_continuation(STATE& state_, const LAMBDA& lambda_) : state_(state_), lambda_(lambda_)
    {
    }


    CX_CONSTEXPR cx_continuation<STATE,LAMBDA>& operator()()
    {
        if (!done_)
        {
            if ((lambda_)(state_))
                return *this;
        }

        done_ = true;

        return *this;
    }
};

template <typename STATE, typename LAMBDA>
inline CX_CONSTEXPR cx_continuation<STATE, LAMBDA>& cx_invoke(cx_continuation<STATE, LAMBDA>& state, int invokeDepth = 0)
{
    return state.done_ ? state : (invokeDepth == 512 ? state :
                                  cx_invoke(cx_invoke(state(), invokeDepth+1)(),invokeDepth+1));
}

template<typename T>
inline ptrdiff_t CX_CONSTEXPR cx_distance(T a, T b)
{
    return a - b;
}

template <size_t S>
struct cx_string
{
    cx_array<char, S> chars {};

    CX_CONSTEXPR cx_string(const char (&s)[S])
    {
        int i = 0;

        for (auto a : s)
        {
            chars[i] = a;
            ++i;
        }
    }

    CX_CONSTEXPR auto size() const
    {
        return S - 1;
    }

    CX_CONSTEXPR auto begin()
    {
        return chars.begin();
    }

    CX_CONSTEXPR auto begin() const
    {
        return chars.begin();
    }

    CX_CONSTEXPR auto end()
    {
        return chars.end();
    }

    CX_CONSTEXPR auto end() const
    {
        return chars.end();
    }
};


class cx_hash
{
    using phistogram_chars = cx_array<cx_array<int, 256>,256> ;
    using phistogram_unique = cx_array<int, 256>;
    using sorted_counts = cx_array<cx_pair<int16_t, int16_t>, 256>;

public:
    size_t min_len { ~(size_t)0 }, max_len { 0 }, average_len { 0 }, count { 0 };
    size_t hash_size { 0 };
    sorted_counts scounts {};
    bool result;

public:
    template<size_t...N>
    CX_CONSTEXPR cx_hash(const cx_string<N>&... w) : result ( do_(w...) )
    {

    }

    template<size_t...N>
    CX_CONSTEXPR bool do_(const cx_string<N>&... w)
    {
        phistogram_unique unique_pos {};
        auto hash_chars = get_hash_chars(unique_pos, w...);
        int c = 0, f = 0;

        for (auto  a : unique_pos)
        {
            if (a)
            {
                cx_pair<int16_t, int16_t> p (a, c);
                scounts[f] = p;
                ++f;
            }
            ++c;
        }

        struct
        {
            constexpr int operator()(const cx_pair<int16_t, int16_t>& a, const cx_pair<int16_t, int16_t>& b) const
            {
                return b.first - a.first;
            }
        } sort_order {};

        cx_sort(scounts, 0, f-1, sort_order);

        int below_min_count = 0, below_avg_count = 0, index = 0;

        for (auto& b : scounts)
        {
            if (!b.first)
                break;

            if (b.second <= min_len)
                ++below_min_count;
            else if (b.second <= average_len)
                ++below_avg_count;

            ++index;

            if ((below_min_count >0) && ((double)average_len / min_len) <= ((double)(below_min_count+below_avg_count) / below_min_count))
            {
                break;
            }

        }

        hash_size = std::min(index + 1, f-1);

        return true;
    }


    template<size_t...N>
    CX_CONSTEXPR phistogram_chars get_hash_chars(phistogram_unique &unique_pos, const cx_string<N>&... w)
    {
        phistogram_chars result;

        get_hash_chars(unique_pos, result, w...);

        return result;
    }


    template<size_t F, size_t...N>
    CX_CONSTEXPR void get_hash_chars(phistogram_unique &unique_pos, phistogram_chars& result, const cx_string<F>& f, const cx_string<N>&... w)
    {
        auto p = 0;
        for (const auto& c : f)
        {
            if (c)
            {
                if (result[p][c]==0)
                    ++unique_pos[p];
                ++result[p][c];
                ++p;
            }
        }

        min_len = std::min(F-1, min_len);
        max_len = std::max(F-1, max_len);
        ++count;
        average_len += F-1;

        get_hash_chars(unique_pos, result, w...);
    }

    template<size_t F>
    CX_CONSTEXPR void get_hash_chars(phistogram_unique &unique_pos, phistogram_chars& result, const cx_string<F>& f)
    {
        auto p = 0;
        for (const auto& c : f)
        {
            if (c)
            {
                if (result[p][c]==0)
                    ++unique_pos[p];
                ++result[p][c];
                ++p;
            }
        }

        min_len = std::min(F-1, min_len);
        max_len = std::max(F-1, max_len);
        ++count;
        average_len += F-1;
        average_len = std::min(average_len / count + 1, max_len);
    }
};

