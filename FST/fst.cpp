#include "fst.h"

// ---------------------------------------------- 

int main()
{
	std::cout << "Enter your phonological rule (valid ones, please):" << std::endl;

	std::string phonrule;

	std::getline(std::cin, phonrule);

	PhonRule myPhonRule(phonrule);

	std::cout << "Your rule is " << myPhonRule.input() << " > " << myPhonRule.output() << " / " << myPhonRule.prefix() << "_" << myPhonRule.suffix() << std::endl;

	std::cout << "Building your FST..." << std::endl;

	FST myFST("abcdefghijklmnopqrstuvwxy", "abcdefghijklmnopqrstuvwxy", myPhonRule);

	std::string underlying;
	std::string surface;

	std::cout << "Enter input string followed by output string to validate the alternation." << std::endl;

	while (true)
	{
		std::getline(std::cin, underlying);

		std::getline(std::cin, surface);

		if (myFST.Validate(underlying, surface))
		{
			std::cout << "Your alternation is valid." << std::endl;
		}
		else
		{
			std::cout << "Your alternation is invalid. Hopefully some error messages were thrown along the way." << std::endl;
		}
	}
}

// ---------------------------------------------- 