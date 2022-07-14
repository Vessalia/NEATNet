#include "Net.h"
#include <cstdlib>

using namespace NetComponents;

Net::Net(size_t numInput, size_t numOutput)
	: m_numInputs(numInput)
	, m_innovation(0)
{
	for (int i = 0; i < numInput + numOutput; ++i)
	{
		Node* node = new Node();
		if (i < numInput)
		{
			node->type = NodeType::Sensor;
		}
		else
		{
			node->type = NodeType::Output;
		}
		node->value = 0;
		m_genotype.nodes.push_back(node);
	}
}

void Net::receiveInput(const std::vector<double>& inputVals)
{
	for (size_t i = 0; i < inputVals.size() && i < m_numInputs; i++)
	{
		Node* node = m_genotype.nodes.at(i);
		node->value = inputVals[i];
	}
}

void Net::addHiddenNode()
{
	std::vector<Connection*> connections = m_genotype.connections;

	size_t index = rand() % connections.size();
	if (connections.size() > 0)
	{
		while (connections[index]->enabled == false)
		{
			index = rand() % connections.size();
		}

		Node* newNode = new Node();
		if (newNode != nullptr)
		{
			newNode->type = NodeType::Hidden;
			newNode->value = 0;
			m_genotype.nodes.push_back(newNode);

			Connection* connection = connections[index];
			connection->enabled = false;

			addConnection(connection->in, m_genotype.nodes.size() - 1);
			addConnection(m_genotype.nodes.size() - 1, connection->out);
		}
	}
}

void Net::addRandomConnection()
{
	size_t in = rand() % m_genotype.nodes.size();
	size_t out = rand() % m_genotype.nodes.size();
	while (in == out
	    || !checkValidConnection(in, out) 
	    || checkExistingConnection(in, out))
	{
		out = rand() % m_genotype.nodes.size();
	}

	addConnection(in, out);
}

void Net::addConnection(size_t in, size_t out)
{
	Connection* existingConnection = checkExistingConnection(in, out);

	if (existingConnection)
	{
		existingConnection->enabled = true;
	}
	else
	{
		Connection* connection = new Connection();

		connection->in = in;
		connection->out = out;
		connection->enabled = true;
		connection->weight = rand() / double(RAND_MAX);
		connection->innovation = m_innovation++;

		m_genotype.connections.push_back(connection);
	}
}

bool Net::checkValidConnection(size_t in, size_t out) const
{
	Node* inNode = m_genotype.nodes[in];
	Node* outNode = m_genotype.nodes[out];
	
	if (in == out 
	 || outNode->type == NodeType::Sensor
	 || (outNode->type == NodeType::Output && inNode->type == NodeType::Output))
	{
		return false;
	}

	return true;
}

Connection* Net::checkExistingConnection(size_t in, size_t out)
{
	std::vector<Connection*> connections = m_genotype.connections;

	if (in < m_genotype.nodes.size() && out < m_genotype.nodes.size())
	{
		for (size_t i = 0; i < m_genotype.connections.size(); i++)
		{
			if (connections[i]->in == in && connections[i]->out == out)
			{
				return connections[i];
			}
		}
	}
	
	return nullptr;
}
