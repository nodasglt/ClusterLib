#ifndef __UTIL_MAYBE_HPP__
#define __UTIL_MAYBE_HPP__

#include <iostream>
#include <stdexcept>

template<typename T>
class Maybe
{
public:
    Maybe () : mIsJust(false) {}

    Maybe (const Maybe& other)
    {
        if (other)
        {
            new(&mMemBlock) T(*other);
            mIsJust = true;
        }
        else
        {
            mIsJust = false;
        }
    }

    Maybe (Maybe&& other)
    {
        if (other)
        {
            new(&mMemBlock) T(std::move(*other));
            mIsJust = true;
        }
        else
        {
            mIsJust = false;
        }
    }

    Maybe& operator= (Maybe other)
    {
        if (*this)
        {
            reinterpret_cast<const T*>(&mMemBlock)->~T();
            mIsJust = false;
        }

        if (other)
        {
            new(&mMemBlock) T(std::move(*other));
            mIsJust = true;
        }

        return *this;
    }

    Maybe (const T& value)
    {
        new(&mMemBlock) T(value);
        mIsJust = true;
    }

    Maybe (T&& value)
    {
        new(&mMemBlock) T(std::move(value));
        mIsJust = true;
    }

    template<typename ...Args>
    Maybe (Args&&... args)
    {
        new(&mMemBlock) T(std::forward<Args>(args)...);
        mIsJust = true;
    }

    bool isJust () const
    {
        return mIsJust;
    }

    operator bool () const
    {
        return isJust();
    }

    const T& operator* () const
    {
        if (!*this) throw std::logic_error("Bad access: Maybe type has no value");

        return *reinterpret_cast<const T*>(&mMemBlock);
    }

    T& operator* ()
    {
        if (!*this) throw std::logic_error("Bad access: Maybe type has no value");

        return *reinterpret_cast<T*>(&mMemBlock);
    }

    const T& operator-> () const
    {
        return **this;
    }

    T& operator-> ()
    {
        return **this;
    }

    const T& operator| (const T& value) const
    {
        return (*this) ? **this : value;
    }

    T& operator| (T& value)
    {
        return (*this) ? **this : value;
    }

    const T& operator| (const Maybe& other) const
    {
        return (*this) ? **this : *other;
    }

    T& operator| (Maybe& other)
    {
        return (*this) ? **this : *other;
    }

    ~Maybe ()
    {
        if (*this)
            reinterpret_cast<T*>(&mMemBlock)->~T();
    }

private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type mMemBlock;
    bool mIsJust;
};

#endif /* end of include guard: __UTIL_MAYBE_HPP__ */
