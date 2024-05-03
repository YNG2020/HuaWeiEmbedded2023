#include <vector>
#include <cmath>
#include <time.h>
#include <fstream>
#include "global_struct.h"
#include "Configure.h"
#include "global_var.h"
#include "UtilityFunction.h"
#include "Solution.h"

using namespace std;

int main()
//int main(int argc, char* argv[])
{
    //sortTranStrategy = atoi(argv[1]);

    if (Configure::forJudger)   // 用于评测
        inputFromJudger();
    else                        // 用于本地测试
        inputFromFile();

    Solution solution;
    solution.runStrategy();     // 运行总策略

    if (Configure::forJudger)   // 用于评测
        outputForJudger();
    else                        // 用于本地测试
        outputForFile();

    return totCost;
}
