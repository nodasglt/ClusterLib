#ifndef __ArrayView__
#define __ArrayView__

#include "Array.hpp"

template<class T>
class ArrayView
{
public:
    constexpr ArrayView() : mSize(0), mData(nullptr) {}
    constexpr ArrayView(ArrayView const&) = default;
    constexpr ArrayView(ArrayView&&) = default;

    template<unsigned N>
    ArrayView(T const (& a)[N]) : mSize(N), mData(N > 0 ? std::addressof(a[0]) : nullptr)
    {
        static_assert(N > 0, "Zero-mSize array is not permitted in ISO C++.");
    }

    ArrayView(Array<T> const& a) : mSize(a.getLength()), mData(a.getLength() ? &a[0] : nullptr) {}

    constexpr ArrayView(T const* a, unsigned const n) : mSize(n), mData(a) {}

    ArrayView(std::initializer_list<T> const& l) : mSize(l.size()), mData(std::begin(l)) {}

    ArrayView& operator=(ArrayView const&) = delete;
    ArrayView& operator=(ArrayView &&) = delete;

    constexpr const T* begin() const noexcept
    {
        return mData;
    }
    constexpr const T* end() const noexcept
    {
        return mData + mSize;
    }

    constexpr unsigned getLength() const noexcept
    {
        return mSize;
    }
    constexpr const T& operator[](unsigned const n) const noexcept
    {
        return *(mData + n);
    }
    constexpr const T& at(unsigned const n) const
    {
        return (n >= mSize)
            ? throw std::out_of_range("ArrayView::at()")
            : *(mData + n);
    }

private:
    const unsigned mSize;
    const T* const mData;
};

template<typename T, unsigned N>
ArrayView<T> asArrayView(T const (& a)[N])
{
    return { a };
}

#endif /* end of include guard: __ArrayView__ */
