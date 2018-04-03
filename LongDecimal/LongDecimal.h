#ifndef LONG_DECIMAL_H
#define LONG_DECIMAL_H

#include <string>
#include <cassert>

template<typename T>
class Decimal;

template<typename T>
void swap(Decimal<T>& first, Decimal<T>& second) noexcept;

template<typename T>
class Decimal
{
    static const long long DECIMAL_BASE = 1000ull * 1000 * 1000;
    static const unsigned int DECIMAL_LENGTH = 3;
    T *arr = nullptr;
    int sign = 0;
    void normalize();
    void increase_modular(const Decimal& d);
    void decrease_modular(const Decimal& d);
    int compare_modular(const Decimal& d) const;
public:
    Decimal add(const Decimal& d, bool minus) const;
    Decimal multiply(const Decimal& d) const;
    Decimal divide(const Decimal& d, bool return_divisor) const;
    int correct_compare(const Decimal& d) const;
    Decimal(T num = T{});
    Decimal(const std::string& s);
    Decimal(const Decimal& d);
    Decimal(Decimal&& d) noexcept;
    Decimal& operator=(Decimal d);
    ~Decimal();
	
    template<typename TT>
    friend void swap(Decimal<TT>& d1, Decimal<TT>& d2) noexcept;
	
    template<typename TT>
    friend std::string to_string(const Decimal<TT>& d);

    Decimal operator +(const Decimal& d) const;
    Decimal operator -(const Decimal& d) const;
    Decimal operator *(const Decimal& d) const;
    Decimal operator /(const Decimal& d) const;
    Decimal operator %(const Decimal& d) const;
    Decimal operator <(const Decimal& d) const;
    Decimal operator >(const Decimal& d) const;
    Decimal operator <=(const Decimal& d) const;
    Decimal operator >=(const Decimal& d) const;
};


template <typename T>
void Decimal<T>::normalize() {
    for (int i = DECIMAL_LENGTH - 1; i > 0; i--) {
        if (arr[i] >= 0) {
            arr[i - 1] += arr[i] / DECIMAL_BASE;
            arr[i] %= DECIMAL_BASE;
        }
        else {
            --arr[i - 1];
            arr[i] += DECIMAL_BASE;
        }
        assert(arr[i] >= 0);
    }
    //beware the overflow when increase_modular!!!
}

template <typename T>
void Decimal<T>::increase_modular(const Decimal& d) {
    for (int i = DECIMAL_LENGTH - 1; i >= 0; --i) {
        arr[i] += d.arr[i];
    }
    normalize();
}

template <typename T>
void Decimal<T>::decrease_modular(const Decimal& d) {
    for (int i = DECIMAL_LENGTH - 1; i >= 0; --i) {
        arr[i] -= d.arr[i];
    }
    normalize();
}

template <typename T>
int Decimal<T>::compare_modular(const Decimal& d) const {
    for (int i = 0; i < DECIMAL_LENGTH; ++i)
        if (arr[i] != d.arr[i]) {
            return (arr[i] > d.arr[i] ? 1 : -1);
        }
    return 0;
}

template <typename T>
Decimal<T> Decimal<T>::add(const Decimal& d, bool minus) const {
    Decimal res;
    if (this->sign == (d.sign * (minus ? -1 : 1))) {
        res = *this;
        res.increase_modular(d);
    }
    else {
        if (compare_modular(d) >= 0) {
            res = *this;
            res.decrease_modular(d);
        }
        else {
            res = d;
            res.decrease_modular(*this);
            res.sign = -res.sign;
        }
    }
    return res;
}

template <typename T>
Decimal<T> Decimal<T>::multiply(const Decimal& d) const {
    Decimal res;
    for (int i = 0; i < DECIMAL_LENGTH; ++i) {
        bool shouldNormalize = false;
        for (int j = 0; j < DECIMAL_LENGTH && i + j < DECIMAL_LENGTH; ++j) {
            res.arr[DECIMAL_LENGTH - i - j - 1] +=
                this->arr[DECIMAL_LENGTH - i - 1] * d.arr[DECIMAL_LENGTH - j - 1];
            shouldNormalize = true;
        }
        if (shouldNormalize) {
            res.normalize();
        }
    }
    res.sign = sign * d.sign;
    return res;
}

template <typename T>
Decimal<T> Decimal<T>::divide(const Decimal& d, bool return_divisor) const {
    Decimal cur(0), res = *this;

    while (res.compare_modular(d) >= 0) {
        res = res.add(d, (d.sign == sign ? 1 : 0));
        cur = cur.add(1, false);
    }
    cur.sign = sign * d.sign; // correct sign
    if (return_divisor) {
        return cur;
    }
    return res;
}

template <typename T>
int Decimal<T>::correct_compare(const Decimal& d) const {
    const int comp_res = compare_modular(d);
    if (sign != d.sign) {
        if (comp_res == 0 && (compare_modular(Decimal(0)) == 0)) {
            return 0; //in case we have -0 and +0
        }
        return sign > d.sign ? 1 : -1;
    }
    return comp_res;
}

template <typename T>
Decimal<T>::Decimal(T num) {
    arr = static_cast<T*>(calloc(DECIMAL_LENGTH, sizeof(*arr)));
    for (int i = 0; i < DECIMAL_LENGTH; i++) {
        arr[i] = 0;
    }
    arr[DECIMAL_LENGTH - 1] = (num >= 0 ? num : -num);
    normalize();
    sign = (num >= 0 ? 1 : -1);
}

template <typename T>
Decimal<T>::Decimal(const std::string& s) {
    arr = static_cast<T*>(calloc(DECIMAL_LENGTH, sizeof(*arr)));
    sign = 1;
    T base = 1;
    int ind = DECIMAL_LENGTH - 1;
    for (int i = s.size() - 1; i >= 0; i--) {
        if (s[i] == '-') {
            sign = -1;
            continue;
        }
        arr[ind] += (s[i] - '0') * base;
        base = (base * 10) % (DECIMAL_BASE - 1);
        if (base == 1) {
            ind--;
        }
    }
}

template <typename T>
Decimal<T>::Decimal(const Decimal& d) : sign(d.sign) {
    arr = static_cast<T*>(calloc(DECIMAL_LENGTH, sizeof(*arr)));
    memcpy(arr, d.arr, DECIMAL_LENGTH * sizeof(*arr));
}

template <typename T>
Decimal<T>::Decimal(Decimal&& d) noexcept {
    swap(*this, d);
}

template <typename T>
Decimal<T>::~Decimal() {
    delete[] arr;
}

template <typename T>
void swap(Decimal<T>& first, Decimal<T>& second) noexcept {
    using std::swap;

    swap(first.arr, second.arr);
    swap(first.sign, second.sign);
}

template <typename T>
Decimal<T>& Decimal<T>::operator=(Decimal d) {
    swap(*this, d);
    return *this;
}

template<typename T>
std::string to_string(const Decimal<T>& d) {
    std::string ans = (d.sign == -1 ? "-" : "");
    int i = 0;
    for (; i < Decimal<T>::DECIMAL_LENGTH && d.arr[i] == 0; ++i) {
    }
    if (i == Decimal<T>::DECIMAL_LENGTH)
        return "0";
    bool complete = false;
    for (; i < Decimal<T>::DECIMAL_LENGTH; ++i) {
        std::string to_format = std::to_string(d.arr[i]);
        if (complete) {
            ans += std::string(std::to_string(Decimal<T>::DECIMAL_BASE).size() - to_format.size() - 1, '0') + to_format;
        }
        else {
            ans += to_format;
        }
        complete = true;
    }
    return ans;
}

template <typename T>
Decimal<T> Decimal<T>::operator+(const Decimal& d) const {
    return this->add(d, false);
}

template <typename T>
Decimal<T> Decimal<T>::operator-(const Decimal& d) const {
    return this->add(d, true);
}

template <typename T>
Decimal<T> Decimal<T>::operator*(const Decimal& d) const {
    return this->multiply(d);
}

template <typename T>
Decimal<T> Decimal<T>::operator/(const Decimal& d) const {
    return this->divide(d, true);
}

template <typename T>
Decimal<T> Decimal<T>::operator%(const Decimal& d) const {
    return this->divide(d, false);
}

template <typename T>
Decimal<T> Decimal<T>::operator<(const Decimal& d) const {
    return this->correct_compare(d) == -1;
}

template <typename T>
Decimal<T> Decimal<T>::operator>(const Decimal& d) const {
    return this->correct_compare(d) == 1;
}

template <typename T>
Decimal<T> Decimal<T>::operator<=(const Decimal& d) const {
    return this->correct_compare(d) <= 0;
}

template <typename T>
Decimal<T> Decimal<T>::operator>=(const Decimal& d) const {
    return this->correct_compare(d) >= 0;
}

#endif