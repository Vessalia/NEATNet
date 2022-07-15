#pragma once

#include "NetComponents.h"
#include <vector>

class Net
{
public:
	Net(size_t numInputs, size_t numOutputs);

	~Net();

	void receiveInput(const std::vector<double>& inputVals);

	void attemptMutation(double mutationRate);

private:
	NetComponents::Genotype m_genotype;
	size_t m_innovation;

	size_t m_numInputs;
	size_t m_numOutputs;

	static int randInt(int val) { return rand() % val; }
	static double randDecimal() { return rand() / double(RAND_MAX); }

	void addHiddenNode();
	void addRandomConnection();
	void addConnection(size_t in, size_t out);

	bool checkValidConnection(size_t in, size_t out) const;
	NetComponents::Connection* checkExistingConnection(size_t in, size_t out);

	void removeHiddenNode();
	void removeNode(NetComponents::Node* node);
	void removeRandomConnection();
	void removeConnection(size_t in, size_t out);
	void removeConnection(NetComponents::Connection* connection);

	bool cullConnections();
};
