// ----------------------------------------------
//			TRANSITION.H
// ----------------------------------------------
// Typedef nonsense so I can hash edges
// ---------------------------------------------- 

#ifndef TRANSITION_H
#define TRANSITION_H

// ---------------------------------------------- 

#include <tuple>
#include <unordered_set>

// ---------------------------------------------- 

typedef std::tuple<char, int, int, char> edge_t;

struct key_hash : public std::unary_function<edge_t, std::size_t>
{
	std::size_t operator()(const edge_t& k) const
	{
		return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k) ^ std::get<3>(k);
	}
};

struct key_equal : public std::binary_function<edge_t, edge_t, bool>
{
	bool operator()(const edge_t& v0, const edge_t& v1) const
	{
		return (
			std::get<0>(v0) == std::get<0>(v1) &&
			std::get<1>(v0) == std::get<1>(v1) &&
			std::get<2>(v0) == std::get<2>(v1) &&
			std::get<3>(v0) == std::get<3>(v1)
			);
	}
};

typedef std::unordered_set<edge_t, key_hash, key_equal> transition_function_t;

// ---------------------------------------------- 

#endif