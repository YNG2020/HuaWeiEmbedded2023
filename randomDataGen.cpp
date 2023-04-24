#pragma once
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <iostream>  
#include <fstream>
using namespace std;

int NMAX = 5000, NMIN = 2;
int MMAX = 5000, MMIN = 2;
int TMAX = 10000, TMIN = 2;
int PMAX = 80, PMIN = 2;
int DMAX = 1000, DMIN = 2;

void randomDataGen();

int main() {
	randomDataGen();
}

void randomDataGen() {

	double pressureBus = 0.5;
	cout << "请输入业务压力值： （默认为0.5） ";
	cin >> pressureBus;
	vector<vector<int>> rEdge, rBus;
	int rN, rM, rT, rP, rD;
	srand(time(NULL) + rand() % 1000);  // 设置随机数种子  
	rN = rand() % (NMAX - NMIN + 1) + NMIN;  // 生成随机数 
	while (true) {
		rM = rand() % (MMAX - MMIN + 1) + MMIN;  // 生成随机数  
		if (rM >= rN - 1)
			break;
	}
	rT = rand() % (TMAX - TMIN + 1) + TMIN;  // 生成随机数  
	rP = rand() % (PMAX - PMIN + 1) + PMIN;  // 生成随机数  
	rD = rand() % (DMAX - DMIN + 1) + DMIN;  // 生成随机数

	rEdge.resize(rM);
	for (auto& one : rEdge) {
		one.resize(3);
	}

	// 先生成节点数为N的数。对于每个0 < t < N，随机选择一个[0, t)中的节点作为其初始节点
	for (int t = 1; t < rN; ++t) {
		int s = rand() % t;
		rEdge[t - 1][0] = s;
		rEdge[t - 1][1] = t;
		rEdge[t - 1][2] = rand() % rD + 1;	// 保证距离大于0且不大于rD
	}

	for (int i = 0; i + rN - 1 < rM; ++i) {

		int s = rand() % rN;	// 生成随机数 1
		int t = rand() % rN;	// 生成随机数 2  

		while (true) {   // 如果两个随机数相同，则重新生成 
			if (s != t)
				break;
			t = rand() % rN;
		}

		if (s > t)
			swap(s, t);

		rEdge[i + rN - 1][0] = s;
		rEdge[i + rN - 1][1] = t;
		rEdge[i + rN - 1][2] = rand() % rD + 1;	// 保证距离大于0且不大于rD
	}

	rBus.resize(rT);
	int cnt = 0;
	for (auto& one : rBus) 
		one.resize(2);

	for (int i = 0; i < rT; ++i) {
		int repeatBusCnt = max(1, int(pressureBus * (rand() % rP + 1)));

		int S = rand() % rN;  // 生成随机数 1
		int T = rand() % rN;   // 生成随机数 2  

		do {   // 如果两个随机数相同，则重新生成 
			if (S != T)
				break;
			T = rand() % rN;
		} while (true);
		if (S > T)
			swap(S, T);
		for (int j = 0; j < repeatBusCnt && i + j < rT; ++j) {
			rBus[i + j][0] = S;
			rBus[i + j][1] = T;
		}
		i = i + repeatBusCnt - 1;

	}

	ofstream outfile("data.txt");
	outfile << rN << " " << rM << " " << rT << " " << rP << " " << rD << endl;
	for (int i = 0; i < rM; ++i)
		outfile << rEdge[i][0] << " " << rEdge[i][1] << " " << rEdge[i][2] << endl;
	for (int i = 0; i < rT; ++i)
		outfile << rBus[i][0] << " " << rBus[i][1] << endl;

	outfile.close();

}
