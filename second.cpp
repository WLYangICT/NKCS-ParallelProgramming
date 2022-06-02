#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include<vector>
#include<iostream>
#include<string>
#include<bitset>
#include<sys/time.h>
using namespace std;
FILE *fi;
FILE *fp;
struct _INDEX {
	unsigned int  len;
	unsigned int *arr;
} *idx;
int MAXARRS = 2000;
unsigned int i, alen;
unsigned int *aarr;
int j, n;

vector<int> strtoints(char* line) {
	vector<int> arr;
	int i = 0;
	int num = 0;
	while (line[i] == ' ' || (line[i] >= 48 && line[i] <= 57)) {
		num = 0;
		while (line[i] != ' ') {
			num *= 10;
			int tmp = line[i] - 48;
			num += tmp;
			i++;
		}
		i++;
		arr.push_back(num);
	}
	return arr;
}

int main() {
	struct timeval t1, t2;
	double timeuse = 0;

	//���ĵ��������ݼ�
	fi = fopen("/home/s2013774/SIMD/ExpIndex", "rb");
	if (NULL == fi) {
		printf("Can not open file ExpIndex!\n");
		return 1;
	}
	idx = (struct _INDEX *)malloc(MAXARRS * sizeof(struct _INDEX));
	if (NULL == idx) {
		printf("Can not malloc %d bytes for idx!\n", MAXARRS * sizeof(struct _INDEX));
		return 2;
	}
	j = 0;
	while (1) {
		fread(&alen, sizeof(unsigned int), 1, fi);
		if (feof(fi)) break;
		aarr = (unsigned int *)malloc(alen * sizeof(unsigned int));
		if (NULL == aarr) {
			printf("Can not malloc %d bytes for aarr!\n", alen * sizeof(unsigned short));
			return 3;
		}
		for (i = 0;i < alen;i++) {
			fread(&aarr[i], sizeof(unsigned int), 1, fi);
			if (feof(fi)) break;
		}
		if (feof(fi)) break;
		idx[j].len = alen;
		idx[j].arr = aarr;
		j++;
		if (j >= MAXARRS) {
			printf("Too many arrays(>=%d)!\n", MAXARRS);
			break;
		}
	}
	fclose(fi);
	//�����Ѿ���һ��idx����洢��������������ļ���idx[i].arr��ʾ��i���ؼ��ʵĵ�����������
	//������query_list�����ѯ�Ķ�ά���飬����ܵ�2000���ؼ��ʣ����������max��������Ϊ2000
	int numIndex = j;
	fp = fopen("/home/s2013774/SIMD/ExpQuery", "r");
	vector<vector<int> > query_list;

	vector<int> arr;
	char* line = new char[100];
	while ((fgets(line, 100, fp)) != NULL)
	{
		arr = strtoints(line);
		query_list.push_back(arr);
	}
	fclose(fp);
	gettimeofday(&t1, NULL);

	//��������ʼʵ�ֵ��������󽻼���
	//ʵ�ְ����󽻵�λͼ�洢����
	int QueryNum = 100;//��ѯ����
	for (int i = 0;i < QueryNum;i++) {
		int TermNum = query_list[i].size();
		bitset<25214976> * lists;//25214976=256*98496
		lists = new bitset<25214976>[TermNum];
		bitset<98496> *  second;
		second = new bitset<98496>[TermNum];
		for (int j = 0;j < TermNum;j++) {
			for (int k = 0;k < idx[query_list[i][j]].len;k++) {
				lists[j].set(idx[query_list[i][j]].arr[k]);//���������ĵ���ţ����ĵ���Ӧ�Ķ�����λ��Ϊ1
			}
		}
		//��һ���λ�����Ѿ��洢��ϣ���������������,���������Ѿ��õ����Ż�
		for (int i = 0;i < TermNum;i++) {
			long addrtemp1 = (long)&lists[i];
			for (int j = 0;j < 98496;j++) {
				bitset<256> * setptr = (bitset<256>*)(addrtemp1 + 32 * j);
				if (*setptr == 0) {
					;
				}
				else {
					second[i].set(j);
				}
			}
		}
		long addr = (long)&lists[0];
		for (int i = 1;i < TermNum;i++) {
			second[0] &= second[i];  //����������ֱ�Ӱ�λ�룬�ײ�SIMD
			for (int j = 0;j < 98496;j++) {
				if (second[0][j] == 1) {  //��jλ��1����Ҫ�ײ���
					bitset<256> * ptrtemp1 = (bitset<256> *)(addr + j * 32);
					long addrtemp2 = (long)&lists[i];
					bitset<256> * ptrtemp2 = (bitset<256> *)(addrtemp2 + j * 32);
					*ptrtemp1 &= *ptrtemp2;
				}
				else {
					bitset<256> * setptr = (bitset<256>*)(addr + 32 * j);
					*setptr = 0;	//ȫ������
				}
			}
		}
		vector<unsigned int> result;
		//���ת���Ż���list[0]���ǽ��λ������256Ϊ��λ�ж�
		long address = (long)&lists[0];
		for (int j = 0;j < 98496;j++) {
			bitset<256> * setptr = (bitset<256>*)(address + 32 * j);
			if (*setptr == 0) {
				;
			}
			//�����Ϊ0��ȥ�ײ��ж�
			else {
				for (int k = 0;k < 256;k++) {
					if (lists[0][j * 256 + k] == 1) {
						result.push_back(j * 256 + k);
					}
				}
			}
		}
		cout << result.size() << endl;
	}
	gettimeofday(&t2, NULL);
	timeuse += (t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec;

	cout << "time_use=" << timeuse << endl;

	for (j = 0;j < n;j++) free(idx[j].arr);
	free(idx);

	return 0;
}
