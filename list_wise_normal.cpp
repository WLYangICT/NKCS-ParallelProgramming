#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include<vector>
#include<iostream>
#include<string>
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
	while (line[i] == ' '||(line[i]>=48&&line[i]<=57)) {
		num = 0;
		while(line[i] != ' ') {
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

bool find(unsigned int e, _INDEX list) {
	for (int i = 0;i < list.len;i++) {
		if (e == list.arr[i]) {
			return true;
		}
	}
	return false;
}

int main() {
	
	fi = fopen("ExpIndex", "rb");
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

	fp = fopen("ExpQuery", "r");
	vector<vector<int> > query_list;

	vector<int> arr;
	char* line = new char[100];
	while ((fgets(line,100,fp)) != NULL)
	{
		arr = strtoints(line);
		query_list.push_back(arr);
	}
	fclose(fp);
	
	
	//��������ʼʵ�ֵ��������󽻼���
	//ʵ�ְ����󽻵�ƽ���㷨
	vector<vector<unsigned int> > results;
	for (int i = 0;i < query_list.size();i++) {  //��������ѯ����
		int num = query_list[i].size();
		_INDEX* indexArr = new _INDEX[num];
		for (int j = 0;j < num;j++) {
			indexArr[j] = idx[query_list[i][j]];
		}
		vector<unsigned int> result;
		for (int i = 0;i < indexArr[0].len;i++) {
			result.push_back(indexArr[0].arr[i]);
		}
		//��indexArr����ļ���������
		//���������õ�һ�������С���ɣ�����϶�Խ��Խ��
		for (int i = 1;i < num;i++) { //�������
			for (int j = 0;j < result.size();j++) {
				if (!find(result[j], indexArr[i])) {
					//ɾ�������Ԫ��
					for (int k = j;k < result.size()-1;k++) {
						result[k] = result[k + 1];
					}
					result.pop_back();
				}
			}
		}
		results.push_back(result);
	}

	for (int i = 0;i < results.size();i++) {
		for (int j = 0;j < results[i].size();j++) {
			cout << results[i][j] << " ";
		}
		cout << endl;
	}

	for (j = 0;j < n;j++) free(idx[j].arr);
	free(idx);
	return 0;
}