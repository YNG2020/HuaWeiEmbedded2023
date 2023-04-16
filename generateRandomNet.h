#pragma once
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
using namespace std;

int NMAX = 5000, NMIN = 2;
int MMAX = 5000, MMIN = 2;
int TMAX = 10000, TMIN = 2;
int PMAX = 80, PMIN = 2;
int DMAX = 1000, DMIN = 2;

void generateRandomNet(vector<vector<int>>& rEdge, vector<vector<int>>& rBus, int& rN, int& rM, int& rT, int& rP, int& rD) {

	srand(time(NULL) + rand() % 1000);  // 设置随机数种子  
	rN = rand() % (NMAX - NMIN + 1) + NMIN;  // 生成随机数  
	//rM = rand() % (MMAX - MMIN + 1) + MMIN;  // 生成随机数  
	rT = rand() % (TMAX - TMIN + 1) + TMIN;  // 生成随机数  
	rP = rand() % (PMAX - PMIN + 1) + PMIN;  // 生成随机数  
	rD = rand() % (DMAX - DMIN + 1) + DMIN;  // 生成随机数

	int rM1 = min(MMAX, 3 * rN);
	rM = rM1 + rN - 1;

	rEdge.resize(rM);
	for (auto& one : rEdge) {
		one.resize(3);
	}

	for (int i = 0; i < rM1; ++i) {

		srand(time(NULL) + rand() % 1000);  // 设置随机数种子 
		int s = rand() % rN;  // 生成随机数 1
		srand(time(NULL) + rand() % 1000);  // 设置随机数种子 
		int t = rand() % rN;   // 生成随机数 2  

		do {   // 如果两个随机数相同，则重新生成 
			if (s != t)
				break;
			srand(time(NULL) + rand() % 1000);  // 设置随机数种子 
			t = rand() % rN;
		} while (true);

		int d = rand() % rD + 1;	// 保证距离大于0且不大于rD
		rEdge[i][0] = s;
		rEdge[i][1] = t;
		rEdge[i][2] = d;

	}

	for (int i = rM1; i < rM; ++i) {

		int d = rand() % rD + 1;	// 保证距离大于0且不大于rD
		rEdge[i][0] = 0;
		rEdge[i][1] = i - rM1 + 1;
		rEdge[i][2] = d;
	}

	rBus.resize(rT);
	for (auto& one : rBus) {
		one.resize(2);

		srand(time(NULL) + rand() % 1000);  // 设置随机数种子 
		int S = rand() % rN;  // 生成随机数 1
		srand(time(NULL) + rand() % 1000);  // 设置随机数种子 
		int T = rand() % rN;   // 生成随机数 2  
  
		do {   // 如果两个随机数相同，则重新生成 
			if (S != T)
				break;

			srand(time(NULL) + rand() % 1000);  // 设置随机数种子 
			T = rand() % rN;
		} while(true);

		one[0] = S;
		one[1] = T;

	}


}