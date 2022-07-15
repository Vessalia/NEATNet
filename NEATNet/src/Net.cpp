#include "Net.h"
#include <cstdlib>
#include <iostream>

using namespace NetComponents;

Net::Net(size_t numInputs, size_t numOutputs)
	: m_numInputs(numInputs)
	, m_numOutputs(numOutputs)
	, m_innovation(0)
{
	for (int i = 0; i < numInputs + numOutputs; ++i)
	{
		Node* node = new Node();
		if (i < numInputs)
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

Net::~Net()
{
	for (Connection* connection : m_genotype.connections)
	{
		free(connection);
	}

	for (Node* node : m_genotype.nodes)
	{
		free(node);
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

void Net::attemptMutation(double mutationRate)
{
	double mutationChance = Net::randDecimal();
	if (mutationChance < mutationRate)
	{
		unsigned int mutation = Net::randInt(4);

		switch (mutation)
		{
			case 0:
				std::cout << "add hidden node\n" << std::endl;
				addHiddenNode();
				break;
			case 1:
				std::cout << "add connection\n" << std::endl;
				addRandomConnection();
				break;
			case 2:
				std::cout << "remove hidden node\n" << std::endl;
				removeHiddenNode();
				break;
			case 3:
				std::cout << "remove connection\n" << std::endl;
				removeRandomConnection();
				break;
		}
	}
}

void Net::addHiddenNode()
{
	std::vector<Connection*> connections = m_genotype.connections;

	if (connections.size() > 0)
	{
		size_t index = (size_t)Net::randInt((int)connections.size());

		while (connections[index]->enabled == false)
		{
			index = (size_t)Net::randInt((int)connections.size());
		}

		Node* newNode = new Node();

		newNode->type = NodeType::Hidden;
		newNode->value = 0;
		m_genotype.nodes.push_back(newNode);

		Connection* connection = connections[index];
		connection->enabled = false;

		addConnection(connection->in, m_genotype.nodes.size() - 1);
		addConnection(m_genotype.nodes.size() - 1, connection->out);
	}
}

void Net::addRandomConnection()
{
	size_t in = (size_t)Net::randInt((int)m_genotype.nodes.size());
	size_t out = (size_t)Net::randInt((int)m_genotype.nodes.size());

	while (m_genotype.nodes[in]->type == NodeType::Output)
	{
		in = (size_t)Net::randInt((int)m_genotype.nodes.size());
	}

	while (in == out
	    || !checkValidConnection(in, out) 
	    || checkExistingConnection(in, out))
	{
		out = (size_t)Net::randInt((int)m_genotype.nodes.size());
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
		connection->weight = Net::randDecimal();
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
	 || inNode->type == NodeType::Output)
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

void Net::removeHiddenNode()
{
	if (m_genotype.nodes.size() > m_numInputs + m_numOutputs)
	{
		std::vector<Node*> nodes = m_genotype.nodes;
		Node* node = nodes[Net::randInt((int)nodes.size())];
		while (node->type != NodeType::Hidden)
		{
			node = nodes[Net::randInt((int)nodes.size())];
		}

		removeNode(node);
		cullConnections();
	}
}

void Net::removeNode(Node* node)
{
	std::vector<Node*> nodes = m_genotype.nodes;
	std::vector<Node*>::iterator index = std::find(nodes.begin(), nodes.end(), node);
	if (index != nodes.end())
	{
		nodes.erase(index);
		free(node);
	}
}

void Net::removeRandomConnection()
{
	if (m_genotype.connections.size() > 0)
	{
		removeConnection(m_genotype.connections[Net::randInt((int)m_genotype.connections.size())]);
		cullConnections();
	}
}

void Net::removeConnection(size_t in, size_t out)
{
	std::vector<Connection*> connections = m_genotype.connections;
	Connection* connection = checkExistingConnection(in, out);

	if (connection)
	{
		std::vector<Connection*>::iterator index = std::find(connections.begin(), connections.end(), connection);
		if (index != connections.end())
		{
			connections.erase(index);
		}
	}

	free(connection);
}

void Net::removeConnection(Connection* connection)
{
	removeConnection(connection->in, connection->out);
}

bool Net::cullConnections()
{
	bool culled = false;

	for (int i = 0; i < m_genotype.nodes.size(); ++i)
	{
		std::vector<size_t> ins;
		std::vector<size_t> outs;

		for (Connection* connection : m_genotype.connections)
		{
			if (connection->in == i) outs.push_back(connection->out);
			else if (connection->out == i) ins.push_back(connection->in);
		}

		if (ins.size() == 0)
		{
			for (size_t out : outs)
			{
				removeConnection(i, out);
				removeNode(m_genotype.nodes[i]);
				culled = true;
			}
		}
		else if (outs.size() == 0)
		{
			for (size_t in : ins)
			{
				removeConnection(in, i);
				removeNode(m_genotype.nodes[i]);
				culled = true;
			}
		}
	}

	return culled;
}
