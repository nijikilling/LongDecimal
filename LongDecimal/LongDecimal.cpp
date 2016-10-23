// LongDecimal.cpp: определяет точку входа для консольного приложения.
//

#include <string>
#include <exception>
#include <cassert>

#include "stdafx.h"

const long long DECIMAL_BASE = 1000ull * 1000 * 1000;
const unsigned int DECIMAL_LENGTH = 3;

class Decimal
{
private:
	long long *arr;
	int sign;

	void normalize() {
		for (int i = DECIMAL_LENGTH - 1; i > 0; i--) {
			if (arr[i] >= 0) {
				arr[i - 1] += arr[i] / DECIMAL_BASE;
				arr[i] %= DECIMAL_BASE;
			}
			else {
				arr[i - 1]--; //for internal usage after decrease_modular, cannot be less than BASE
				arr[i] += DECIMAL_BASE;
			}
			assert(arr[i] >= 0); //for sure
		}
		//beware the overflow when increase_modular!!!

	}
	
	void increase_modular(const Decimal &d) {
		for (int i = DECIMAL_LENGTH - 1; i >= 0; i--)
			arr[i] += d.arr[i];
		normalize();
	}
	
	void decrease_modular(const Decimal &d) {
		for (int i = DECIMAL_LENGTH - 1; i >= 0; i--)
			arr[i] -= d.arr[i];
		normalize();
	}

	int compare_modular(const Decimal &d) const { // 1 when *this > d
		for (int i = 0; i < DECIMAL_LENGTH; i++)
			if (arr[i] != d.arr[i])
				return (arr[i] > d.arr[i] ? 1 : -1);
		return 0;
	}
public:
	Decimal add(const Decimal &d, bool minus) const {
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
		return res; //todo: test it
	}

	Decimal multiply(const Decimal &d) const {
		Decimal res;
		for (int i = 0; i < DECIMAL_LENGTH; i++) {
			bool shouldNormalize = false;
			for (int j = 0; j < DECIMAL_LENGTH && i + j < DECIMAL_LENGTH; j++) {
				res.arr[DECIMAL_LENGTH - i - j - 1] += this->arr[DECIMAL_LENGTH - i - 1] * d.arr[DECIMAL_LENGTH - j - 1];
				shouldNormalize = true;
			}
			if (shouldNormalize)
				res.normalize();
		}
		res.sign = sign * d.sign;
		return res;
	}

	Decimal divide(const Decimal &d, bool return_divisor) const {
		Decimal cur(0), res = *this;
		
		while (res.compare_modular(d) >= 0)
			res = res.add(d, (d.sign == sign ? 1 : 0)), cur = cur.add(1, 0);
		cur.sign = sign * d.sign; // correct sign
		if (return_divisor)
			return cur;
		else
			return res;
	}

	int correct_compare(const Decimal &d) const {
		int comp_res = compare_modular(d);
		if (sign != d.sign) {
			if (comp_res == 0 && (compare_modular(Decimal(0)) == 0))
				return 0; //in case we have -0 and +0
			return sign > d.sign ? 1 : -1;
		}
		return comp_res;
	}

	Decimal() {
		arr = (long long*)calloc(DECIMAL_LENGTH, sizeof(long long));
		for (int i = 0; i < DECIMAL_LENGTH; i++)
			arr[i] = 0;
		sign = 1;
	}

	Decimal(long long l) {
		arr = (long long*)calloc(DECIMAL_LENGTH, sizeof(long long));
		for (int i = 0; i < DECIMAL_LENGTH; i++)
			arr[i] = 0;
		arr[DECIMAL_LENGTH - 1] = (l >= 0 ? l : -l);
		normalize();
		sign = (l >= 0 ? 1 : -1);
	}

	Decimal(const std::string &s) {
		arr = (long long*)calloc(DECIMAL_LENGTH, sizeof(long long));
		sign = 1;
		long long base = 1;
		int ind = DECIMAL_LENGTH - 1;
		for (int i = s.size() - 1; i >= 0; i--) {
			if (s[i] == '-') {
				sign = -1;
				continue;
			}
			arr[ind] += (s[i] - '0') * base;
			base = (base * 10) % (DECIMAL_BASE - 1);
			if (base == 1)
				ind--;
		}
	}

	Decimal(const Decimal& d) : sign(d.sign)
	{
		arr = (long long*)calloc(DECIMAL_LENGTH, sizeof(long long));
		memcpy(arr, d.arr, sizeof(unsigned long long) * DECIMAL_LENGTH);
	}

	Decimal(Decimal&& d)   // string&& is an rvalue reference to a string
	{
		arr = d.arr;
		d.arr = nullptr;
		sign = d.sign;
		d.sign = 0;
	}

	~Decimal()
	{
		delete[] arr;
	}

	void swap(Decimal& first, Decimal& second) // nothrow
	{

		std::swap(first.arr, second.arr);
		std::swap(first.sign, second.sign);
	}

	Decimal& operator=(Decimal d)
	{
		swap(*this, d);
		return *this;
	}

	friend std::string to_string(const Decimal &d) {
		std::string ans = (d.sign == -1 ? "-" : "");
		int i = 0;
		for (; i < DECIMAL_LENGTH && d.arr[i] == 0; i++) { }
		if (i == DECIMAL_LENGTH)
			return "0";
		bool complete = false;
		for (; i < DECIMAL_LENGTH; i++) {

			std::string to_format = std::to_string(d.arr[i]);
			if (complete)
				ans += std::string(std::to_string(DECIMAL_BASE).size() - to_format.size() - 1, '0') + to_format;
			else
				ans += to_format;
		    complete = true;
		}
		return ans;
	}

	Decimal operator +(const Decimal &d) const { return this->add(d, false); }
	Decimal operator -(const Decimal &d) const { return this->add(d, true); }
	Decimal operator *(const Decimal &d) const { return this->multiply(d); }
	Decimal operator /(const Decimal &d) const { return this->divide(d, true); }
	Decimal operator %(const Decimal &d) const { return this->divide(d, false); }
	Decimal operator <(const Decimal &d) const { return this->correct_compare(d) == -1; } //ToDo fix
	Decimal operator >(const Decimal &d) const { return this->correct_compare(d) == 1; }
	Decimal operator <=(const Decimal &d) const { return this->correct_compare(d) <= 0; }
	Decimal operator >=(const Decimal &d) const { return this->correct_compare(d) >= 0; }
};

#include <fstream>
#include <string>

//contains simple test for all the operations

int make_everything(Decimal d1, Decimal d2) {
	std::ofstream stream("output.txt", std::ofstream::app);
	stream << to_string(d1) << " + " << to_string(d2) << " = " << to_string(d1 + d2) << std::endl;
	stream << to_string(d1) << " - " << to_string(d2) << " = " << to_string(d1 - d2) << std::endl;
	stream << to_string(d1) << " * " << to_string(d2) << " = " << to_string(d1 * d2) << std::endl;
	stream << to_string(d1) << " / " << to_string(d2) << " = " << to_string(d1 / d2) << std::endl;
	stream << to_string(d1) << " % " << to_string(d2) << " = " << to_string(d1 % d2) << std::endl;
	stream << to_string(d1) << " < " << to_string(d2) << " = " << to_string(d1 < d2) << std::endl;
	stream << to_string(d1) << " <= " << to_string(d2) << " = " << to_string(d1 <= d2) << std::endl;
	stream << to_string(d1) << " > " << to_string(d2) << " = " << to_string(d1 > d2) << std::endl;
	stream << to_string(d1) << " >= " << to_string(d2) << " = " << to_string(d1 >= d2) << std::endl;
	stream.close();
	return 0;
}

int some_test()
{
	std::ofstream stream("output.txt");
	for (int i = -1000000002; i < -1000000000; i++) {
		for (int j = 1000000000; j < 1000000002; j++) {
			make_everything(Decimal(i), Decimal(j));
		}
	}
	stream.close();
    return 0;
}

int main() {
	std::ofstream stream("output.txt");
	stream.close();
	std::ifstream input("input.txt");
	

	std::string s1, s2;
	//input format: [NUMBER] [NUMBER]. Everything except this will cause undefiined behaviour. If you fell like 
	//having troubles with number size, try increasing DECIMAL_LENGTH value 
	input >> s1 >> s2;
	make_everything(Decimal(s1), Decimal(s2));
	return 0;
}