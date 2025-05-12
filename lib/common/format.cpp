#include <common/format.h>

namespace NCommon {

std::string Trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }
    
    auto end = s.end();
    if (start != s.end()) {
        end--;
        while (end != start && std::isspace(*end)) {
            end--;
        }
        end++;
    }
    
    return std::string(start, end);
}

std::string EscapeSymbols(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for (char c : str) {
        switch (c) {
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            default: result += c;
        }
    }
    
    return result;
}

std::vector<std::string> Split(const std::string& s, const std::string& delimiter, size_t limit) {
    std::vector<std::string> result;
    size_t pos = 0;
    size_t count = 0;
    
    while (pos < s.length()) {
        size_t found = s.find(delimiter, pos);
        
        if (found == std::string::npos || (limit > 0 && count == limit - 1)) {
            result.push_back(s.substr(pos));
            break;
        }
        
        result.push_back(s.substr(pos, found - pos));
        pos = found + delimiter.length();
        count++;
    }
    
    return result;
}

FormatOptions::FormatOptions(const std::string& modifierStr) {
    ParseFromString(modifierStr);
}

FormatValue::FormatValue(const FormatOptions& value) : type_(EFormatValueType::Options) {
    value_.options_value = new FormatOptions(value);
}

FormatValue::FormatValue(const FormatValue& other) : type_(other.type_) {
    switch (type_) {
        case EFormatValueType::Bool:
            value_.bool_value = other.value_.bool_value;
            break;
        case EFormatValueType::String:
            new(&value_.string_value) std::string(other.value_.string_value);
            break;
        case EFormatValueType::Options:
            value_.options_value = other.value_.options_value ? 
                new FormatOptions(*other.value_.options_value) : nullptr;
            break;
        case EFormatValueType::None:
            break;
    }
}

FormatValue::FormatValue(FormatValue&& other) noexcept : type_(other.type_) {
    switch (type_) {
        case EFormatValueType::Bool:
            value_.bool_value = other.value_.bool_value;
            break;
        case EFormatValueType::String:
            new(&value_.string_value) std::string(std::move(other.value_.string_value));
            break;
        case EFormatValueType::Options:
            value_.options_value = other.value_.options_value;
            other.value_.options_value = nullptr;
            break;
        case EFormatValueType::None:
            break;
    }
    other.type_ = EFormatValueType::None;
}

FormatValue& FormatValue::operator=(const FormatValue& other) {
    if (this != &other) {
        this->~FormatValue();
        new(this) FormatValue(other);
    }
    return *this;
}

FormatValue& FormatValue::operator=(FormatValue&& other) noexcept {
    if (this != &other) {
        this->~FormatValue();
        new(this) FormatValue(std::move(other));
    }
    return *this;
}

FormatValue::~FormatValue() {
    if (type_ == EFormatValueType::String) {
        value_.string_value.~basic_string();
    } else if (type_ == EFormatValueType::Options) {
        delete value_.options_value;
    }
}

const FormatOptions& FormatValue::AsOptions() const {
    static const FormatOptions emptyOptions;
    return (type_ == EFormatValueType::Options && value_.options_value) ? 
           *value_.options_value : emptyOptions;
}

const FormatOptions& FormatValue::AsOptions(const FormatOptions& defaultValue) const {
    return (type_ == EFormatValueType::Options && value_.options_value) ? 
           *value_.options_value : defaultValue;
}

bool FormatOptions::Has(const std::string& key) const {
    return options_.find(key) != options_.end();
}

bool FormatOptions::GetBool(const std::string& key, bool defaultValue) const {
    auto it = options_.find(key);
    if (it != options_.end() && it->second.IsBool()) {
        return it->second.AsBool();
    }
    return defaultValue;
}

int FormatOptions::GetInt(const std::string& key, int defaultValue) const {
    auto it = options_.find(key);
    if (it != options_.end() && it->second.IsInt()) {
        return it->second.AsInt();
    }
    return defaultValue;
}

double FormatOptions::GetDouble(const std::string& key, double defaultValue) const {
    auto it = options_.find(key);
    if (it != options_.end() && it->second.IsDouble()) {
        return it->second.AsDouble();
    }
    return defaultValue;
}

std::string FormatOptions::GetString(const std::string& key, const std::string& defaultValue) const {
    auto it = options_.find(key);
    if (it != options_.end() && it->second.IsString()) {
        return it->second.AsString();
    }
    return defaultValue;
}

const FormatOptions& FormatOptions::GetOptions(const std::string& key, const FormatOptions& defaultValue) const {
    auto it = options_.find(key);
    if (it != options_.end() && it->second.IsOptions()) {
        return it->second.AsOptions();
    }
    return defaultValue;
}

void FormatOptions::Set(const std::string& key, bool value) {
    options_[key] = FormatValue(value);
}

void FormatOptions::Set(const std::string& key, int value) {
    options_[key] = FormatValue(value);
}

void FormatOptions::Set(const std::string& key, double value) {
    options_[key] = FormatValue(value);
}

void FormatOptions::Set(const std::string& key, const std::string& value) {
    options_[key] = FormatValue(value);
}

void FormatOptions::Set(const std::string& key, const char* value) {
    options_[key] = FormatValue(std::string(value));
}

void FormatOptions::Set(const std::string& key, const FormatOptions& value) {
    options_[key] = FormatValue(value);
}

FormatOptions FormatOptions::GetSubOptions(const std::string& prefix) const {
    FormatOptions result;
    const std::string prefixDot = prefix + ".";
    
    for (const auto& [key, value] : options_) {
        if (key.rfind(prefixDot, 0) == 0) {
            std::string subKey = key.substr(prefixDot.size());
            if (value.IsBool()) result.Set(subKey, value.AsBool());
            else if (value.IsString()) result.Set(subKey, value.AsString());
            else if (value.IsOptions()) result.Set(subKey, value.AsOptions());
        }
    }
    
    if (result.options_.empty()) {
        auto it = options_.find(prefix);
        if (it != options_.end() && it->second.IsOptions()) {
            return it->second.AsOptions();
        }
    }
    
    return result;
}

FormatOptions FormatOptions::Merge(FormatOptions other) const {
    FormatOptions temp = *this;
    temp.options_.merge(other.options_);
    return temp;
}

void FormatOptions::ParseFromString(const std::string& modifierStr) {
    if (modifierStr.empty()) {
        return;
    }
    
    size_t pos = 0;
    while (pos < modifierStr.size()) {
        while (pos < modifierStr.size() && std::isspace(modifierStr[pos])) {
            pos++;
        }
        
        if (pos >= modifierStr.size()) {
            break;
        }
        
        size_t commaPos = modifierStr.find(',', pos);
        size_t equalsPos = modifierStr.find('=', pos);
        
        if (equalsPos == std::string::npos || (commaPos != std::string::npos && commaPos < equalsPos)) {
            size_t endPos = (commaPos != std::string::npos) ? commaPos : modifierStr.size();
            std::string key = Trim(modifierStr.substr(pos, endPos - pos));
            if (!key.empty()) {
                options_[key] = FormatValue(true);
            }
            pos = (commaPos != std::string::npos) ? commaPos + 1 : modifierStr.size();
            continue;
        }
        
        std::string key = Trim(modifierStr.substr(pos, equalsPos - pos));
        pos = equalsPos + 1;
        
        while (pos < modifierStr.size() && std::isspace(modifierStr[pos])) {
            pos++;
        }
        
        if (pos < modifierStr.size() && modifierStr[pos] == '{') {
            int braceDepth = 1;
            size_t startPos = pos + 1;
            size_t i = startPos;
            
            while (i < modifierStr.size() && braceDepth > 0) {
                if (modifierStr[i] == '{') {
                    braceDepth++;
                } else if (modifierStr[i] == '}') {
                    braceDepth--;
                }
                i++;
            }
            
            if (braceDepth == 0) {
                std::string nestedOptionsStr = modifierStr.substr(startPos, i - startPos - 1);
                auto value = FormatValue(FormatOptions(nestedOptionsStr));
                options_[key] = value;
                
                pos = i;
                if (pos < modifierStr.size() && modifierStr[pos] == ',') {
                    pos++;
                }
            } else {
                size_t endPos = modifierStr.find(',', pos);
                std::string value = (endPos != std::string::npos) ? 
                    modifierStr.substr(pos, endPos - pos) : modifierStr.substr(pos);
                options_[key] = FormatValue(Trim(value));
                pos = (endPos != std::string::npos) ? endPos + 1 : modifierStr.size();
            }
        } 
        else if (pos < modifierStr.size() && modifierStr[pos] == '\'') {
            pos++;
            std::string value;
            bool escaped = false;
            
            while (pos < modifierStr.size()) {
                char c = modifierStr[pos++];
                
                if (escaped) {
                    switch (c) {
                        case '\'': value += '\''; break;
                        case '\\': value += '\\'; break;
                        case 'n': value += '\n'; break;
                        case 'r': value += '\r'; break;
                        default: value += c;
                    }
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true;
                } else if (c == '\'') {
                    break;
                } else {
                    value += c;
                }
            }
            
            if (escaped) {
                value += '\\';
            }
            
            options_[key] = FormatValue(value);
            
            if (pos < modifierStr.size() && modifierStr[pos] == ',') {
                pos++;
            }
        } 
        else {
            size_t endPos = modifierStr.find(',', pos);
            std::string value;
            
            if (endPos != std::string::npos) {
                value = modifierStr.substr(pos, endPos - pos);
                pos = endPos + 1;
            } else {
                value = modifierStr.substr(pos);
                pos = modifierStr.size();
            }
            
            value = Trim(value);
            
            bool convertToBoolean = (value == "true" || value == "false");
            bool keepAsString = (key == "true" || key == "false"); 

            if (convertToBoolean && !keepAsString) {
                options_[key] = FormatValue(value == "true");
            } else {
                options_[key] = FormatValue(value);
            }
        }
    }
}

namespace detail {

////////////////////////////////////////////////////////////////////////////////

size_t FindNext(const std::string& str, char c, size_t pos) {
    bool inQuote = false;
    
    for (size_t i = pos; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            i++;
            continue;
        }
        
        if (str[i] == '\'') {
            inQuote = !inQuote;
            continue;
        }
        
        if (str[i] == c && !inQuote) {
            return i;
        }
    }
    
    return std::string::npos;
}

TPlaceholder FindPlaceHolder(const std::string& str, size_t pos) {
    size_t begin = FindNext(str, '{', pos);
    if (begin == std::string::npos) {
        return {std::string::npos, std::string::npos, std::string::npos};
    }

    size_t cur = begin;
    size_t end = FindNext(str, '}', begin);
    while (true) {
        if (end == std::string::npos) {
            return {std::string::npos, std::string::npos, std::string::npos};
        }
        cur = FindNext(str, '{', cur + 1);
        if (cur == std::string::npos || cur > end) {
            size_t dollar = std::string::npos;
            if (begin > 0) {
                int i = static_cast<int>(begin) - 1;
                
                while (i >= 0 && std::isdigit(str[i])) {
                    i--;
                }

                if (i >= 0 && str[i] == '$') {
                    dollar = static_cast<size_t>(i);
                }
            }
            
            return {begin, end, dollar};
        }
        end = FindNext(str, '}', end + 1);
    }
}

bool HasIndexedPlaceholders(const std::string& formatStr) {
    size_t pos = 0;
    while (pos < formatStr.size()) {
        TPlaceholder placeholder = FindPlaceHolder(formatStr, pos);
        
        if (placeholder.beginBrace == std::string::npos) {
            return false;
        }
        
        if (placeholder.dollar != std::string::npos) {
            return true;
        }
        
        pos = placeholder.endBrace + 1;
    }
    
    return false;
}

std::string UnescapeSymbols(const std::string& str) {
    std::string result;
    result.reserve(str.length()); // The unescaped string will be at most as long as the input
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            // Handle escape sequences
            switch (str[i + 1]) {
                case '\\': result += '\\'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case '\'': result += '\''; break;
                case '{': result += '{'; break;
                case '}': result += '}'; break;
                default: result += str[i + 1]; break;
            }
            i++; // Skip the next character since we've already processed it
        } else {
            result += str[i]; // Regular character, just add it
        }
    }
    
    return result;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace detail

} // namespace NCommon
