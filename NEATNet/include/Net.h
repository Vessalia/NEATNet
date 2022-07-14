#pragma once

#include "NetComponents.h"
#include <vector>

class Net
{
public:
	Net(size_t numInputs, size_t numOutputs);

	void receiveInput(const std::vector<double>& inputVals);

private:
	NetComponents::Genotype m_genotype;
	size_t m_innovation;

	size_t m_numInputs;

	void addHiddenNode();
	void addRandomConnection();
	void addConnection(size_t, size_t);

	bool checkValidConnection(size_t, size_t) const;
	NetComponents::Connection* checkExistingConnection(size_t, size_t);
};
