#pragma once

#include <format>
#include <source_location>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace dracon {

    // Defer -----------------------------------------------------------------

#define CONCAT_(x, y) x##y
#define CONCAT(x, y)  CONCAT_(x, y)

#ifdef __cpp_lib_experimental_scope
#    if __cpp_lib_experimental_scope >= 201902
#        include <experimental/scope>
#        define DEFER(Function) CONCAT(scope_exit_, __COUNTER__) = std::experimental::scope_exit(Function)
#    else
#        define DRACON_DEFER_NEEDED
#    endif
#else
#    define DRACON_DEFER_NEEDED
#endif

#ifdef DRACON_DEFER_NEEDED

    template <class FType> class Defer {
      private:
        FType func;

      public:
        inline Defer(FType f) : func{f} { }
        inline Defer(const Defer & other) = delete;
        inline Defer(Defer && other)      = default;
        inline auto operator=(const Defer & other) -> Defer & = delete;
        inline auto operator=(Defer && other) -> Defer &      = default;
        inline ~Defer() { func(); }
    };

#    define DEFER(Function) dracon::Defer CONCAT(Defer_, __COUNTER__) = dracon::Defer(Function)

#endif

    // End Defer -------------------------------------------------------------

    // Variant Utils ---------------------------------------------------------

    template <class... Ts> struct overloads : Ts... {
        using Ts::operator()...;
    };

    // End Variant Utils -----------------------------------------------------

    // Bit Tools -------------------------------------------------------------

    template <class To, class From>
    inline auto bcast(const From & from)
      -> std::enable_if<sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From> && std::is_trivially_copyable_v<To>,
                        To> {
        static_assert(std::is_trivially_constructible_v<To>,
                      "bcast requires destination type to be trivially constructible");
        To dest;
        memcpy(&dest, &from, sizeof(To));
        return dest;
    }

    // End Bit Tools ---------------------------------------------------------

    // Error Tools -----------------------------------------------------------

    class UnreachableException : public std::runtime_error {
      public:
        explicit inline UnreachableException(const std::source_location location = std::source_location::current()) :
          std::runtime_error(std::format("Unreachable Exception: {}({}:{}) `{}`",
                                         location.file_name(),
                                         location.line(),
                                         location.column(),
                                         location.function_name())) { }

        explicit inline UnreachableException(const std::string &        msg,
                                             const std::source_location location = std::source_location::current()) :
          std::runtime_error(std::format("Unreachable Exception: {}({}:{}) `{}`\n{}",
                                         location.file_name(),
                                         location.line(),
                                         location.column(),
                                         location.function_name(),
                                         msg)) { }
    };

#define UNREACHABLE       throw UnreachableException();
#define UNREACHABLEM(msg) throw UnreachableException(msg);

    // End Error Tools -------------------------------------------------------

} // namespace dracon
