#include <iostream>
#include <vector>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <netinet/in.h>

using namespace std;

const int PORT_NUMBER = 10000;
const int SERVER_PORT_NUMBER = 10001;
const int MAX_LENGTH = 512;
const int THREAD_NUMBER = 4;

long count = 0;
int queen_number = 8;
int fd;
struct sockaddr_in addr;
socklen_t len;

pthread_mutex_t stdio_mutex;
pthread_mutex_t count_mutex;

struct point
{
	int x;
	int y;
};

struct statue
{
	struct point point_1;
	struct point point_2;
};

struct parameter
{
	vector<statue> statues;
};

struct thread_parameter
{
	vector<statue> statues;
};

void get_p(FILE * fr,struct parameter & p);
void solve_n_queens_with_multithread();
void * thread_function(void *);
bool is_safe(const vector<vector<int> > &,int,int);
void solve(vector<vector<int> > &,int);

struct parameter p;

int main()
{
	struct sockaddr_in saddr;
	struct hostent * hp;
	char hostname[MAX_LENGTH];
	int server_socket_id;
	FILE * fr;

	server_socket_id = socket(AF_INET,SOCK_STREAM,0);
	if(server_socket_id == -1)
	{
		cout<<"socket error"<<endl;
		exit(-1);
	}

	bzero((void*)&saddr,sizeof(saddr));
	gethostname(hostname,MAX_LENGTH);	
	hp = gethostbyname(hostname);

	//bcopy((void*)hp->h_addr_list[0],(void*)&saddr.sin_addr,hp->h_length);
	struct ifaddrs * ifAddrStruct = NULL;
	void * tmpAddrPtr = NULL;

	getifaddrs(&ifAddrStruct);
	while(ifAddrStruct != NULL)
	{
		if(ifAddrStruct->ifa_addr->sa_family == AF_INET)
		{
			tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			//得到外网的ip
			if(strcmp(ifAddrStruct->ifa_name,"eth1") == 0)
			{
				//cout<<"ipaddress:"<<addressBuffer<<endl;
				saddr.sin_addr.s_addr = inet_addr(addressBuffer);
				break;
			}
		}
		ifAddrStruct=ifAddrStruct->ifa_next;
	}
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORT_NUMBER);

	cout<<"hostname:"<<hostname<<endl;
	cout<<"address:"<<inet_ntoa(saddr.sin_addr)<<endl;
	cout<<"port:"<<PORT_NUMBER<<endl;

	if(bind(server_socket_id,(struct sockaddr *)&saddr,sizeof(saddr)) != 0)
	{
		cout<<"bind error"<<endl;
		exit(-1);
	}

	if(listen(server_socket_id,10) != 0)
	{
		cout<<"listen error"<<endl;
		exit(-1);
	}

	cout<<"listenning..."<<endl;

	if((fd = accept(server_socket_id,NULL,NULL)) < 0)
	{
		cout<<"accept error"<<endl;
		exit(-1);
	}
	
	getpeername(fd,(struct sockaddr *)&addr,&len);
	cout<<"accept connection"<<endl;

	fr = fdopen(fd,"r");
	get_p(fr,p);

	struct timeval start,end;
	double multi_thread_timeuse;

	gettimeofday(&start,NULL);
	solve_n_queens_with_multithread();
	gettimeofday(&end,NULL);
	multi_thread_timeuse = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	cout<<"time use:"<<multi_thread_timeuse/1000000<<endl;
}

void solve_n_queens_with_multithread()
{
	int statue_per_thread = p.statues.size() / THREAD_NUMBER;
	int remainder = p.statues.size() - statue_per_thread * THREAD_NUMBER;
	vector<int> thread_statue_number(THREAD_NUMBER,statue_per_thread);
	for(int i = 0;i < remainder;i++)
		thread_statue_number[i]++;

	int index = 0;
	pthread_t tid_array[THREAD_NUMBER];
	for(int i = 0;i < THREAD_NUMBER;i++)
	{
		struct thread_parameter * thread_p = new thread_parameter();
		for(int j = 0;j < thread_statue_number[i];j++)
		{
			thread_p->statues.push_back(p.statues[index]);
			index++;
		}
		pthread_create(&tid_array[i],NULL,thread_function,thread_p);
	}
	
	for(int i = 0;i < THREAD_NUMBER;i++)
		pthread_join(tid_array[i],NULL);

	cout<<"count:"<<count<<endl;
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT_NUMBER);
	cout<<"count send back address:"<<inet_ntoa(addr.sin_addr)<<endl;
	cout<<"count send back port:"<<SERVER_PORT_NUMBER<<endl;

	if(connect(sockfd,(struct sockaddr *)&addr,sizeof(addr)) != 0)
	{
		cout<<"connect error"<<endl;
		exit(-1);
	}
	FILE * fw = fdopen(sockfd,"w");
	fprintf(fw,"%ld\n",count);
	fflush(fw);
	fclose(fw);
}

void * thread_function(void * p)
{
	struct thread_parameter * thread_p = (struct thread_parameter *)p;

	for(int i = 0;i < thread_p->statues.size();i++)
	{
		vector<vector<int> > solution;
		for(int j = 0;j < queen_number;j++)
		{
			vector<int> temp(queen_number,0);
			solution.push_back(temp);
		}
		solution[thread_p->statues[i].point_1.y][thread_p->statues[i].point_1.x] = 1;
		solution[thread_p->statues[i].point_2.y][thread_p->statues[i].point_2.x] = 1;
		
		solve(solution,2);
	}
	free(thread_p);
}

void solve(vector<vector<int> > & solution,int n)
{
	if(n == solution.size())
	{
		/*
		pthread_mutex_lock(&stdio_mutex);
		for(int j = 0;j < queen_number;j++)
		{
			for(int k = 0;k < queen_number;k++)
				cout<<solution[j][k];
			cout<<endl;
		}
		cout<<endl;
		*/

		pthread_mutex_unlock(&stdio_mutex);

		pthread_mutex_lock(&count_mutex);
		count++;
		pthread_mutex_unlock(&count_mutex);
		return;
	}
	else
	{
		for(int i = 0;i < solution.size();i++)
		{
			if(is_safe(solution,i,n))
			{
				solution[n][i] = 1;
				solve(solution,n+1);
				solution[n][i] = 0;
			}
		}
	}
}

bool is_safe(const vector<vector<int> > & solution,int x,int y)//判断能否在(x,y)放置皇后
{
    int n = solution.size();
    for(int i = 0;i < y;i++)//判断当前位置所在的列有没有皇后
        if(solution[i][x] == 1)
            return false;

    for(int i = x-1,j = y-1;i >= 0 && j >= 0;i--,j--)//判断左上角有没有皇后
        if(solution[j][i] == 1)
            return false;

    for(int i = x+1,j = y-1;i < n && j >= 0;i++,j--)//判断右上角有没有皇后
        if(solution[j][i] == 1)
            return false;

    return true;
}

void get_p(FILE * fr,struct parameter & p)
{
	int n;
	fscanf(fr,"%d",&queen_number);
	fscanf(fr,"%d",&n);

	for(int i = 0;i < n;i++)
	{
		struct statue s;
		fscanf(fr,"%d%d%d%d",&s.point_1.x,&s.point_1.y,&s.point_2.x,&s.point_2.y);
		p.statues.push_back(s);
	}
}
