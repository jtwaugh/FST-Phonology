// ----------------------------------------------
//			FST.H
// ----------------------------------------------
// Finite state transducer class
// ---------------------------------------------- 

#ifndef FST_H
#define FST_H

// ---------------------------------------------- 

#include "phonrule.h"
#include "transition.h"

#include <vector>

// ---------------------------------------------- 

// Gonna hardcode the state types and input/output as chars. Sorry.

// The FST is an automaton that holds a transition relation and two sets of symbols.
// It validates a string of symbols in the first set against a string of symbols in the second.

class FST
{
private:
	std::unordered_set<int>					states_;														// Set of states
	std::unordered_set<int>					initial_states_;												// Set of starting states; honestly, probably always going to be zero
	std::unordered_set<int>					final_states_;													// Set of valid terminal states
	
	std::unordered_set<char>				input_alphabet_;												// Set of symbols allowed on the input tape
	std::unordered_set<char>				output_alphabet_;												// Set of symbols allowed on the output tape
		
	transition_function_t					transition_;													// Set of 4-tuples (imagine as graph edges) that we validate against
		
	int										OutputState(char input, char output, int current_state);		// Gets output state as a function of input symbol, output symbol, and current state
	void									AddDefaultEdge(int j, char prefix_0);							// Adds edges feeding back to either zero or the beginning of the prefix chain
	void									FillChain(std::string source, int &j, char prefix_0);			// Fills out either the prefix or the suffix chain

public:
	FST(std::vector<int> states,																			// "Synthetic" constructor
		std::vector<int> initial_states, 
		std::vector<int> final_states, 
		std::string input_alphabet, 
		std::string output_alphabet);
	
	FST(std::string input_alphabet,																			// Builds a transition function from a phonological rule
		std::string output_alphabet, 
		PhonRule& rule);

	bool									Validate(std::string input_tape, std::string output_tape);		// Iterates through the transition function, aborts with error is the output tape is invalid
};

// ---------------------------------------------- 

FST::FST(std::vector<int> states, std::vector<int> initial_states, std::vector<int> final_states, std::string input_alphabet, std::string output_alphabet)
{
	states_ = std::unordered_set<int>();

	// Add all the states
	for (int i = 0; i < states.size(); i++)
	{
		states_.insert(states[i]);
	}

	// Verify that the initial states are all states
	for (int i = 0; i < initial_states.size(); i++)
	{
		if (states_.find(initial_states[i]) == states_.end())
		{
			std::cout << "Error: initial_states not subset of states" << std::endl;
		}

		initial_states_.insert(initial_states[i]);
	}

	// Verify that the final states are all states
	for (int i = 0; i < final_states.size(); i++)
	{
		if (states_.find(final_states[i]) == states_.end())
		{
			std::cout << "Error: final_states not subset of states" << std::endl;
		}

		final_states_.insert(final_states[i]);
	}

	// Create the alphabets

	for (int i = 0; i < input_alphabet.length(); i++)
	{
		input_alphabet_.insert(input_alphabet[i]);
	}

	for (int i = 0; i < output_alphabet.length(); i++)
	{
		output_alphabet_.insert(output_alphabet[i]);
	}

}

void FST::AddDefaultEdge(int j, char prefix_0)
{
	// Add the "prefix[0]/prefix[0] goes to start of chain, else ?/? goes to 0" flag

	for (auto i = input_alphabet_.begin(); i != input_alphabet_.end(); i++)
	{
		// If there isn't already an x/x edge going somewhere else...
		bool edgefound = false;

		for (auto k = states_.begin(); k != states_.end(); k++)
		{
			edge_t candidate{ *i, j, *k, *i };
			if (transition_.find(candidate) != transition_.end())
			{
				edgefound = true;
				break;
			}
		}

		// Then add one either starting the chain or resetting it
		if (!edgefound)
		{
			if (*i == prefix_0)
			{
				edge_t default_edge{ *i, j, 1, *i };
				transition_.insert(default_edge);
			}
			else
			{
				edge_t default_edge{ *i, j, 0, *i };
				transition_.insert(default_edge);
			}
		}
	}
}

void FST::FillChain(std::string source, int &j, char prefix_0)
{
	for (int i = 0; i < source.length(); i++)
	{
		// Add the state
		states_.insert(j);

		// Ensure that we can end here
		final_states_.insert(j);

		// Create a cadidate edge and add it
		edge_t edge{ source[i], j - 1, j, source[i] };

		transition_.insert(edge);

		// Print for debugging
		std::cout << "Nontrivial edge " << j - 1 << " -> " << j << ": " << source[i] << "/" << source[i] << std::endl;

		// Add default edges
		AddDefaultEdge(j - 1, prefix_0);

		j++;
	}
}

FST::FST(std::string input_alphabet, std::string output_alphabet, PhonRule& rule) :
states_(std::unordered_set<int>()), initial_states_(std::unordered_set<int>()), final_states_(std::unordered_set<int>()), transition_(transition_function_t())
{
	// TODO: sanity check
	// TODO: allow symbols to denote sets
	// TODO: allow insertion of multiple symbols

	// Create the alphabets
	for (int i = 0; i < input_alphabet.length(); i++)
	{
		input_alphabet_.insert(input_alphabet[i]);
	}

	for (int i = 0; i < output_alphabet.length(); i++)
	{
		output_alphabet_.insert(output_alphabet[i]);
	}

	// Set up the first state
	initial_states_.insert(0);
	states_.insert(0);
	final_states_.insert(0);

	// Create the circuit
	int j = 1;

	// If there's a prefix
	if (rule.prefix().length() > 0)
	{
		// Add the states and wire them up
		FillChain(rule.prefix(), j, rule.prefix()[0]);

		// Add default edges to the last node
		AddDefaultEdge(j-1, rule.prefix()[0]);
	}
	else
	{
		// Add default edges to the first/last node
		AddDefaultEdge(0, rule.prefix()[0]);
	}

	// Add the alternation state
	states_.insert(j);

	if (rule.suffix().length() > 0)
	// If there's a suffix
	{
		// Set the alternation edge and add it
		edge_t change{ rule.input(), j - 1, j, rule.output() };
		transition_.insert(change);

		// Debug print and increment the counter
		std::cout << "Nontrivial edge " << j - 1 << " -> " << j << ": " << rule.input() << "/" << rule.output() << std::endl;
		j++;

		// Add the suffix chain
		FillChain(rule.suffix(), j, rule.prefix()[0]);

		// Add the last state
		states_.insert(j - 1);

		// Add the edge looping back to the terminal state
		edge_t end{ rule.suffix().back(), j - 1, 0, rule.suffix().back() };
		transition_.insert(end);

		// Debug print
		std::cout << "Nontrivial edge " << j - 1 << " -> " << 0 << ": " << rule.suffix().back() << "/" << rule.suffix().back() << std::endl;

		// Add default edges to the last node
		AddDefaultEdge(j, rule.prefix()[0]);
	}
	else
	// Otherwise
	{
		// Set the alternation edge to loop back to the initial node and add it
		edge_t change{ rule.input(), j - 1, 0, rule.output() };
		transition_.insert(change);

		// Debug print
		std::cout << "Nontrivial edge " << j - 1 << " -> " << j << ": " << rule.input() << "/" << rule.output() << std::endl;

	}

	// Remember in evaluation that taking any < x, n, n, x > after a final state n is valid if edges don't specify otherwise
}

int FST::OutputState(char input, char output, int current_state)
{
	// Determine the output state as a function of the input and output characters and the current state

	for (auto j = states_.begin(); j != states_.end(); j++)
	{
		edge_t edge{ input, current_state, *j, output };

		if (transition_.find(edge) != transition_.end())
		{
			return *j;
		}
	}

	// Otherwise otherwise, we've reallly screwed the pooch
	// We probably want only positively-numbered states anyway
	return -9001;
}

bool FST::Validate(std::string input_tape, std::string output_tape)
{
	// TODO: allow for tapes to be different lengths

	// TODO: "for each possible initial state"? I think they must all start at state 0.
	int current_state = 0;

	// Walk through the graph and abort if we fail
	for (int i = 0; i < input_tape.length(); i++)
	{
		int new_state = OutputState(input_tape[i], output_tape[i], current_state);
		
		// Debug print
		if (new_state == -9001)
		{
			std::cout << "Undefined output character at index " << i << ":" << input_tape[i] << "/" << output_tape[i] <<  "." << std::endl;
			return false;
		}
		else
		{
			std::cout << input_tape[i] << "/" << output_tape[i] << " takes state " << current_state << " -> " << new_state << "." << std::endl;
		}

		current_state = new_state;
	}

	std::cout << "All output characters were defined." << std::endl;

	return final_states_.find(current_state) != final_states_.end();
}

// ---------------------------------------------- 

#endif