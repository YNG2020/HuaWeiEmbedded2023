#include <vector>
#include <cmath>
#include <time.h>
#include <fstream>
#include "global_struct.h"
#include "configure.h"
#include "global_var.h"
#include "UtilityFunction.h"
#include "Solution.h"

using namespace std;

int main()
//int main(int argc, char* argv[])
{
    if (Configure::forJudger)   // 用于评测
        inputFromJudger();
    else                        // 用于本地测试
        inputFromFile();
    Solution solution;
    
    //solution.forSortTran = atoi(argv[1]);
    //solution.forDoubleSortTran = atoi(argv[2]);
    //solution.forTryDeleteEdge = atoi(argv[3]);
    //solution.forIter = atoi(argv[4]);
    //solution.forBatchTranReAllocate = atoi(argv[5]);
    //solution.forNoDetour = atoi(argv[6]);

    ofstream myCout("BFS_Finding_Path.txt");
    myCout.close();
    solution.runStrategy();     // 运行总策略

    if (Configure::forJudger)   // 用于评测
        outputForJudger();
    else                        // 用于本地测试
        outputForFile();

    //calculateCost();
    return newEdge.size() * 1000000 + totUsedEdge;
    //return 0;
}
