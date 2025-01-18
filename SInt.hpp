#ifndef SINT_H
#define SINT_H

#include <iostream>
#include <cmath>
#include <string>
#include <unordered_map>

using llong = long long;

class SInt {
    std::string s{"0"};
    bool minus{false};
    bool nan{false};

    static std::string prunePrefix(std::string &s) {
        int i = 0;
        while (s[i] == '0') ++i;
        if (i == s.size()) --i;
        return s = s.substr(i);
    }

    std::string prunePrefix() { return prunePrefix(s); }

    // for */
    static int saveSuffix(std::string &str) {
        int i = str.size() - 1;
        while (str[i] == '0') --i;
        if (i == -1) return 0; // doesn't prune "00...00"
        const int suffix = str.size() - i - 1;
        str = str.substr(0, i + 1);
        return suffix;
    }

    // for *
    static void addSuffix(std::string &str, const int suffix) { str += std::string(suffix, '0'); }

    // for /
    void truncate(std::string &str, const int suffix) const { str = s.substr(0, s.size() - suffix); }


    // for +-
    static int toEqualLength(std::string &a, std::string &b) {
        int i = std::max(a.size(), b.size());
        while (a.size() < i) a = '0' + a;
        while (b.size() < i) b = '0' + b;
        return i;
    }

    static std::string uStrPlus(std::string a, std::string b) {
        //
        int i = toEqualLength(a, b);
        --i;
        bool carry = false;
        std::string result;
        while (i >= 0) {
            char out = a[i] + b[i] - '0' - '0' + carry;
            carry = out / 10;
            out = out % 10;
            out += '0';
            result = out + result;
            --i;
        }
        if (carry) result = '1' + result;
        return result;
    }

    static std::string uStrMinusP(std::string a, std::string b) {
        // P --- a must be not less than b (result should positive)
        int i = toEqualLength(a, b);
        --i;
        bool borrowed = false;
        std::string result;
        while (i >= 0) {
            char out = a[i] - b[i];
            if (borrowed) --out; // from last digit
            if (out < 0) {
                borrowed = true;
                out += 10;
            } else borrowed = false;
            out += '0';
            result = out + result;
            --i;
        }
        return result;
    }

    static std::string uStrMultiply(const std::string &a, const std::string &b) {
        std::string result = "0";
        if (a == "0" || b == "0") return "0";
        std::unordered_map<char, std::string> calc; // 乘法中间结果
        for (int i = 0; i < b.size(); ++i) {
            std::string tmp;
            const int i2 = b.size() - 1 - i;
            if (calc.count(b[i2])) tmp = calc[b[i2]];
            else {
                // 1-digit multiplication
                char carry = 0;
                for (int j = a.size() - 1; j >= 0; --j) {
                    char out = (a[j] - '0') * (b[i2] - '0') + carry;
                    carry = out / 10;
                    out %= 10;
                    out += '0';
                    tmp = out + tmp;
                }
                if (carry) tmp = char(carry + '0') + tmp;
                calc[b[i2]] = tmp;
            }
            for (int j = 0; j < i; ++j) tmp += '0';
            result = uStrPlus(result, tmp);
        }
        return result;
    }

    static std::string uStrDivide(const std::string &a, const std::string &b) {
        std::string dividend;
        std::string divisor = b;
        std::string result;
        std::unordered_map<char, std::string> calc; // 记录乘法结果
        for (int i = 0; i < a.size(); ++i) {
            dividend += a[i];
            toEqualLength(dividend, divisor);
            if (dividend < divisor) {
                result = '0' + result;
                prunePrefix(dividend);
                prunePrefix(divisor);
                continue;
            }
            prunePrefix(dividend);
            prunePrefix(divisor);
            // 1-digit-quotient division
            char low = 0, high = 9;
            char quotient = 0;
            while (low <= high) {
                const char mid = low + ((high - low) >> 1) + '0';
                std::string mul_res;
                if (calc.count(mid)) mul_res = calc[mid];
                else {
                    mul_res = uStrMultiply(std::string(1, mid), divisor);
                    calc[mid] = mul_res;
                }
                toEqualLength(mul_res, dividend);
                if (mul_res <= dividend) {
                    quotient = mid;
                    low = mid - '0' + 1;
                } else high = mid - '0' - 1;
            }
            result += quotient;
            dividend = uStrMinusP(dividend, calc[quotient]);
        }
        return result;
    }

public:
    SInt() = default;

    SInt(const llong x, const int suf = 0) {
        if (x < 0ll) {
            s = std::to_string(-x);
            minus = true;
        } else s = std::to_string(x);
    }

    SInt(const std::string &str, const int suf = 0) {
        // non-trivial: ^[+-]*0*[1-9]\d*$
        //   "-++---++-+000012345067890" -> "12345067890"
        // else: -> "0"
        int i = 0;
        while (i < str.size() && (str[i] == '+' || str[i] == '-')) {
            if (str[i] == '-') minus = !minus;
            ++i;
        }
        while (i < str.size() && str[i] == '0') ++i;
        while (i < str.size()) {
            auto &ch = str[i];
            if ('0' <= ch && ch <= '9') {
                s += ch;
                ++i;
            } else break;
        }
        prunePrefix();
        if (s.empty()) {
            s = "0";
            minus = false;
        }
    }

    bool isNaN() const { return nan; }

    void print() const {
        if (nan) std::cout << "NaN" << std::endl;
        else std::cout << (minus ? "-" : "") << s << std::endl;
    }

    std::string to_string() const {
        if (nan) return "NaN";
        return (minus ? "-" : "") + s;
    }

    SInt abs() const {
        if (nan) return *this;
        return {s};
    }

    std::string abs_str() const { return abs().to_string(); }

    SInt &to_abs() {
        minus = false;
        return *this;
    }

    // Operators

    SInt &operator=(const SInt &num) = default;

    SInt &operator=(const std::string &str) {
        *this = SInt(str);
        return *this;
    }

    SInt &operator=(const llong num) {
        *this = SInt(num);
        return *this;
    }

    bool operator==(const SInt &num) const {
        if (nan || num.nan) return false; // as long as there's a NaN, equality is meaningless
        if (this->minus != num.minus) return false;
        if (this->s != num.s) return false;
        return true;
    }

    bool operator<(const SInt &num) const {
        if (nan || num.nan) return false; // NaN is regarded as the largest
        const bool sa = minus;
        const bool sb = num.minus;
        if (sa && !sb) return true;
        if (!sa && sb) return false;
        // same sign
        std::string a = s;
        std::string b = num.s;
        toEqualLength(a, b);
        if (!sa) return a < b;
        return b < a;
    }

    SInt operator+(const SInt &num) const {
        if (nan) return *this;
        if (num.nan) return num;
        std::string a = s;
        std::string b = num.s;
        const bool sa = minus;
        const bool sb = num.minus;
        if (sa == sb) return {(sa ? "-" : "") + uStrPlus(a, b)};
        toEqualLength(a, b); // for string comparison
        // different sign --- the greater one master the sign
        if (a > b) return {(sa ? "-" : "") + uStrMinusP(a, b)}; // -"5" and "4"
        return {(sb ? "-" : "") + uStrMinusP(b, a)}; // -"3" and "4"
    }

    SInt &operator+=(const SInt &num) { return *this = *this + num; }

    SInt operator-(const SInt &num) const {
        if (nan) return *this;
        if (num.nan) return num;
        if (*this == num) return {}; // 0
        std::string a = s;
        std::string b = num.s;
        const bool sa = minus;
        const bool sb = num.minus;
        if (sa != sb) return {(sa ? "-" : "") + uStrPlus(a, b)};
        // same sign:
        // (-1) - (-7) =  6    (a < b) and "-" -> "+"  11|0
        //   1  -   7  = -6    (a < b) and "+" -> "-"  10|1
        // (-7) - (-1) = -6    (a > b) and "-" -> "-"  01|1
        //   7  -   1  =  6    (a > b) and "+" -> "+"  00|0
        toEqualLength(a, b);
        if (a < b) return {(sa ? "" : "-") + uStrMinusP(b, a)};
        return {(sa ? "-" : "") + uStrMinusP(a, b)};
    }

    SInt &operator-=(const SInt &num) { return *this = *this - num; }

    SInt operator*(const SInt &num) const {
        if (nan) return *this;
        if (num.nan) return num;
        std::string a = s;
        std::string b = num.s;
        const bool sa = minus;
        const bool sb = num.minus;
        const int suffix = saveSuffix(a) + saveSuffix(b);
        std::string result = uStrMultiply(a, b);
        addSuffix(result, suffix);
        return {((sa ^ sb) ? "-" : "") + result};
    }

    SInt &operator*=(const SInt &num) { return *this = *this * num; }

    SInt operator/(const SInt &num) const {
        if (nan) return *this;
        if (num.nan) return num;
        std::string a = s;
        std::string b = num.s;
        prunePrefix(b);
        if (b == "0") {
            SInt result;
            result.nan = true;
            return result;
        }
        const bool sa = minus;
        const bool sb = num.minus;
        const int suffix = saveSuffix(b);
        truncate(a, suffix);
        return {((sa ^ sb) ? "-" : "") + uStrDivide(a, b)};
    }

    SInt &operator/=(const SInt &num) { return *this = *this / num; }

    SInt operator%(const SInt &num) const {
        if (nan) return *this;
        if (num.nan) return num;
        if (num.s == "0") {
            SInt result;
            result.nan = true;
            return result;
        }
        const SInt q = *this / num;
        return {*this - q * num};
    }

    SInt &operator%=(const SInt &num) { return *this = *this % num; }

    // Overloads

    SInt operator+(const std::string &str) const { return operator+(SInt(str)); }
    SInt operator+(const llong num) const { return operator+(SInt(num)); }
    SInt operator-(const std::string &str) const { return operator-(SInt(str)); }
    SInt operator-(const llong num) const { return operator-(SInt(num)); }
    SInt operator*(const std::string &str) const { return operator*(SInt(str)); }
    SInt operator*(const llong num) const { return operator*(SInt(num)); }
    SInt operator/(const std::string &str) const { return operator/(SInt(str)); }
    SInt operator/(const llong num) const { return operator/(SInt(num)); }
    SInt operator%(const std::string &str) const { return operator%(SInt(str)); }
    SInt operator%(const llong num) const { return operator%(SInt(num)); }
    SInt operator+=(const std::string &str) { return operator+=(SInt(str)); }
    SInt operator+=(const llong num) { return operator+=(SInt(num)); }
    SInt operator-=(const std::string &str) { return operator-=(SInt(str)); }
    SInt operator-=(const llong num) { return operator-=(SInt(num)); }
    SInt operator*=(const std::string &str) { return operator*=(SInt(str)); }
    SInt operator*=(const llong num) { return operator*=(SInt(num)); }
    SInt operator/=(const std::string &str) { return operator/=(SInt(str)); }
    SInt operator/=(const llong num) { return operator/=(SInt(num)); }
    SInt operator%=(const std::string &str) { return operator%=(SInt(str)); }
    SInt operator%=(const llong num) { return operator%=(SInt(num)); }
};

#endif //SINT_H