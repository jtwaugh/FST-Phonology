// ----------------------------------------------
//			FST.H
// ----------------------------------------------
// Finite state transducer class
// ---------------------------------------------- 

#ifndef FST_H
#define FST_H

// ---------------------------------------------- 

#include <iostream>
#include <string>
#include <unordered_set>
#include <tuple>
#include <algorithm>
#include <vector>

// ---------------------------------------------- 

// Gonna hardcode the state types and input/output as chars. Sorry.

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

// Now a bunch of nonsense to build the transition function data structure:

typedef std::tuple<char, int, int, char> transition_t;

struct key_hash : public std::unary_function<transition_t, std::size_t>
{
	std::size_t operator()(const transition_t& k) const
	{
		return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k) ^ std::get<3>(k);
	}
};

struct key_equal : public std::binary_function<transition_t, transition_t, bool>
{
	bool operator()(const transition_t& v0, const transition_t& v1) const
	{
		return (
			std::get<0>(v0) == std::get<0>(v1) &&
			std::get<1>(v0) == std::get<1>(v1) &&
			std::get<2>(v0) == std::get<2>(v1) &&
			std::get<3>(v0) == std::get<3>(v1)
			);
	}
};

typedef std::unordered_set<transition_t, key_hash, key_equal> transition_function_t;

// ----------------------------------------------

class FST
{
private:
	std::unordered_set<int>									states_;
	std::unordered_set<int>									initial_states_;
	std::unordered_set<int>									final_states_;
	
	std::unordered_set<char>								input_alphabet_;
	std::unordered_set<char>								output_alphabet_;

	transition_function_t									transition_;

	int														OutputState(char input, char output, int current_state);

public:
	FST(std::vector<int> states, std::vector<int> initial_states, std::vector<int> final_states, std::string input_alphabet, std::string output_alphabet);
	FST(std::string input_alphabet, std::string output_alphabet, PhonRule& rule);

	bool													Validate(std::string input_tape, std::string output_tape);
};

// ---------------------------------------------- 

FST::FST(std::vector<int> states, std::vector<int> initial_states, std::vector<int> final_states, std::string input_alphabet, std::string output_alphabet)
{
	states_ = std::unordered_set<int>();

	for (int i = 0; i < states.size(); i++)
	{
		states_.insert(states[i]);
	}

	for (int i = 0; i < initial_states.size(); i++)
	{
		// Sanity check for subset inclusion
		if (states_.find(initial_states[i]) == states_.end())
		{
			std::cout << "Error: initial_states not subset of states" << std::endl;
		}

		initial_states_.insert(initial_states[i]);
	}

	for (int i = 0; i < final_states.size(); i++)
	{
		// Sanity check for subset inclusion
		if (states_.find(final_states[i]) == states_.end())
		{
			std::cout << "Error: final_states not subset of states" << std::endl;
		}

		final_states_.insert(final_states[i]);
	}

	for (int i = 0; i < input_alphabet.length(); i++)
	{
		input_alphabet_.insert(input_alphabet[i]);
	}

	for (int i = 0; i < output_alphabet.length(); i++)
	{
		output_alphabet_.insert(output_alphabet[i]);
	}

}

FST::FST(std::string input_alphabet, std::string output_alphabet, PhonRule& rule) :
states_(std::unordered_set<int>()), initial_states_(std::unordered_set<int>()), final_states_(std::unordered_set<int>()), transition_(transition_function_t())
{
	// TODO: sanity check
	// TODO: allow symbols to denote sets
	// TODO: allow insertion of multiple symbols

	for (int i = 0; i < input_alphabet.length(); i++)
	{
		input_alphabet_.insert(input_alphabet[i]);
	}

	for (int i = 0; i < output_alphabet.length(); i++)
	{
		output_alphabet_.insert(output_alphabet[i]);
	}

	// Set up first state

	initial_states_.insert(0);
	states_.insert(0);
	final_states_.insert(0);

	// Create the circuit

	int j = 1;

	for (int i = 0; i < rule.prefix().length(); i++)
	{
		states_.insert(j);
		final_states_.insert(j);

		std::tuple<char, int, int, char> edge { rule.prefix()[i], j-1, j, rule.prefix()[i] };

		transition_.insert(edge);

		j++;
	}

	std::tuple<char, int, int, char> change{ rule.input(), j-1, j, rule.output() };
	transition_.insert(change);

	for (int i = 0; i < rule.suffix().length() - 1; i++)
	{
		states_.insert(j);

		std::tuple<char, int, int, char> edge{ rule.suffix()[i], j, j+1, rule.suffix()[i] };

		transition_.insert(edge);

		j++;
	}

	states_.insert(j);

	std::tuple<char, int, int, char> end{ rule.suffix().back(), j, 0, rule.suffix().back() };
	transition_.insert(end);

	// Remember in evaluation that taking any < x, n, n, x > after a final state n is valid if edges don't specify otherwise
}

int FST::OutputState(char input, char output, int current_state)
{
	// Determine the output state as a function of the input and output characters and the current state

	for (auto j = states_.begin(); j != states_.end(); j++)
	{
		std::tuple<char, int, int, char> edge{ input, current_state, *j, output };

		if (transition_.find(edge) != transition_.end())
		{
			return *j;
		}
	}

	// Otherwise, we've reallly screwed the pooch
	// We probably want only positively-numbered states anyway

	return -9001;
}

bool FST::Validate(std::string input_tape, std::string output_tape)
{
	// TODO: allow for tapes to be different lengths

	// TODO: for each possible initial state? I think they must all start at state 0.
	int current_state = 0;

	for (int i = 0; i < input_tape.length(); i++)
	{
		int new_state = OutputState(input_tape[i], output_tape[i], current_state);
		
		if (new_state == -9001)
		{
			// Invalid output character
			std::cout << "Undefined output character at index " << i << " ." << std::endl;
			return false;
		}
		else
		{
			std::cout << input_tape[i] << "/" << output_tape[i] << " takes state " << current_state << " to state " << new_state << "." << std::endl;
		}

		current_state = new_state;
	}

	std::cout << "All output characters were defined." << std::endl;

	return final_states_.find(current_state) != final_states_.end();
}

// ---------------------------------------------- 

#endif