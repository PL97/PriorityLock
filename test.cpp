#include<pthread.h>
#include<stdio.h>
#include<unistd.h>
#include<iostream>
#include<vector>
#include<utility>
using namespace std;

#define MAX_THREAD 10

class PriorityLock{
public:
   void enter (int priority_level);
   void exit(); 

private: 
   vector<pair<pthread_cond_t*, int>> Queue;
   vector<pair<pthread_cond_t*, int>> ExeQueue;
   pthread_mutex_t lock;
};

bool meetCond(vector<pair<pthread_cond_t*, int>> Queue, vector<pair<pthread_cond_t*, int>> ExeQueue, int priority_level){
	return (ExeQueue.size() == 0 && Queue.front().second == priority_level);
}

void PriorityLock::enter(int priority_level){
	pthread_cond_t *myCV, *nextWaiter;
	pthread_mutex_lock(&lock);

	myCV = new pthread_cond_t();
	if(Queue.size() == 0){
		Queue.insert(Queue.begin(), make_pair(myCV, priority_level));
	}else{
		bool flag = false;
		for(int i = 0; i<Queue.size(); i++){
			if(Queue[i].second > priority_level){
				Queue.insert(Queue.begin()+i, make_pair(myCV, priority_level));
				flag = true;
				break;
			} 
		}
		if(!flag){
			Queue.insert(Queue.end(), make_pair(myCV, priority_level));
		}
	}
	cout<<"Waiting Queue:";	
	if(Queue.size() == 0){
		cout<<"empty"<<endl;
	}else{
		for(int i = 0; i<Queue.size(); i++){
			cout<<Queue[i].second<<'\t';
		}
		cout<<endl;
	}
	cout<<"Size of Executing Queue:\t"<<ExeQueue.size()<<endl;

	while(!meetCond(Queue, ExeQueue, priority_level)){
                pthread_cond_wait(myCV, &lock);
        }

	Queue.erase(Queue.begin());
	ExeQueue.insert(ExeQueue.begin(), make_pair(myCV, priority_level));
/*
	if(Queue.size() > 1){
		nextWaiter = Queue[1].first;
		pthread_cond_signal(nextWaiter);
	}
*/
	pthread_mutex_unlock(&lock);
}


void PriorityLock::exit(){
	pthread_mutex_lock(&lock);
	ExeQueue.erase(ExeQueue.begin());
	if(Queue.size() > 0){
		auto nextWaiter = Queue[0].first;
		pthread_cond_signal(nextWaiter);
	}
	pthread_mutex_unlock(&lock);
}

PriorityLock Lock;

//main for testing PriorityLock
void * ThreadRoutine (void * param)
{	
   int priority_level = *((int*) param);
   Lock.enter(priority_level);  
   std::cout <<"Thread "<< priority_level << " calling enter \n";
   //printf("thread %d calling enter\n", priority_level);

   //sleep for some random amount of time 
   sleep(1);
   Lock.exit();
}

int main()
{
	pthread_t thread[10];

	int priority_level[10];

	for(int j = 0; j < 10; j++){
		priority_level[j] = j;
	}

	for(volatile int i = 0; i< 10; i++){
		pthread_create(&thread[i], NULL, ThreadRoutine, (void*)(&priority_level[i]));
	}
	
	for(int i = 0; i< 10; i++){
		pthread_join(thread[i], NULL);
	}
	return 0;
}
