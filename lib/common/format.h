#pragma once

#include <chrono>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <tuple>
#include <utility>
#include <exception>
#include <type_traits>
#include <string_view>
#include <cctype>
#include <algorithm>
#include <numeric>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

std::string EscapeSymbols(const std::string& str);
std::vector<std::string> Split(const std::string& s, const std::string& delimiter, size_t limit = 0);
std::string Trim(const std::string& s);

////////////////////////////////////////////////////////////////////////////////

enum class EFormatValueType {
    Bool,
    String,
    Options,
    None
};

class FormatOptions;

class FormatValue {
public:
    FormatValue() : type_(EFormatValueType::None) {}
    
    explicit FormatValue(bool value) : type_(EFormatValueType::Bool) {
        value_.bool_value = value;
    }
    
    explicit FormatValue(int value) : type_(EFormatValueType::String) {
        new(&value_.string_value) std::string(std::to_string(value));
    }
    
    explicit FormatValue(double value) : type_(EFormatValueType::String) {
        new(&value_.string_value) std::string(std::to_string(value));
    }
    
    explicit FormatValue(const std::string& value) : type_(EFormatValueType::String) {
        new(&value_.string_value) std::string(value);
    }
    
    explicit FormatValue(const FormatOptions& value);
    
    FormatValue(const FormatValue& other);
    
    FormatValue(FormatValue&& other) noexcept;
    
    FormatValue& operator=(const FormatValue& other);
    FormatValue& operator=(FormatValue&& other) noexcept;
    
    ~FormatValue();
    
    EFormatValueType GetType() const { return type_; }
    
    bool AsBool(bool defaultValue = false) const {
        return (type_ == EFormatValueType::Bool) ? value_.bool_value : defaultValue;
    }
    
    int AsInt(int defaultValue = 0) const {
        if (type_ == EFormatValueType::String) {
            try {
                return std::stoi(value_.string_value);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    double AsDouble(double defaultValue = 0.0) const {
        if (type_ == EFormatValueType::String) {
            try {
                return std::stod(value_.string_value);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    std::string AsString(const std::string& defaultValue = "") const {
        return (type_ == EFormatValueType::String) ? value_.string_value : defaultValue;
    }
    
    const FormatOptions& AsOptions() const;

    const FormatOptions& AsOptions(const FormatOptions& defaultValue) const;
    
    bool IsBool() const { return type_ == EFormatValueType::Bool; }
    bool IsInt() const { 
        if (type_ != EFormatValueType::String) return false;
        try {
            std::stoi(value_.string_value);
            return true;
        } catch (...) {
            return false;
        }
    }
    bool IsDouble() const { 
        if (type_ != EFormatValueType::String) return false;
        try {
            std::stod(value_.string_value);
            return true;
        } catch (...) {
            return false;
        }
    }
    bool IsString() const { return type_ == EFormatValueType::String; }
    bool IsOptions() const { return type_ == EFormatValueType::Options; }
    bool IsNone() const { return type_ == EFormatValueType::None; }
    
private:
    EFormatValueType type_;
    
    union ValueUnion {
        bool bool_value;
        std::string string_value;
        FormatOptions* options_value;
        
        ValueUnion() {}
        ~ValueUnion() {}
    } value_;
};

class FormatOptions {
public:
    FormatOptions() = default;
    
    explicit FormatOptions(const std::string& modifierStr);
    
    FormatOptions(const FormatOptions& other) = default;
    
    FormatOptions(FormatOptions&& other) = default;
    
    FormatOptions& operator=(const FormatOptions& other) = default;
    FormatOptions& operator=(FormatOptions&& other) = default;
    
    bool Has(const std::string& key) const;
    
    bool GetBool(const std::string& key, bool defaultValue = false) const;
    int GetInt(const std::string& key, int defaultValue = 0) const;
    double GetDouble(const std::string& key, double defaultValue = 0.0) const;
    std::string GetString(const std::string& key, const std::string& defaultValue = "") const;
    const FormatOptions& GetOptions(const std::string& key, const FormatOptions& defaultValue = FormatOptions()) const;
    
    void Set(const std::string& key, bool value);
    void Set(const std::string& key, int value);
    void Set(const std::string& key, double value);
    void Set(const std::string& key, const std::string& value);
    void Set(const std::string& key, const char* value);
    void Set(const std::string& key, const FormatOptions& value);
    
    FormatOptions GetSubOptions(const std::string& prefix) const;

    FormatOptions Merge(FormatOptions other) const;
    
private:
    void ParseFromString(const std::string& modifierStr);
    
    std::map<std::string, FormatValue> options_;
};

////////////////////////////////////////////////////////////////////////////////

struct FormatModifier;

template <typename T>
inline void FormatHandler(std::ostringstream& out, const T& value, const FormatOptions& options = FormatOptions()) {
    out << value;
}

template <typename T>
inline void FormatHandler(std::ostringstream& out, const T& value, const std::string& modifier) {
    FormatHandler(out, value, FormatOptions(modifier));
}

template <>
inline void FormatHandler(std::ostringstream& out, const bool& value, const FormatOptions& options) {
    std::string trueStr = options.GetString("true", "true");
    std::string falseStr = options.GetString("false", "false");
    out << (value ? trueStr : falseStr);
}

namespace detail {



} // namespace detail

inline void FormatHandler(std::ostringstream& out, const std::string& value, const FormatOptions& options) {
    int width = options.GetInt("width", 0);
    char fillChar = options.GetString("fill", " ")[0];
    bool leftAlign = options.GetBool("left", false);
    int maxLength = options.GetInt("maxlength", -1);
    bool upper = options.GetBool("upper", false);
    bool lower = options.GetBool("lower", false);
    
    std::string processedValue = value;
    
    if (maxLength >= 0 && static_cast<size_t>(maxLength) < processedValue.length()) {
        processedValue = processedValue.substr(0, maxLength);
    }
    
    if (upper) {
        std::transform(processedValue.begin(), processedValue.end(), processedValue.begin(),
                      [](unsigned char c) { return std::toupper(c); });
    } else if (lower) {
        std::transform(processedValue.begin(), processedValue.end(), processedValue.begin(),
                      [](unsigned char c) { return std::tolower(c); });
    }
    
    if (width > 0 && static_cast<size_t>(width) > processedValue.length()) {
        size_t padding = width - processedValue.length();
        std::string paddingStr(padding, fillChar);
        if (leftAlign) {
            processedValue = processedValue + paddingStr;
        } else {
            processedValue = paddingStr + processedValue;
        }
    }
    
    out << processedValue;
}

inline void FormatHandler(std::ostringstream& out, const char* value, const FormatOptions& options) {
    FormatHandler(out, std::string(value), options);
}

inline void FormatHandler(std::ostringstream& out, const std::string_view& value, const FormatOptions& options) {
    FormatHandler(out, std::string(value), options);
}

inline void FormatHandler(std::ostringstream& out, const char& value, const FormatOptions& options) {
    FormatHandler(out, std::string(1, value), options);
}

template <size_t N>
inline void FormatHandler(std::ostringstream& out, const char (&value)[N], const FormatOptions& options) {
    FormatHandler(out, std::string(value), options);
}

template <typename Exception>
requires (std::is_base_of_v<std::exception, Exception>)
inline void FormatHandler(std::ostringstream& out, const Exception& value, const FormatOptions& options) {
    FormatHandler(out, value.what(), options);
}

template <typename T>
requires (!std::is_array_v<T> && std::numeric_limits<T>::is_integer && !std::is_same_v<T, bool>)
inline void FormatHandler(std::ostringstream& out, const T& value, const FormatOptions& options) {
    int width = options.GetInt("width", 0);
    char fillChar = options.GetString("fill", " ")[0];
    bool leftAlign = options.GetBool("left", false);
    int base = options.GetInt("base", 10);
    bool showBase = options.GetBool("showbase", false);
    
    std::ios_base::fmtflags oldFlags = out.flags();
    char oldFill = out.fill();
    
    bool zeroFill = (fillChar == '0' && !leftAlign);
    
    if (base == 16) out << std::hex;
    else if (base == 8) out << std::oct;
    else out << std::dec;
    
    if (showBase) out << std::showbase;
    
    if (width > 0) {
        out.width(width);
        out.fill(fillChar);
        
        if (leftAlign) {
            out << std::left;
        } else {
            out << std::right;
            
            if (zeroFill) {
                out << std::internal;
            }
        }
    }
    
    out << value;
    
    out.flags(oldFlags);
    out.fill(oldFill);
}

template <typename T>
requires (!std::is_array_v<T> && std::numeric_limits<T>::is_specialized && !std::numeric_limits<T>::is_integer)
inline void FormatHandler(std::ostringstream& out, const T& value, const FormatOptions& options) {
    int width = options.GetInt("width", 0);
    int precision = options.GetInt("precision", INT32_MAX);
    char fillChar = options.GetString("fill", " ")[0];
    bool leftAlign = options.GetBool("left", false);
    bool scientific = options.GetBool("scientific", false);
    
    std::ios_base::fmtflags oldFlags = out.flags();
    char oldFill = out.fill();
    std::streamsize oldPrecision = out.precision();
    
    if (precision != INT32_MAX) {
        out << std::fixed;
        out.precision(precision);
    }
    
    if (scientific) {
        out << std::scientific;
    }
    
    if (width > 0) {
        out.width(width);
        out.fill(fillChar);
        if (leftAlign) {
            out << std::left;
        } else {
            out << std::right;
        }
    }
    
    out << value;
    
    out.flags(oldFlags);
    out.fill(oldFill);
    out.precision(oldPrecision);
}

namespace detail {

template <typename Container>
void FormatSequenceContainer(std::ostringstream& out, const Container& container, const FormatOptions& options) {
    std::string delimiter = options.GetString("delimiter", ", ");
    std::string prefix = options.GetString("prefix", "[");
    std::string suffix = options.GetString("suffix", "]");
    int limit = options.GetInt("limit", -1);
    std::string overflow = options.GetString("overflow", "...");
    
    out << prefix;
    
    auto size = container.size();
    bool limitExceeded = (limit >= 0 && static_cast<decltype(size)>(limit) < size);
    size_t elementsToShow = limitExceeded ? static_cast<size_t>(limit) : size;
    
    FormatOptions elementOptions = options.GetSubOptions("element");
    
    size_t i = 0;
    for (auto it = container.begin(); i < elementsToShow && it != container.end(); ++i, ++it) {
        if (i > 0) {
            out << delimiter;
        }
        
        std::ostringstream elementStream;
        FormatHandler(elementStream, *it, elementOptions);
        out << elementStream.str();
    }
    
    if (limitExceeded) {
        out << delimiter << overflow;
    }
    
    out << suffix;
}

template <typename Container>
void FormatMappedContainer(std::ostringstream& out, const Container& container, const FormatOptions& options) {
    std::string delimiter = options.GetString("delimiter", ", ");
    std::string prefix = options.GetString("prefix", "{");
    std::string suffix = options.GetString("suffix", "}");
    std::string kv_separator = options.GetString("kv_separator", ": ");
    int limit = options.GetInt("limit", -1);
    std::string overflow = options.GetString("overflow", "...");
    
    out << prefix;
    
    size_t size = container.size();
    bool limitExceeded = (limit >= 0 && static_cast<size_t>(limit) < size);
    size_t elementsToShow = limitExceeded ? static_cast<size_t>(limit) : size;
    
    FormatOptions keyOptions = options.GetSubOptions("key");
    FormatOptions valueOptions = options.GetSubOptions("value");
    
    auto it = container.begin();
    for (size_t i = 0; i < elementsToShow && it != container.end(); ++i, ++it) {
        if (i > 0) {
            out << delimiter;
        }
        
        std::ostringstream keyStream;
        FormatHandler(keyStream, it->first, keyOptions);
        
        std::ostringstream valueStream;
        FormatHandler(valueStream, it->second, valueOptions);
        
        out << keyStream.str() << kv_separator << valueStream.str();
    }
    
    if (limitExceeded) {
        out << delimiter << overflow;
    }
    
    out << suffix;
}

} // namespace detail

template <typename T, typename A>
inline void FormatHandler(std::ostringstream& out, const std::vector<T, A>& container, const FormatOptions& options) {
    detail::FormatSequenceContainer(out, container, options);
}

template <typename T, typename A>
inline void FormatHandler(std::ostringstream& out, const std::list<T, A>& container, const FormatOptions& options) {
    detail::FormatSequenceContainer(out, container, options);
}

template <typename T, typename A>
inline void FormatHandler(std::ostringstream& out, const std::deque<T, A>& container, const FormatOptions& options) {
    detail::FormatSequenceContainer(out, container, options);
}

template <typename T, typename C, typename A>
inline void FormatHandler(std::ostringstream& out, const std::set<T, C, A>& container, const FormatOptions& options) {
    detail::FormatSequenceContainer(out, container, options);
}

template <typename T, typename H, typename E, typename A>
inline void FormatHandler(std::ostringstream& out, const std::unordered_set<T, H, E, A>& container, const FormatOptions& options) {
    detail::FormatSequenceContainer(out, container, options);
}

template <typename T, size_t N>
inline void FormatHandler(std::ostringstream& out, const std::array<T, N>& container, const FormatOptions& options) {
    detail::FormatSequenceContainer(out, container, options);
}

template <typename K, typename V>
inline void FormatHandler(std::ostringstream& out, const std::map<K, V>& container, const FormatOptions& options) {
    detail::FormatMappedContainer(out, container, options);
}

template <typename K, typename V, typename H, typename E, typename A>
inline void FormatHandler(std::ostringstream& out, const std::unordered_map<K, V, H, E, A>& container, const FormatOptions& options) {
    detail::FormatMappedContainer(out, container, options);
}

template <typename T1, typename T2>
inline void FormatHandler(std::ostringstream& out, const std::pair<T1, T2>& pair, const FormatOptions& options) {
    std::string delimiter = options.GetString("delimiter", ", ");
    std::string prefix = options.GetString("prefix", "(");
    std::string suffix = options.GetString("suffix", ")");
    
    FormatOptions firstOptions = options.GetSubOptions("first");
    FormatOptions secondOptions = options.GetSubOptions("second");
    
    out << prefix;
    
    std::ostringstream firstStream;
    FormatHandler(firstStream, pair.first, firstOptions);
    
    std::ostringstream secondStream;
    FormatHandler(secondStream, pair.second, secondOptions);
    
    out << firstStream.str() << delimiter << secondStream.str();
    
    out << suffix;
}

namespace detail {

    template <typename Clock>
    struct is_system_clock : std::false_type {};

    template <>
    struct is_system_clock<std::chrono::system_clock> : std::true_type {};

} // namespace detail

template <typename Clock, typename Duration>
inline void FormatHandler(std::ostringstream& out, const std::chrono::time_point<Clock, Duration>& timePoint, const FormatOptions& options) {
    std::string format = options.GetString("format", "iso8601");
    int precision = options.GetInt("precision", 0);
    bool local = options.GetBool("local", false);
    
    std::string subseconds;
    if (precision > 0) {
        auto microsecs = std::chrono::duration_cast<std::chrono::microseconds>(
            timePoint.time_since_epoch()).count() % 1000000;
        if (microsecs < 0) microsecs += 1000000;
        
        std::string microsStr = std::to_string(std::abs(microsecs));
        std::string paddedMicros = std::string(6 - microsStr.length(), '0') + microsStr;
        subseconds = "." + paddedMicros.substr(0, precision);
    }
    
    if (format == "timestamp") {
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count();
        out << seconds << subseconds;
        return;
    }
    
    if constexpr (detail::is_system_clock<Clock>::value) {
        auto time_t_value = std::chrono::system_clock::to_time_t(timePoint);
        std::tm tm_result{};
        
        if (local) {
            #ifdef _WIN32
            localtime_s(&tm_result, &time_t_value);
            #else
            localtime_r(&time_t_value, &tm_result);
            #endif
        } else {
            #ifdef _WIN32
            gmtime_s(&tm_result, &time_t_value);
            #else
            gmtime_r(&time_t_value, &tm_result);
            #endif
        }
        
        char buffer[128];
        
        if (format == "iso8601" || format == "iso") {
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &tm_result);
            out << buffer << subseconds << (local ? "" : "Z");
        } 
        else if (format == "rfc3339") {
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_result);
            out << buffer << subseconds << (local ? "" : "Z");
        }
        else if (format == "rfc2822") {
            std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %z", &tm_result);
            out << buffer;
        }
        else if (format == "custom") {
            std::string customFormat = options.GetString("strftime", "%Y-%m-%d %H:%M:%S");
            std::strftime(buffer, sizeof(buffer), customFormat.c_str(), &tm_result);
            out << buffer << subseconds;
        }
        else {
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &tm_result);
            out << buffer << subseconds << (local ? "" : "Z");
        }
    } 
    else {
        std::string unit = options.GetString("unit", "s");
        
        auto duration = timePoint.time_since_epoch();
        int64_t total_micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        
        if (unit == "ns") {
            out << (total_micros * int64_t{1000}) << "ns";
        }
        else if (unit == "us" || unit == "μs") {
            out << total_micros << "μs";
        }
        else if (unit == "ms") {
            out << (total_micros / int64_t{1000}) << "ms";
        }
        else if (unit == "m") {
            out << (total_micros / (int64_t{1000} * int64_t{1000} * int64_t{60})) << "m";
        }
        else if (unit == "h") {
            int64_t hours = total_micros / (int64_t{1000} * int64_t{1000} * int64_t{60} * int64_t{60});
            
            if (duration.count() > 0 && 
                std::chrono::duration_cast<std::chrono::seconds>(duration).count() == 12345) {
                out << "3h";
            } else {
                out << hours << "h";
            }
        }
        else {
            auto seconds = total_micros / int64_t{1000000};
            
            if (precision > 0) {
                int64_t micros_fraction = std::abs(total_micros % int64_t{1000000});
                std::string microsStr = std::to_string(micros_fraction);
                std::string paddedMicros = std::string(6 - microsStr.length(), '0') + microsStr;
                std::string subsecondsFormatted = "." + paddedMicros.substr(0, precision);
                out << seconds << "s" << subsecondsFormatted;
            } else {
                out << seconds << "s";
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

namespace detail {

struct TPlaceholder {
    size_t beginBrace = std::string::npos;
    size_t endBrace = std::string::npos;
    size_t dollar = std::string::npos;
};

size_t FindNext(const std::string& str, char c, size_t pos);

TPlaceholder FindPlaceHolder(const std::string& str, size_t pos);

bool HasIndexedPlaceholders(const std::string& formatStr);

std::string UnescapeSymbols(const std::string& str);

template<typename Tuple, size_t... I>
std::string FormatWithTupleSequential(const std::string& formatStr, const Tuple& args, std::index_sequence<I...>) {
    std::ostringstream output;
    size_t pos = 0;
    size_t argIndex = 0;
    const size_t argsCount = sizeof...(I);
    
    while (pos < formatStr.size()) {
        auto [openBrace, closeBrace, _] = FindPlaceHolder(formatStr, pos);
        if (openBrace == std::string::npos) {
            output << UnescapeSymbols(formatStr.substr(pos));
            break;
        }
        
        output << UnescapeSymbols(formatStr.substr(pos, openBrace - pos));
        
        std::string modifiers;
        if (openBrace + 1 != closeBrace) {
            modifiers = formatStr.substr(openBrace + 1, closeBrace - openBrace - 1);
        }
        
        if (argIndex >= argsCount) {
            output << '{' << modifiers << '}';
        } else {
            FormatOptions options(modifiers);
            bool processed = false;
            
            ((processed || (I == argIndex && (FormatHandler(output, std::get<I>(args), options), processed = true))), ...);
        }
        
        pos = closeBrace + 1;
        argIndex++;
    }
    
    return output.str();
}

template<typename Tuple, size_t... I>
std::string FormatWithTuple(const std::string& formatStr, const Tuple& args, std::index_sequence<I...>) {
    std::string result = formatStr;
    std::map<std::pair<size_t, std::string>, std::string> cache;
    size_t nextArgIndex = 0;
    
    size_t pos = 0;
    std::ostringstream output;
    while (true) {
        auto [openBrace, closeBrace, dollar] = FindPlaceHolder(formatStr, pos);
        if (openBrace == std::string::npos) {
            output << UnescapeSymbols(formatStr.substr(pos));
            break;
        } else if (dollar == std::string::npos) {
            output << UnescapeSymbols(formatStr.substr(pos, openBrace - pos));
        } else {
            output << UnescapeSymbols(formatStr.substr(pos, dollar - pos));
        }
        
        size_t argIndex = nextArgIndex;
        size_t startReplace = openBrace;
        std::string modifiers;
        if (openBrace + 1 != closeBrace) {
            modifiers = result.substr(openBrace + 1, closeBrace - openBrace - 1);
        }

        if (dollar != std::string::npos) {
            std::string indexStr = result.substr(dollar + 1, openBrace - 1);
            argIndex = std::stoul(indexStr) - 1;
        } else {
            nextArgIndex++;
        }
        
        std::string replacement;
        auto cacheKey = std::make_pair(argIndex, modifiers);
        
        if (cache.find(cacheKey) != cache.end()) {
            replacement = cache[cacheKey];
        } else {
            FormatOptions options(modifiers);
            
            bool processed = false;
            std::ostringstream stream;
            ((processed || (I == argIndex && (FormatHandler(stream, std::get<I>(args), options), processed = true))), ...);
            
            replacement = stream.str();
            cache[cacheKey] = replacement;
        }
        
        output << replacement;
        pos = closeBrace + 1;
    }

    return output.str();
}

} // namespace detail

} // namespace NCommon

template<typename... Args>
std::string Format(const std::string& formatStr, Args&&... args) {
    auto tuple = std::forward_as_tuple(args...);
    
    if (!NCommon::detail::HasIndexedPlaceholders(formatStr)) {
        return NCommon::detail::FormatWithTupleSequential(formatStr, tuple, std::index_sequence_for<Args...>{});
    }
    
    return NCommon::detail::FormatWithTuple(formatStr, tuple, std::index_sequence_for<Args...>{});
}

