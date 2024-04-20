#ifndef SOLUTION_H
#define SOLUTION_H

#include "global_struct.h"

class Solution
{
public:
	void BFS_loadBus(Business& bus, bool ifLoadNewEdge);
	bool BFS_detectPath(Business& bus, int blockEdge);
	void BFS_addNewEdge(Business& bus);
	void loadBus(int busId, bool ifLoadRemain);
	void loadMultiplier(int busId);
	void allocateBus();
	void reAllocateBus(int HLim);
	void tryDeleteEdge();
	void reverseArray(vector<int>& arr);
	void reCoverNetwork(int lastBusID, int lastPileId);
	void reloadBus(int lastBusID, int lastPileId, vector<int>& pathTmp);

	bool ifLast = false;
	bool ifTryDeleteEdge = true;

};


#endif // SOLUTION_H
