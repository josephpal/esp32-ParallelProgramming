/*
 * HelperFunctions.cpp
 *
 *  Created on: Dec 2, 2019
 *      Author: joseph
 */

#include "HelperFunctions.h"

HelperFunctions *HelperFunctions::instance = 0;

HelperFunctions* HelperFunctions::getInstance() {
	if (!instance)
		instance = new HelperFunctions();
	return instance;
}

int HelperFunctions::gcd(int a, int b) {
	while (a != b) {
		if (a > b)
			a -= b;
		else
			b -= a;
	}
	return a;
}

int HelperFunctions::gd(int a, int b) {
	if( a < b ) {
		int c = a;
		a = b;
		b = c;
	}

	while( a%b != 0 ) {
		b--;
	}

	return b;
}

bool HelperFunctions::isPerfectSquare(int n) {
	for (int i = 1; i * i <= n; i++) {
		if ((n % i == 0) && (n / i == i)) {
			return true;
		}
	}
	return false;
}

String HelperFunctions::decToBinary(String &input, int bit) {

	// return result
	String result = "";

	if (input != "") {
		// as an input number
		int n = input.toInt();
		bit = bit - 1;

		do {
			if ((n >> bit) & 1)
				result += "1";
			else
				result += "0";

			--bit;
		} while (bit >= 0);
	}

	return result;
}

