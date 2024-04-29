#ifndef SOLUTION_H
#define SOLUTION_H

#include "global_struct.h"
#include "global_var.h"

class Solution
{
public:

	Solution();
	void runStrategy();
	void runStatistic();
	void BFS_loadTran(Transaction& tran, bool ifTryDeleteEdge);
	bool BFS_detectPath(Transaction& tran, int blockEdge);
	void BFS_addNewEdge(Transaction& tran);
	void BFS_detectMinPathSize(Transaction& tran);
	void BFS_tranStatistic(Transaction& tran);
	void loadTran(int tranID, bool ifTryDeleteEdge);
	void loadMultiplier(int tranID);
	void backtrackPath(Transaction& tran);
	void preAllocateTran();
	void reAllocateTran(int HLim);
	void tryDeleteEdge();
	void recoverNetwork(int lastTranID, int lastPileID);
	void reloadTran(int lastTranID, int lastPileID, vector<int>& pathTmp);
	void sortTran();
	vector<int> tmpOKPath;      // 用于存储在某个通道下找到的路径

	// 策略参数
	const bool forSortTran = true;		// 是否对加载业务进行排序
	const bool forIter = false;			// 是否使用迭代策略
	const bool forTryDeleteEdge = false;	// 是否尝试删除边
	float pathSizeLimRatio = 3.0;			// 限制找到的路径长度相对于理论上的最短路径长度的倍数
	int cntLimit = 20;						// 最大迭代次数
	float reAllocateTranNumFunBase = 2.71;	// 重新分配业务时，要被重分配的业务的数量函数（一个指数函数）的基数
	float reAllocateTranNumFunExpRatio = -0.005;	// 重新分配业务时，要被重分配的业务的数量函数（一个指数函数）的指数
};

#endif // SOLUTION_H
