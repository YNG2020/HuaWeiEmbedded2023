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
Solution solution;

// 主函数
int main()
{
    if (Configure::forJudger)
        inputFromJudger();
    else
        inputFromFile();
    solution.allocateBus();

    solution.ifTryDeleteEdge = false;

    int cnt = 0;

    if (Configure::forIterOutput)
        std::cout << "Original newEdge.size = " << newEdge.size() << endl;
    while (cnt < Configure::cntLimit) {

        solution.reAllocateBus(pow(2.71, -0.005 * cnt)*T);
        solution.tryDeleteEdge();
        if (Configure::forIterOutput)
            std::cout << "newEdge.size = " << newEdge.size() << endl;
        cnt = cnt + 1;
    }

    solution.ifLast = true;
    solution.tryDeleteEdge();
    solution.tryDeleteEdge();
    if (Configure::forIterOutput)
        std::cout << "newEdge.size = " << newEdge.size() << endl;

    if (Configure::forJudger)
        outputForJudger();
    else
        outputForFile();

    return 0;
}

