#pragma once

#include <vector>

namespace NetComponents
{
	enum NodeType
	{
		Sensor,
		Hidden,
		Output
	};

	struct Node
	{
		double value;
		NodeType type;
	};

	struct Connection
	{
		size_t in;
		size_t out;

		size_t innovation;

		double weight;
		bool enabled;
	};

	struct Genotype
	{
		std::vector<Node*> nodes;
		std::vector<Connection*> connections;
	};
}
