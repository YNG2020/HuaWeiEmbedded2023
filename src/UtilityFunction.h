#ifndef UTILITY_FUNCTION_H
#define UTILITY_FUNCTION_H
#include <global_struct.h>
#include <fstream>

void addEdge(int s, int t, int d);	// 加边函数，s起点，t终点，d距离
void deleteEdge(int edgeID);	// 删边函数
void addTran(int start, int end);	// 加业务函数
void inputFromJudger(); // 标准输入流，用于判题器
void inputFromFile();	// 文件输入流，用于文件
void outputForJudger(); // 标准输出流，用于判题器
void outputForFile();	// 文件输出流，用于文件
void outputStatistic();	// 输出业务在网络上的分布的统计结果
void calculateCost();	// 成本计算函数
void sumUPAllUsedEdge();	// 统计所有业务上用掉的边的数量
void backtrackPathAndPrint(Transaction& tran, vector<int>& tmpOKPath, int p, ofstream& myCout);	// 回溯路径并输出
#endif