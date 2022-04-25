#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include<vector>
#include<iostream>
#include<string>
#include<bitset>
#include<arm_neon.h>
#include<pthread.h>
using namespace std;

//ȫ�ֱ������󽻵�λ�������׵�ַ
bitset<25214976>* first1; //�ײ������׵�ַ
bitset<196992>* second1;  // ���������׵�ַ
bitset<25214976>* first2; //�ײ������׵�ַ
bitset<196992>* second2;  // ���������׵�ַ

FILE *fi;
FILE *fp;
struct _INDEX {
	unsigned int  len;
	unsigned int *arr;
} *idx;
struct package {
	bitset<25214976> first; //�ײ�����
	bitset<196992> second;  // ��������
	_INDEX index;  //�ؼ��ʵĵ����б�
};
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

//�����߳�ִ�еĴ�����
void* tobits (void* parm) {
	//package pack = *(package*)parm;  //�Ѳ�����ת����package��Ҫ��pack�еĹؼ����б�ת����λ�����Ͷ�������
	for (int k = 0;k < (*(package*)parm).index.len;k++) {
		(*(package*)parm).first.set((*(package*)parm).index.arr[k]);//���������ĵ���ţ����ĵ���Ӧ�Ķ�����λ��Ϊ1
	}
	//λ����ת���ɹ�
	long addrtemp = (long)&(*(package*)parm).first;
	for (int j = 0;j < 196992;j++) {
		bitset<128> * setptr = (bitset<128>*)(addrtemp + 16 * j);
		if (*setptr == 0) {
			;
		}
		else {
			(*(package*)parm).second.set(j);
		}
	}
	//�������������ɹ�
	return NULL;
}
//�����߳�ִ�еĴ�����,�������ʵ��
void* Pthread_and(void* parm) {
	long rank = (long)parm;
	bitset<12312>* mysecond1 = (bitset<12312>*)((long)second1 + rank * 1539);
	bitset<12312>* mysecond2 = (bitset<12312>*)((long)second2 + rank * 1539);
	bitset<1575936>* myfirst1 = (bitset<1575936>*)((long)first1 + rank * 196992);
	bitset<1575936>* myfirst2 = (bitset<1575936>*)((long)first2 + rank * 196992);

	*mysecond1 &= *mysecond2;
	for (int j = 0;j < 12312;j++) {
		if ((*mysecond1)[j] == 1) {
			long addrtemp1 = (long)myfirst1;
			int* ptrtemp1 = (int *)(addrtemp1 + j * 16);
			long addrtemp2 = (long)myfirst2;
			int* ptrtemp2 = (int *)(addrtemp2 + j * 16);
			int32x4_t temp1 = vld1q_s32(ptrtemp1);
			int32x4_t temp2 = vld1q_s32(ptrtemp2);
			temp1 = vandq_s32(temp1, temp2);
			vst1q_s32(ptrtemp1, temp1);
		}
		else {
			long addr = (long)myfirst1;
			bitset<128> * setptr = (bitset<128>*)(addr + 16 * j);
			*setptr = 0;
		}
	}
	return NULL;
}
int main() {
	//���ĵ��������ݼ�
	fi = fopen("/home/s2013774/Pthread/ExpIndex", "rb");
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

	//��������ʼʵ�ֵ��������󽻼���
	//ʵ�ְ����󽻵�λͼ�洢����
	int QueryNum = 1000;//��ѯ����
	vector<vector<unsigned int>> results;
	for (int i = 0;i < QueryNum;i++) {
		int TermNum = query_list[i].size();
		package* packages = new package[TermNum];
		for (int j = 0;j < TermNum;j++) {
			packages[j].index = idx[query_list[i][j]];
		}

		long thread;
		int thread_count = TermNum;//�߳������ǹؼ��ʵ�����
		pthread_t* thread_handles;
		thread_handles = new pthread_t[thread_count];
		for (thread = 0;thread < thread_count;thread++) {
			pthread_create(&thread_handles[thread], NULL, tobits, (void*)&packages[thread]);
		}
		for (thread = 0;thread < thread_count;thread++) {
			pthread_join(thread_handles[thread], NULL);
		}
		free(thread_handles);

		first1 = &(packages[0].first);
		second1 = &(packages[0].second);
		for (int i = 1;i < TermNum;i++) {
			//��ȫ�ֱ�����ַ��ֵ
			first2 = &(packages[i].first);
			second2 = &(packages[i].second);
			//���̻߳�������
			thread_count = 16;
			pthread_t* thread_handle;
			thread_handle = new pthread_t[thread_count];
			for (thread = 0;thread < thread_count;thread++) {
				pthread_create(&thread_handle[thread], NULL, Pthread_and, (void*)thread);
			}
			for (thread = 0;thread < thread_count;thread++) {
				pthread_join(thread_handle[thread], NULL);
			}
			free(thread_handle);
		}


		vector<unsigned int> result;
		//���ת���Ż���list[0]���ǽ��λ������256Ϊ��λ�ж�
		long address = (long)&packages[0].first;
		for (int j = 0;j < 98496;j++) {
			bitset<256> * setptr = (bitset<256>*)(address + 32 * j);
			if (*setptr == 0) {
				;
			}
			//�����Ϊ0��ȥ�ײ��ж�
			else {
				for (int k = 0;k < 256;k++) {
					if (packages[0].first[j * 256 + k] == 1) {
						result.push_back(j * 256 + k);
					}
				}
			}
		}
		//������һ�β�ѯ
		//results.push_back(result);
		cout << result.size() << endl;
	}

	for (j = 0;j < n;j++) free(idx[j].arr);
	free(idx);

	return 0;
}
