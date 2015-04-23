#include <iostream>
#include <vector>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdio.h>

using namespace std;

#define REPEAT_TIME 1

pthread_mutex_t stdio_mutex;
pthread_mutex_t count_mutex;

long count = 0;

void * thread_function(void *);

struct thread_parameter
{
	int n;
	int start;
	int end;
};

bool is_safe(const vector<vector<int> > & solution,int x,int y)//判断能否在(x,y)放置皇后
{
    int n = solution.size();
    for(int i = 0;i < y;i++)//判断当前位置所在的列有没有皇后
        if(solution[x][i] == 1)
            return false;

    for(int i = x-1,j = y-1;i >= 0 && j >= 0;i--,j--)//判断左上角有没有皇后
        if(solution[i][j] == 1)
            return false;

    for(int i = x+1,j = y-1;i < n && j >= 0;i++,j--)//判断右上角有没有皇后
        if(solution[i][j] == 1)
            return false;

    return true;
}

void solve(vector<vector<int> > & solution,int n)
{
    if(n == solution.size())
    {
		count++;
		/*
        for(int i = 0;i < solution.size();i++)
        {
            for(int j = 0;j < solution.size();j++)
                cout<<solution[i][j]<<" ";
            cout<<endl;
        }
        cout<<endl;
		*/
        return;
    }
    else
    {
        for(int i = 0;i < solution.size();i++)
        {
            if(is_safe(solution,i,n))
            {
                solution[i][n] = 1;
                solve(solution,n+1);
                solution[i][n] = 0;
            }
        }
    }
}

void solve_n_queens(int n)
{
    vector<vector<int> > solution;
    for(int i = 0;i < n;i++)
    {
        vector<int> temp(n,0);
        solution.push_back(temp);
    }
    solve(solution,0);
}

void solve_with_multithread(vector<vector<int> > & solution,int n,int start,int end)
{
    if(n == solution.size())
    {
		pthread_mutex_lock(&count_mutex);
		count++;
		pthread_mutex_unlock(&count_mutex);

		/*
		pthread_mutex_lock(&stdio_mutex);
        for(int i = 0;i < solution.size();i++)
        {
            for(int j = 0;j < solution.size();j++)
                cout<<solution[i][j]<<" ";
            cout<<endl;
        }
        cout<<endl;
		pthread_mutex_unlock(&stdio_mutex);
		*/
        return;
    }
    else
    {
		if(n != 0)
		{
        	for(int i = 0;i < solution.size();i++)
        	{
            	if(is_safe(solution,i,n))//一行一行地放置皇后
            	{
               		solution[i][n] = 1;
                	solve_with_multithread(solution,n+1,start,end);
                	solution[i][n] = 0;//回溯
            	}
        	}
		}
		else//n等于0的时候
		{
			for(int i = start;i <= end;i++)
			{
				if(is_safe(solution,i,n))//一行一行地放置皇后
            	{
               		solution[i][n] = 1;
                	solve_with_multithread(solution,n+1,start,end);
                	solution[i][n] = 0;//回溯
            	}
			}
		}
    }
}

void solve_n_queens_with_multithread(int n,int thread_number)
{
	int n_per_thread = n / thread_number;
	int aliquot = n % thread_number == 0 ? 0 : 1;
	int start = 0;
	int end = 0;
	pthread_t tid_array[thread_number];
	pthread_t tid;
	for(int i = 0;i < thread_number;i++)
	{
		end = start + n_per_thread - 1 + aliquot;//注意aliquot变量的使用
		
		if(end >= n)
			end = n - 1;

		struct thread_parameter * thread_p = (struct thread_parameter *)malloc(sizeof(struct thread_parameter));
		thread_p->n = n;
		thread_p->start = start;
		thread_p->end = end;
		pthread_create(&tid_array[i],NULL,thread_function,thread_p);
		
		/*
		pthread_mutex_lock(&stdio_mutex);
		cout<<"start:"<<start<<endl;
		cout<<"end:"<<end<<endl;
		pthread_mutex_unlock(&stdio_mutex);
		*/

		start = end + 1;
	}
	for(int i = 0;i < thread_number;i++)
		pthread_join(tid_array[i],NULL);
}

void * thread_function(void * p)
{
	struct thread_parameter * thread_p = (struct thread_parameter *)p;
	int start = thread_p->start;
	int end = thread_p->end;
	int n = thread_p->n;
	
	/*
	pthread_mutex_lock(&stdio_mutex);
	cout<<"n:"<<n<<endl;
	cout<<"start:"<<start<<endl;
	cout<<"end:"<<end<<endl;
	pthread_mutex_unlock(&stdio_mutex);
	*/

	vector<vector<int> > solution;
    for(int i = 0;i < n;i++)
    {
        vector<int> temp(n,0);
        solution.push_back(temp);
    }
	free(thread_p);
    solve_with_multithread(solution,0,start,end);
}

int main()
{
	int n,thread_number;
	struct timeval start,finish;
	float single_thread_timeuse;
	float multi_thread_timeuse;
	pthread_mutex_init(&stdio_mutex,NULL);
	pthread_mutex_init(&stdio_mutex,NULL);
    cout<<"please input how many queens are there?"<<endl;
    cin>>n;
	cout<<"please input how many threads to create?(at most 8)"<<endl;
	cin>>thread_number;
	cout<<endl;

	gettimeofday(&start,NULL);
	for(int i = 0;i < REPEAT_TIME;i++)
    	solve_n_queens(n);
	gettimeofday(&finish,NULL);
	single_thread_timeuse = 1000000*(finish.tv_sec - start.tv_sec) + (finish.tv_usec - start.tv_usec);
	cout<<"the average time used to solve "<<n<<" queens problem with single thread is "<<(single_thread_timeuse/1000000/REPEAT_TIME)<<" seconds"<<endl;
	cout<<endl;

	gettimeofday(&start,NULL);
	for(int i = 0;i < REPEAT_TIME;i++)
    	solve_n_queens_with_multithread(n,thread_number);
	gettimeofday(&finish,NULL);
	multi_thread_timeuse = 1000000*(finish.tv_sec - start.tv_sec) + (finish.tv_usec - start.tv_usec);
	cout<<"the average time used to slove "<<n<<" queens problem with "<<thread_number<<" threads is "<<(multi_thread_timeuse/1000000/REPEAT_TIME)<<" seconds"<<endl;
	cout<<endl;
	cout<<"there are "<<count/REPEAT_TIME/2<<" ways"<<endl;
	cout<<endl;

	cout<<"the speed up rate is :"<<single_thread_timeuse/multi_thread_timeuse<<endl;
    return 0;
}

