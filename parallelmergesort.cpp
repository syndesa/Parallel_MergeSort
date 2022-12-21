#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <string.h>
#include <chrono>

using namespace std;

const int minDepth = std::thread::hardware_concurrency();
mutex threadLock; 
int depth {0}; 
int sizeT {0};

void incDepth(){
	threadLock.lock();
	depth++;
	threadLock.unlock();
}

class Name {
	public:
		char* firstName = (char*)malloc(sizeof(char)*25);
		char* lastName = (char*)malloc(sizeof(char)*25);

	Name(string fullName, int delim) 
	{
		fullName.copy(firstName, delim , 0);
		firstName[delim] = '\0';
		fullName.copy(lastName, fullName.size() - delim, delim+1);
		lastName[fullName.size() - (delim+1)] = '\0';
	};
	Name(){};
};

Name* loadFile(string fileName) {
	sizeT = 0;
	int size = 500;
	int iter = 0;
	Name* Names = (Name*)malloc(sizeof(Name) * size);
	string name;
	ifstream NamesFile(fileName);
	while (getline(NamesFile, name)) 
	{
		Name aName;
		if (name.size() > 0) 
		{
			for (int i = 0; i < name.size(); i++) 
			{
				if (name[i] == ' ') 
				{
					aName = Name(name, i);
					break;
				}
			}
			Names[iter] = aName;
			iter++;
		}

		if ( iter > size - 1) 
		{
			size *= 2 ;
			Names = (Name*)realloc(Names, sizeof(Name)*size);
		}
	};
	NamesFile.close();
	sizeT = iter; 
	return Names;
}


void writeFile(Name* names, string outputFile){
	ofstream OutputFile(outputFile);
	if (OutputFile.is_open()) {
		for (int i = 0; i < sizeT; i++) {
			OutputFile << names[i].firstName << ' ' << names[i].lastName << endl;
		}
	}

};

void merge(Name* names, int left, int right, int mid) {
	int i {left} ;
	int k {left} ;
	int j {mid + 1} ;
	Name* temp = (Name*)malloc(sizeof(Name) * (right - left + 1)) ;

	while (i<=mid && j<=right){
		if (strcmp(names[i].lastName , names[j].lastName) < 0)
		{
			temp[k - left] = names[i];
			k++;
			i++;
		}
		else if (strcmp(names[i].lastName , names[j].lastName) == 0){
			if (strcmp(names[i].firstName , names[j].firstName) < 0){
				temp[k - left] = names[i];
				k++;
				i++;
			}
			else {
				temp[k - left] = names[j];
				k++;
				j++;
			}
		}
		else {
			temp[k - left] = names[j];
			k++;
			j++;
		}
	}
	while (i <= mid) {
		temp[k - left] = names[i];
		k++;
		i++;
	}
	while (j <= right) {
		temp[k - left] = names[j];
		k++;
		j++;
	}

	for (i = left; i < k; i++) {
		names[i] = temp[i - left];
	}

	free(temp);
};


void mergeSort(Name* names , int left, int right) {
	int mid; 
	if (left < right) {
		mid = (right + left) / 2;
		mergeSort(names, left, mid);
		mergeSort(names, mid + 1, right);
		merge(names, left, right, mid);
	}
}

void parallelMergeSort(Name* names , int left, int right) {
	int mid; 
	if (left < right) {
		mid = (right + left) / 2;
		
		if (depth > minDepth) {
		mergeSort(names, left, mid);
		mergeSort(names, mid + 1, right);
		}
		else {
			incDepth();
			thread t1(mergeSort, names, left, mid);
			incDepth();
			thread t2(mergeSort, names, mid + 1, right);
			t1.join();
			t2.join();
		}
		merge(names, left, right, mid);
	}
}



int main() {
	Name* Names = loadFile("names.txt");

	auto parStart = chrono::high_resolution_clock::now();
	parallelMergeSort(Names, 0, sizeT - 1);
	auto parStop  = chrono::high_resolution_clock::now();
	auto parDuration = chrono::duration_cast<chrono::microseconds>(parStop - parStart);


	Name* NamesSeq = loadFile("names.txt");
	auto seqStart = chrono::high_resolution_clock::now();
	mergeSort(NamesSeq, 0, sizeT - 1);
	auto seqStop = chrono::high_resolution_clock::now();
	auto seqDuration = chrono::duration_cast<chrono::microseconds>(seqStop - seqStart);



	cout << "Sequential Operation: " << seqDuration.count() << " us" << endl;
	cout << "Parallel Operation: " << parDuration.count() << " us";

	writeFile(Names, "parrSorted.txt");
	writeFile(NamesSeq, "SeqSorted.txt");
	return 0;
}

	