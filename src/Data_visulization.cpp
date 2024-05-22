#include "Solution.h"
#include "UtilityFunction.h"
#include "global_var.h"
#include "global_struct.h"
#include <iostream>
#include <cmath>
#include <queue>
#include <algorithm>
#include <vector>
#include <random>
#include <fstream>
#include <time.h>
using namespace std;

void backtrackPathAndPrint(Transaction& tran, vector<int>& tmpOKPath, int p, ofstream& myCout)
{
    int curNode = tran.end;
    vector<int> path;
    while (tmpOKPath[curNode] != -1)
    {
        int edgeID = tmpOKPath[curNode];  // �洢��edge�����������ıߵ�ID
        path.push_back(edgeID / 2); // edgeID / 2��Ϊ����Ӧ��ĿҪ��
        curNode = edge[tmpOKPath[curNode]].from;
    }
    std::reverse(path.begin(), path.end());
    myCout << p << " ";
    for (int i = 0; i < path.size(); ++i)
    {
        myCout << path[i];
		if (i != path.size() - 1)
            myCout << " ";
    }
    myCout << endl;
}