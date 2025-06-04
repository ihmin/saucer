#pragma once

#include "data.hpp"

#include "args/args.hpp"
#include "../executor.hpp"

#include <memory>
#include <functional>

#include <variant>
#include <expected>

#include <string>
#include <string_view>

#include <format>

#include <coco/promise/promise.hpp>

namespace saucer
{
    struct serializer_core
    {
        using parse_result = std::variant<std::unique_ptr<function_data>, std::unique_ptr<result_data>, std::monostate>;
        using executor     = saucer::executor<std::string_view>;

      public:
        template <typename T>
        using result = std::expected<T, std::string>;

      public:
        using resolver = std::move_only_function<void(std::unique_ptr<result_data>)>;
        using function = std::move_only_function<void(std::unique_ptr<function_data>, executor)>;

      public:
        virtual ~serializer_core() = default;

      public:
        [[nodiscard]] virtual std::string script() const                 = 0;
        [[nodiscard]] virtual std::string js_serializer() const          = 0;
        [[nodiscard]] virtual parse_result parse(std::string_view) const = 0;
    };

    namespace impl
    {
        template <typename T, typename Interface>
        struct is_writable;

        template <typename T>
        struct is_serializer;
    } // namespace impl

    template <typename T, typename Interface>
    concept Writable = impl::is_writable<T, Interface>::value;

    template <typename T, typename Interface, typename V = std::string>
    concept Readable = requires(V value) {
        { Interface::template read<T>(value) } -> std::same_as<serializer_core::result<T>>;
    };

    template <typename T>
    concept Serializer = impl::is_serializer<T>::value;

    template <typename T>
    concept Interface = requires() {
        requires std::derived_from<typename T::result_data, result_data>;
        requires std::derived_from<typename T::function_data, function_data>;
        requires Writable<int, T>;
        requires Readable<int, T>;
        requires Readable<int, T, typename T::result_data>;
        requires Readable<int, T, typename T::function_data>;
    };

    template <typename Interface>
    struct serializer : serializer_core
    {
        serializer();

      public:
        template <typename T>
        static auto convert(T &&);

        template <Readable<Interface> T>
        static auto resolve(coco::promise<T>);

      public:
        template <Writable<Interface> T>
        static auto serialize(T &&);
    };

    template <Serializer Serializer, typename... Ts>
    using format_string = std::format_string<std::enable_if_t<Writable<Ts, Serializer>, std::string>...>;
} // namespace saucer

#include "serializer.inl"
