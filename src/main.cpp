#include <iostream>
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

// 主函数
int main()
{
    if (Configure::forJudger)
        inputFromJudger();
    else
        inputFromFile();

    Solution solution;
    solution.allocateBus();

    solution.ifTryDeleteEdge = false;

    int cnt = 0;

    if (Configure::forIterOutput && !Configure::forJudger)
        std::cout << "Original newEdge.size = " << newEdge.size() << endl;
    while (cnt < solution.cntLimit)
    {
        solution.reAllocateBus(pow(solution.reAllocateBusNumFunBase, solution.reAllocateBusNumFunExpRatio * cnt) * T);
        solution.tryDeleteEdge();
        if (Configure::forIterOutput && !Configure::forJudger)
            std::cout << "newEdge.size = " << newEdge.size() << endl;
        cnt = cnt + 1;
    }

    solution.ifLast = true;
    solution.tryDeleteEdge();
    solution.tryDeleteEdge();
    if (Configure::forIterOutput && !Configure::forJudger)
        std::cout << "newEdge.size = " << newEdge.size() << endl;

    if (Configure::forJudger)
        outputForJudger();
    else
        outputForFile();

    return 0;
}
