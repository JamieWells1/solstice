#ifndef RESOLUTION_H
#define RESOLUTION_H

#include <expected>
#include <string>

namespace resolution
{

template <typename T, typename E = std::string>
class Resolution
{
    template <typename U, typename E2>
    friend class Resolution;

    template <typename U>
    friend auto ok(U value);

    template <typename Err>
    friend auto err(Err error);

   private:
    std::expected<T, E> m_data;

    static Resolution ok(T value) { return Resolution(std::move(value)); }
    static Resolution err(E err) { return Resolution(std::unexpected(std::move(err))); }

   public:
    Resolution(T value) : m_data(std::move(value)) {}
    Resolution(std::unexpected<E> err) : m_data(std::move(err)) {}

    bool has_value() const { return m_data.has_value(); }

    // Bool overload
    operator bool() const { return has_value(); }

    T& operator*() { return *m_data; }
    E& error() { return m_data.error(); }

    // const variations
    const T& operator*() const { return *m_data; }
    const E& error() const { return m_data.error(); }

    T value_or(T default_value) const { return has_value() ? *m_data : default_value; }

    template <typename Func>
    auto and_then(Func func) -> decltype(func(std::declval<T&>()))
    {
        if (has_value())
        {
            return func(*m_data);
        }

        using ReturnType = decltype(func(std::declval<T&>()));
        return ReturnType::err(error());
    }

    template <typename Func>
    auto map(Func func) -> Resolution<decltype(func(std::declval<T&>())), E>
    {
        if (has_value())
        {
            // Transform value
            return Resolution<decltype(func(std::declval<T&>())), E>::ok(func(*m_data));
        }
        // Pass error through
        return Resolution<decltype(func(std::declval<T&>())), E>::err(error());
    }
};

template <typename T>
auto ok(T value)
{
    return Resolution<T>::ok(std::move(value));
}

template <typename E>
auto err(E error)
{
    return std::unexpected(std::string(std::move(error)));
}

}  // namespace resolution

#endif  // RESOLUTION_H
