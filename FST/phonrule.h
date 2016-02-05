// ----------------------------------------------
//			PHONRULE.H
// ----------------------------------------------
// Class to represent phonological rules
// ---------------------------------------------- 

#ifndef PHONRULE_H
#define PHONRULE_H

// ---------------------------------------------- 

#include <iostream>
#include <string>
#include <algorithm>

// ---------------------------------------------- 

class PhonRule
{
	// Replaces a symbol with a string of symbols if the prefix and suffix are satisfied
private:
	char									input_;
	char									output_;
	std::string								prefix_;
	std::string								suffix_;

public:
	PhonRule(std::string plaintext);

	char									input();
	char									output();
	std::string								prefix();
	std::string								suffix();

	bool									MatchesEnvironment(std::string prefix, std::string suffix);
};

// ---------------------------------------------- 

PhonRule::PhonRule(std::string plaintext)
{
	// Screw C++ and all of this string manipulation nonsense
	// Let the record show that hours were spent trying to make this one tangential thing succinct

	plaintext.erase(std::remove_if(plaintext.begin(), plaintext.end(), isspace), plaintext.end());

	// std::cout << "Removed whitespace. Result: \"" << plaintext << "\"." << std::endl;

	// Split at ">"
	std::string input = "";
	size_t i = plaintext.find_first_of('>');
	input.append(plaintext, 0, i);

	if (input.length() != 1)
	{
		std::cout << "Error: PhonRule input string " << input << " is not a single character." << std::endl;
	}

	input_ = input[0];

	// Split at "/"
	std::string output = "";
	size_t j = plaintext.find_first_of('/');
	output.append(plaintext, i + 1, j - i - 1);

	if (output.length() != 1)
	{
		std::cout << "Error: PhonRule output string " << output << " is not a single character." << std::endl;
	}

	output_ = output[0];

	// Split at "_"
	prefix_ = "";
	suffix_ = "";
	size_t k = plaintext.find_first_of('_');
	prefix_.append(plaintext, j + 1, k - j - 1);
	suffix_.append(plaintext, k + 1, plaintext.length() - k - 1);
}

char PhonRule::input()
{
	return input_;
}

char PhonRule::output()
{
	return output_;
}

std::string PhonRule::prefix()
{
	return prefix_;
}

std::string PhonRule::suffix()
{
	return suffix_;
}

bool PhonRule::MatchesEnvironment(std::string prefix, std::string suffix)
{
	return (prefix == prefix_ && suffix == suffix_);
}

// ---------------------------------------------- 

#endif