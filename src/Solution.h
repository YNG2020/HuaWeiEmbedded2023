#ifndef SOLUTION_H
#define SOLUTION_H

#include "global_struct.h"
#include "global_var.h"

class Solution
{
public:

	Solution();
	void runStrategy();
	void sumUptheAllocationPressure();
	void BFS_loadTran(Transaction& tran, bool ifTryDeleteEdge);
	bool BFS_detectPath(Transaction& tran, int blockEdge);
	void BFS_addNewEdge(Transaction& tran);
	void BFS_tranStatistic(Transaction& tran);
	void loadTran(int tranID, bool ifTryDeleteEdge);
	void loadMultiplier(int tranID);
	void backtrackPath(Transaction& tran);
	void preAllocateTran();
	void reAllocateTran(int HLim);
	void tryDeleteEdge(bool increasing = true, bool ifSimDeleteEdge = false);
	bool tryDeleteEdgeSim(int oriNewEdgeNum, int oriUsedEdgeNum, int curUsedEdgeNum, bool increasing = true);
	void performDeleteEdge(int idxEdge, int tranCnt, const vector<int>& lastTranIDs);
	void recoverNetwork(int lastTranID, int lastPileID);
	void reloadTran(int lastTranID, int lastPileID, vector<int>& lastEdgesOfShortestPaths);
	void sortTran();
	void resetEverything();
	void transferTranInMultiEdge(Transaction& tran);
	void getReallocatedTranID(vector<int>& totTranIDx, vector<int>& tranIDx, int gap, int gapStart);
	vector<int> tmpOKPath;      // 用于存储在某个通道下找到的路径

	// 策略参数
	//bool forSortTran = false;		// 是否对加载业务进行排序
	//bool forDoubleSortTran = false;		// 是否对加载业务进行二次排序
	//bool forTryDeleteEdge = false;	// 是否尝试删除边
	//bool forIter = false;			// 是否使用迭代策略
	//bool forBatchTranReAllocate = false;		// 是否批量重新分配业务
	//bool forNoDetour = false;		// 是否禁止绕行，即是否使用pathSizeLimRatio参数
		
	bool forSortTran = true;		// 是否对加载业务进行排序
	bool forDoubleSortTran = false;		// 是否对加载业务进行二次排序
	bool forTryDeleteEdge = true;	// 是否尝试删除边
	bool forIter = true;			// 是否使用迭代策略
	bool forBatchTranReAllocate = true;		// 是否批量重新分配业务
	bool forNoDetour = true;		// 是否禁止绕行，即是否使用pathSizeLimRatio参数
	
	float pathSizeLimRatio = 3.0;			// 限制找到的路径长度相对于理论上的最短路径长度的倍数
	int cntLimit = 600;					// 大迭代次数限制
	float reAllocateRatio = 0.05;		// 重新分配的业务占全体业务的比例
	int iterCnter = 0;					// 小迭代次数
};

#endif // SOLUTION_H
