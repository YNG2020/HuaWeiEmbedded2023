#ifndef SOLUTION_H
#define SOLUTION_H

#include "global_struct.h"
#include "global_var.h"

class Solution
{
public:

	Solution();
	void BFS_loadBus(Business& bus);
	bool BFS_detectPath(Business& bus, int blockEdge);
	void BFS_addNewEdge(Business& bus);
	void loadBus(int busId, bool ifLoadRemain);
	void loadMultiplier(int busId);
	void allocateBus();
	void reAllocateBus(int HLim);
	void tryDeleteEdge();
	void recoverNetwork(int lastBusID, int lastPileId);
	void reloadBus(int lastBusID, int lastPileId, vector<int>& pathTmp);

	bool ifLast = false;
	bool ifTryDeleteEdge = true;
	float pathSizeLimRatio = 3.0;
	int cntLimit = 20;	// 最大迭代次数
	float reAllocateBusNumFunBase = 2.71;
	float reAllocateBusNumFunExpRatio = -0.005;
	vector<int> tmpOKPath;      // 用于存储在某个通道下找到的路径
};


#endif // SOLUTION_H
