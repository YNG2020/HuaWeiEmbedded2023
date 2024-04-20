#ifndef SOLUTION_H
#define SOLUTION_H

class Solution
{
public:
	void init();
	void addEdge(int s, int t, int d);
	void addBus(int start, int end);
	void BFS_loadBus(Business& bus, bool ifLoadNewEdge);
	bool BFS_detectPath(Business& bus, int blockEdge);
	void BFS_addNewEdge(Business& bus);
	void loadBus(int busId, bool ifLoadRemain);
	void loadMultiplier(int busId);
	void allocateBus();
	void reAllocateBus(int HLim);
	void tryDeleteEdge();
	void reverseArray(vector<int>& arr);
	void outPut();
	void reCoverNetwork(int lastBusID, int lastPileId);
	void reloadBus(int lastBusID, int lastPileId, vector<int>& pathTmp);

};


#endif // SOLUTION_H
