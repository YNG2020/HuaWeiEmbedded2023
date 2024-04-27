#ifndef UTILITY_FUNCTION_H
#define UTILITY_FUNCTION_H

void addEdge(int s, int t, int d);	// 加边函数，s起点，t终点，d距离
void addBus(int start, int end);	// 加业务函数
void inputFromJudger(); // 标准输入流，用于判题器
void inputFromFile();	// 文件输入流，用于文件
void outputForJudger(); // 标准输出流，用于判题器
void outputForFile();	// 文件输出流，用于文件
void outputStatistic();	// 输出业务在网络上的分布的统计结果
#endif