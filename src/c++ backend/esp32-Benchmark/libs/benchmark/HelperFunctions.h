/*
 * HelperFunctions.h
 *
 *  Created on: Dec 2, 2019
 *      Author: joseph
 *
 *  src:
 *
 *  [1]	https://www.geeksforgeeks.org/program-decimal-binary-conversion/
 *  [2] http://www.cplusplus.com/reference/string/stoi/
 *  [3]	https://www.geeksforgeeks.org/c-program-find-gcd-hcf-two-numbers/
 *  [4] https://stackoverflow.com/questions/29983943/converting-int-to-binary-in-c-arduino
 *  [5] https://stackoverflow.com/questions/8148235/eclipse-cdt-shows-semantic-errors-but-compilation-is-ok
 *  [6] https://www.programiz.com/cpp-programming/examples/even-odd
 *  [7] https://www.programiz.com/cpp-programming/examples/hcf-gcd
 */

#ifndef HELPERFUNCTIONS_H_
#define HELPERFUNCTIONS_H_

#include <Arduino.h>

class HelperFunctions {
  public:
	static HelperFunctions* getInstance();

	int gcd(int a, int b);

	int gd(int a, int b);

	bool isPerfectSquare(int n);

	String decToBinary(String &input, int bit);

  private:
	static HelperFunctions* instance;
};

#endif /* HELPERFUNCTIONS_H_ */
