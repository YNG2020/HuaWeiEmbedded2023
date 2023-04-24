//#pragma once
//#include <iostream>
//#include <ctime>
//#include <cstdlib>
//#include <vector>
//#include <fstream>
//#include <iostream>  
//#include <fstream>
//using namespace std;
//
//int NMAX = 5000, NMIN = 2;
//int MMAX = 5000, MMIN = 2;
//int TMAX = 10000, TMIN = 2;
//int PMAX = 80, PMIN = 2;
//int DMAX = 1000, DMIN = 2;
//
//void randomDataGen();
//
//int main() {
//	randomDataGen();
//}
//
//void randomDataGen() {
//
//	double pressureBus = 0.5;
//	cout << "������ҵ��ѹ��ֵ�� ����ΧΪ0~1��Ĭ��Ϊ0.5�� ";
//	cin >> pressureBus;
//	while (pressureBus < 0 || pressureBus > 1) {
//		cout << "ҵ��ѹ��ֵ��Χ����ȷ������������!\n ";
//		cout << "������ҵ��ѹ��ֵ�� ����ΧΪ0~1��Ĭ��Ϊ0.5�� ";
//		cin >> pressureBus;
//	}
//	vector<vector<int>> rEdge, rBus;
//	int rN, rM, rT, rP, rD;
//	srand(time(NULL) + rand() % 1000);  // �������������  
//	rN = rand() % (NMAX - NMIN + 1) + NMIN;  // ��������� 
//	while (true) {
//		rM = rand() % (MMAX - MMIN + 1) + MMIN;  // ���������  
//		if (rM >= rN - 1)
//			break;
//	}
//	rT = rand() % (TMAX - TMIN + 1) + TMIN;  // ���������  
//	rP = rand() % (PMAX - PMIN + 1) + PMIN;  // ���������  
//	rD = rand() % (DMAX - DMIN + 1) + DMIN;  // ���������
//
//	rEdge.resize(rM);
//	for (auto& one : rEdge) {
//		one.resize(3);
//	}
//
//	// �����ɽڵ���ΪN����������ÿ��0 < t < N�����ѡ��һ��[0, t)�еĽڵ���Ϊ���ʼ�ڵ�
//	for (int t = 1; t < rN; ++t) {
//		int s = rand() % t;
//		rEdge[t - 1][0] = s;
//		rEdge[t - 1][1] = t;
//		rEdge[t - 1][2] = rand() % rD + 1;	// ��֤�������0�Ҳ�����rD
//	}
//
//	for (int i = 0; i + rN - 1 < rM; ++i) {
//
//		int s = rand() % rN;	// ��������� 1
//		int t = rand() % rN;	// ��������� 2  
//
//		while (true) {   // ��������������ͬ������������ 
//			if (s != t)
//				break;
//			t = rand() % rN;
//		}
//
//		if (s > t)
//			swap(s, t);
//
//		rEdge[i + rN - 1][0] = s;
//		rEdge[i + rN - 1][1] = t;
//		rEdge[i + rN - 1][2] = rand() % rD + 1;	// ��֤�������0�Ҳ�����rD
//	}
//
//	rBus.resize(rT);
//	int cnt = 0;
//	for (auto& one : rBus) 
//		one.resize(2);
//
//	for (int i = 0; i < rT; ++i) {
//		int repeatBusCnt = max(1, int(pressureBus * rP * (rand() % rP + 1)));
//
//		int S = rand() % rN;	// ��������� 1
//		int T = rand() % rN;	// ��������� 2  
//
//		do {   // ��������������ͬ������������ 
//			if (S != T)
//				break;
//			T = rand() % rN;
//		} while (true);
//		if (S > T)
//			swap(S, T);
//		for (int j = 0; j < repeatBusCnt && i + j < rT; ++j) {
//			rBus[i + j][0] = S;
//			rBus[i + j][1] = T;
//		}
//		i = i + repeatBusCnt - 1;
//
//	}
//
//	ofstream outfile("data.txt");
//	outfile << rN << " " << rM << " " << rT << " " << rP << " " << rD << endl;
//	for (int i = 0; i < rM; ++i)
//		outfile << rEdge[i][0] << " " << rEdge[i][1] << " " << rEdge[i][2] << endl;
//	for (int i = 0; i < rT; ++i)
//		outfile << rBus[i][0] << " " << rBus[i][1] << endl;
//
//	outfile.close();
//
//}
