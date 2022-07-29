#include "Net.h"
#include <cstdlib>
#include <iostream>
#include <assert.h>

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
	for (auto connection : m_genotype.connections)
	{
		free(connection);
	}

	for (auto node : m_genotype.nodes)
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
		unsigned int mutation = Net::randInt(5);

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
			case 4:
				std::cout << "randomize a weight\n" << std::endl;
				randomizeRandomWeight();
				break;
		}
	}
}

void Net::addHiddenNode()
{
	auto& connections = m_genotype.connections;

	if (connections.size() > 0 && checkLiveConnections())
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
	if (isFullyConnected())
	{
		addHiddenNode();
	}
	else
	{
		size_t in = (size_t)Net::randInt((int)m_genotype.nodes.size());
		size_t out = (size_t)Net::randInt((int)m_genotype.nodes.size());

		while (m_genotype.nodes[in]->type == NodeType::Output || isNodeFullyConnected(in))
		{
			in = (size_t)Net::randInt((int)m_genotype.nodes.size());
		}

		while (!checkValidConnection(in, out))
		{
			out = (size_t)Net::randInt((int)m_genotype.nodes.size());
		}

		addConnection(in, out);
	}
}

void Net::addConnection(size_t in, size_t out)
{
	Connection* existingConnection = getExistingConnection(in, out);

	if (existingConnection)
	{
		existingConnection->enabled = true;
	}
	else if (in != out)
	{
		Connection* connection = new Connection();

		connection->in = in;
		connection->out = out;
		connection->enabled = true;
		connection->weight = 2 * Net::randDecimal() - 1;

		if (!connectionIsCyclic(connection, nullptr))
		{
			connection->innovation = m_innovation++;
			m_genotype.connections.push_back(connection);
		}
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

Connection* Net::getExistingConnection(size_t in, size_t out)
{
	auto& connections = m_genotype.connections;

	if (in < m_genotype.nodes.size() && out < m_genotype.nodes.size())
	{
		for (size_t i = 0; i < m_genotype.connections.size(); ++i)
		{
			if ((connections[i]->in == in && connections[i]->out == out)
			 || (connections[i]->in == out && connections[i]->out == in))
			{
				return connections[i];
			}
		}
	}
	
	return nullptr;
}

bool Net::checkLiveConnections()
{
	for (Connection* connection : m_genotype.connections)
	{
		if (connection->enabled) return true;
	}

	return false;
}

void Net::removeHiddenNode()
{
	if (m_genotype.nodes.size() > m_numInputs + m_numOutputs)
	{
		auto& nodes = m_genotype.nodes;
		size_t index = Net::randInt((int)nodes.size());
		Node* node = nodes[index];
		while (node->type != NodeType::Hidden)
		{
			index = Net::randInt((int)nodes.size());
			node = nodes[index];
		}

		std::vector<Connection*> toRemove;
		for (auto connection : m_genotype.connections)
		{
			if (connection->in == index || connection->out == index)
			{
				toRemove.push_back(connection);
			}
		}
		for (auto remove : toRemove)
		{
			removeConnection(remove);
		}

		correctConnections(index);
		removeNode(node);
	}
}

void Net::removeNode(Node* node)
{
	auto& nodes = m_genotype.nodes;
	auto index = std::find(nodes.begin(), nodes.end(), node);

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
		m_genotype.connections[Net::randInt((int)m_genotype.connections.size())]->enabled = false;
	}
}

void Net::removeConnection(size_t in, size_t out)
{
	auto& connections = m_genotype.connections;
	Connection* connection = getExistingConnection(in, out);

	auto index = std::find(connections.begin(), connections.end(), connection);
	if (index != connections.end())
	{
		connections.erase(index);
	}

	free(connection);
}

void Net::removeConnection(Connection* connection)
{
	removeConnection(connection->in, connection->out);
}

void Net::randomizeRandomWeight()
{
	if (m_genotype.connections.size() > 0)
	{
		m_genotype.connections[randInt((int)m_genotype.connections.size())]->weight = 2 * Net::randDecimal() - 1;
	}
}

bool Net::cullConnections()
{
	bool culled = false;

	for (size_t i = m_numInputs + m_numInputs; i < m_genotype.nodes.size(); ++i)
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

void Net::correctConnections(size_t index)
{
	for (auto connection : m_genotype.connections)
	{
		if (connection->in > index)
		{
			connection->in--;
		}
		if (connection->out > index)
		{
			connection->out--;
		}
	}
}

bool Net::isFullyConnected()
{
	for (size_t i = 0; i < m_genotype.nodes.size(); ++i)
	{
		if (!isNodeFullyConnected(i)) return false;
	}

	return true;
}

bool Net::isNodeFullyConnected(size_t index)
{
	Node* node = m_genotype.nodes[index];

	size_t count = 0;
	for (Connection* connection : m_genotype.connections)
	{
		if (connection->in == index || connection->out == index) count++;
	}

	switch (node->type)
	{
		case NodeType::Sensor:
			return count == m_genotype.nodes.size() - m_numInputs;

		case NodeType::Output:
			return count == m_genotype.nodes.size() - m_numOutputs;

		case NodeType::Hidden:
			return count == m_genotype.nodes.size() - 1;
	}
}

bool Net::connectionIsCyclic(const Connection* masterConnection, const Connection* connection) const
{
	size_t start = masterConnection->in;

	if (m_genotype.nodes[masterConnection->out]->type == NodeType::Output)
	{
		return false;
	}
	else if (connection)
	{
		if (masterConnection->in == connection->in
			&& masterConnection->out == connection->out)
		{
			return true;
		}
	}
	else
	{
		std::vector<Connection*> branches;

		for (Connection* conn : m_genotype.connections)
		{
			if (conn->in == masterConnection->out)
			{
				branches.push_back(conn);
			}
		}

		for (Connection* conn : branches)
		{
			return connectionIsCyclic(masterConnection, conn);
		}
	}

	return false;
}
