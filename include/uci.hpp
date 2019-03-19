#pragma once

#include <iostream>
#include <variant>
#include <string>

namespace goldfish::UCI {

namespace {

using OptionValue = std::variant<int, std::string>;


struct Option
{
protected:
    std::string name_;
    std::string type_;
    OptionValue current_value_;
    OptionValue default_value_;

    Option(std::string n, std::string t, OptionValue d)
        : name_(n), type_(t), current_value_(d), default_value_(d) {}

public:
    const std::string& name() const { return name_; }
    const std::string& type() const { return type_; }
    void set(const OptionValue& value) { current_value_ = value; }
};

inline std::ostream& operator<<(std::ostream& stm, const Option& s)
{
    return stm << "option name " << s.name() << " type " << s.type();
}

struct StringOption : public Option
{
    StringOption(std::string n, std::string d)
        : Option(n, "string", d) {}
    const std::string& default_value() const { return std::get<std::string>(default_value_); }
    const std::string& current_value() const { return std::get<std::string>(current_value_); }
};

inline std::ostream& operator<<(std::ostream& stm, const StringOption& s)
{
    return stm << static_cast<Option>(s) << " default " << s.default_value();
}

struct IntegerOption : public Option
{
private:
    int min_value_;
    int max_value_;

public:
    IntegerOption(std::string n, int d, int min, int max)
        : Option(n, "spin", d), min_value_(min), max_value_(max) {}
    int default_value() const { return std::get<int>(default_value_); }
    int current_value() const { return std::get<int>(current_value_); }
    int minimum()       const { return min_value_; }
    int maximum()       const { return max_value_; }
};

inline std::ostream& operator<<(std::ostream& stm, const IntegerOption& s)
{
    return stm << static_cast<Option>(s)
               << " default " << s.default_value()
               << " min " << s.minimum()
               << " max " << s.maximum();
}

}

class OptionsManager
{
private:
    StringOption syzygy_path_ {"SyzygyPath", "syzygy"};
    IntegerOption tthash_ {"Hash", 256, 0, 10 * 1024};

public:
    StringOption&  SyzygyPath() { return syzygy_path_; }
    IntegerOption& Hash() { return tthash_; }
};
inline std::ostream& operator<<(std::ostream& stm, OptionsManager& opt)
{
    stm << opt.SyzygyPath() << '\n';
    stm << opt.Hash();
    return stm;
}

OptionsManager options;
}
