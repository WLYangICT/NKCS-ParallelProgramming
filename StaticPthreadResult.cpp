#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include<vector>
#include<iostream>
#include<string>
#include<bitset>
#include<arm_neon.h>
#include<pthread.h>
#include<semaphore.h>
using namespace std;

pthread_mutex_t amutex = PTHREAD_MUTEX_INITIALIZER;
int QueryNum = 100;//��ѯ��������Ϊȫ�ֱ���

bitset<25214976>* resultbits;

vector<unsigned int> result;
vector<vector <unsigned int>> results;

//�ź�������,7���߳�
sem_t sem_main; //���������ź���
sem_t sem_workerstart[7]; // ÿ���߳����Լ�ר�����ź���
sem_t sem_workerend[7];

struct package {
	int id;
	vector<unsigned int> myresult;
};

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

//�̺߳���
void* threadFunc(void* param) {
	int t_id = ((package*)param)->id;
	for (int k = 0; k < QueryNum; k++) {
		sem_wait(&sem_workerstart[t_id]);     // �������ȴ���������󽻲����������Լ�ר�����ź�����
		//ѭ����������
		long addr = (long)resultbits;
		long myaddr = addr + 450272 * t_id;//��ʼ��ַ
		if (t_id == 6) {//���߸��̸߳���ʣ��3601920λ
			for (int j = 0;j < 14070;j++) {
				bitset<256> * setptr = (bitset<256>*)(myaddr + 32 * j);
				if (*setptr == 0) {
					;
				}
				//�����Ϊ0��ȥ�ײ��ж�
				else {
					for (int k = 0;k < 256;k++) {
						if (((*resultbits)[t_id * 450272 + j * 256 + k]) == 1) {
							((package*)param)->myresult.push_back(t_id * 450272 + j * 256 + k);
						}
					}
				}
			}
		}
		else {  //�����̸߳���14071*256λ
			for (int j = 0;j < 14071;j++) {
				bitset<256> * setptr = (bitset<256>*)(myaddr + 32 * j);
				if (*setptr == 0) {
					;
				}
				//�����Ϊ0��ȥ�ײ��ж�
				else {
					for (int k = 0;k < 256;k++) {
						if (((*resultbits)[t_id*450272+j*256+k]) == 1) {
							((package*)param)->myresult.push_back(t_id * 450272 + j * 256 + k);
						}
					}
				}
			}
		}
		//��ȡ��һ���Լ��̵߳�myresult��Ҫ��ӵ�result��,��Ҫ����ͬ�����ƣ����Կ��ǻ�����
		//��ӽ���result
		pthread_mutex_lock(&amutex);
		for (int x = 0;x < ((package*)param)->myresult.size();x++) {
			result.push_back(((package*)param)->myresult[x]);
		}
		pthread_mutex_unlock(&amutex);
		
		((package*)param)->myresult.clear();//�����ʱ����б�Ϊ�´���׼��
		sem_post(&sem_main); //�������߳�
		sem_wait(&sem_workerend[t_id]); //�������ȴ����̻߳��ѽ�����һ��
	}
	pthread_exit(NULL);
	return NULL;
}

int main() {
	
	pthread_mutex_init(&amutex, NULL);
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
	fp = fopen("/home/s2013774/Pthread/ExpQuery", "r");
	vector<vector<int> > query_list;

	vector<int> arr;
	char* line = new char[100];
	while ((fgets(line, 100, fp)) != NULL)
	{
		arr = strtoints(line);
		query_list.push_back(arr);
	}
	fclose(fp);

	//��ʼ���ź���
	sem_init(&sem_main, 0, 0);
	for (int i = 0;i < 7;i++) {
		sem_init(&sem_workerstart[i], 0, 0);
		sem_init(&sem_workerend[i], 0, 0);
	}

	//�����߳�
	pthread_t handles[7];// ������Ӧ�� Handle
	//����������
	package packages[7];
	//threadParam_t param[NUM_THREADS];// ������Ӧ���߳����ݽṹ
	for (int t_id = 0; t_id < 7; t_id++) {
		packages[t_id].id = t_id;
		pthread_create(&handles[t_id], NULL, threadFunc, (void*)&packages[t_id]);
	}

	//��������ʼʵ�ֵ��������󽻼���
	
	for (int i = 0;i < QueryNum;i++) {
		//��ʼ����ÿ�β�ѯ
		//����λ�����Ͷ�������
		int TermNum = query_list[i].size();
		bitset<25214976> * lists;//25214976=128*196992
		lists = new bitset<25214976>[TermNum];
		bitset<196992> *  second;
		second = new bitset<196992>[TermNum];
		for (int j = 0;j < TermNum;j++) {
			for (int k = 0;k < idx[query_list[i][j]].len;k++) {
				lists[j].set(idx[query_list[i][j]].arr[k]);//���������ĵ���ţ����ĵ���Ӧ�Ķ�����λ��Ϊ1
			}
		}
		//��һ���λ�����Ѿ��洢��ϣ���������������,���������Ѿ��õ����Ż�
		for (int i = 0;i < TermNum;i++) {
			long addrtemp1 = (long)&lists[i];
			for (int j = 0;j < 196992;j++) {
				bitset<128> * setptr = (bitset<128>*)(addrtemp1 + 16 * j);
				if (*setptr == 0) {
					;
				}
				else {
					second[i].set(j);
				}
			}
		}

		//��ʼ��λ�����
		for (int i = 1;i < TermNum;i++) {
			second[0] &= second[i];  //����������ֱ�Ӱ�λ�룬�ײ�SIMD
			for (int j = 0;j < 196992;j++) {
				if (second[0][j] == 1) {  //��jλ��1����Ҫ�ײ���
					long addrtemp1 = (long)&lists[0];
					int* ptrtemp1 = (int *)(addrtemp1 + j * 16);
					long addrtemp2 = (long)&lists[i];
					int* ptrtemp2 = (int *)(addrtemp2 + j * 16);
					int32x4_t temp1 = vld1q_s32(ptrtemp1);
					int32x4_t temp2 = vld1q_s32(ptrtemp2);
					temp1 = vandq_s32(temp1, temp2);
					vst1q_s32(ptrtemp1, temp1);
				}
				else {
					long addr = (long)&lists[0];
					bitset<128> * setptr = (bitset<128>*)(addr + 16 * j);
					*setptr = 0;//ȫ������
				}
			}
		}

		resultbits = &lists[0];

		//�����Ѿ��н��λ�����ˣ�ת���ɽ���б����þ�̬�̵߳�Pthread���
		//��ʼ���ѹ����߳�
		for (int t_id = 0; t_id < 7; t_id++) {
			sem_post(&sem_workerstart[t_id]);
		}
		//���߳�˯�ߣ��ȴ����еĹ����߳���ɴ�����ȥ����
		for (int t_id = 0; t_id < 7; t_id++) {
			sem_wait(&sem_main);
		}
		// ���߳��ٴλ��ѹ����߳̽�����һ�ִε���ȥ����
		for(int t_id = 0; t_id < 7; t_id++) {
			sem_post(&sem_workerend[t_id]);
		}
		cout << result.size() << endl;
		results.push_back(result);
		result.clear();
	}

	for (int t_id = 0; t_id < 7; t_id++) {
		pthread_join(handles[t_id], NULL);
	}	//���������ź���
	sem_destroy(&sem_main);
	for (int i = 0;i < 7;i++) {
		sem_destroy(&sem_workerstart[i]);
		sem_destroy(&sem_workerend[i]);
	}

	for (j = 0;j < n;j++) free(idx[j].arr);
	free(idx);

	return 0;
}
