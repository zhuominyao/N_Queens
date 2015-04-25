#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <pthread.h>

using namespace std;

struct point
{
	int x;
	int y;
};

struct statue
{
	struct point point_1;//第一列的点
	struct point point_2;//第二列的点
};

struct parameter
{
//	struct statue start;//开始状态
//	struct statue end;//结束状态
	vector<struct statue> statues;
};

int COMPUTER_NUMBER = 2;
const int PORT_NUMBER = 10000;
const int SERVER_PORT_NUMBER = 10001;
int server_socket_id;
vector<parameter> p;//要传给各计算机的参数

void search(vector<vector<int> >,int,vector<point>,vector<statue> &);
void * thread_function(void*);

void find_all_statue(vector<statue> & statue_vec,int n)
{
	vector<vector<int> > solution;
	for(int i = 0;i < n;i++)
	{
		vector<int> temp(n,0);
		solution.push_back(temp);
	}
	vector<point> points;
	search(solution,0,points,statue_vec);
}

bool is_safe(vector<vector<int> > solution,int x,int y)//判断能否在(x,y)放置皇后
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

void search(vector<vector<int> > solution,int n,vector<point> points,vector<statue> & statue_vec)
{
    if(n == 2)
    {
		struct statue new_statue;
		new_statue.point_1 = points[0];//此时points里一定有两个point
		new_statue.point_2 = points[1];
		statue_vec.push_back(new_statue);
        return;
    }
    else
    {
        for(int i = 0;i < solution.size();i++)
        {
            if(is_safe(solution,i,n))
            {
                solution[i][n] = 1;
				
				struct point new_point;
				new_point.x = i;
				new_point.y = n;
				points.push_back(new_point);
                
				search(solution,n+1,points,statue_vec);
				
				points.pop_back();
                solution[i][n] = 0;
            }
        }
    }
}

void distribution(int n,int computer_number)//n皇后,computer_number个计算机求解
{
	int statue_sum = (n - 1) * (n - 2);
	int statue_per_computer = statue_sum / computer_number;//每台计算机需要计算多少种statue
	int remainder = statue_sum - statue_per_computer * computer_number;//余数
	
	vector<int> computer_statue_number(computer_number,statue_per_computer);//保存各计算机计算的statue数
	for(int i = 0;i < remainder;i++)
		computer_statue_number[i]++;

	for(int i = 0;i < computer_number;i++)
		cout<<computer_statue_number[i]<<endl;

	vector<statue> statue_vec;
	find_all_statue(statue_vec,n);

	cout<<"there is "<<statue_vec.size()<<" legal statue"<<endl;
	
	for(int i = 0;i < statue_vec.size();i++)
	{
		cout<<"legal statue "<<i+1<<":"<<endl;
		cout<<"point on row 1:("<<statue_vec[i].point_1.x<<","<<statue_vec[i].point_1.y<<")"<<endl;
		cout<<"point on row 2:("<<statue_vec[i].point_2.x<<","<<statue_vec[i].point_2.y<<")"<<endl;
		cout<<endl;
	}
	
	struct statue start;
	struct statue end;
	int index = 0;
	for(int i = 0;i < computer_number;i++)
	{
		start = statue_vec[index];
		cout<<"start:"<<index<<endl;
		int statue_number = computer_statue_number[i];
		while(statue_number > 0)
		{
			p[i].statues.push_back(statue_vec[index]);
			index++;
			statue_number--;
		}
		end = statue_vec[index - 1];
		cout<<"end:"<<index - 1<<endl;

		//p[i].start = start;
		//p[i].end = end;
		cout<<"statues size:"<<p[i].statues.size()<<endl;
	}
}

int main()
{
	int n;
	cout<<"please input how many queens are there?"<<endl;
	cin>>n;
	FILE * fr = fopen("ip.txt","r");
	fscanf(fr,"%d",&COMPUTER_NUMBER);
	for(int i = 0;i < COMPUTER_NUMBER;i++)
	{
		struct parameter par;
		p.push_back(par);
	}

	distribution(n,COMPUTER_NUMBER);
	//读取ip地址
	char ip[COMPUTER_NUMBER][30];
	for(int i = 0;i < COMPUTER_NUMBER;i++)
	{
		fscanf(fr,"%s",ip[i]);
		cout<<"ip"<<i+1<<":"<<ip[i]<<endl;
	}
	for(int i = 0;i < COMPUTER_NUMBER;i++)
		cout<<ip[i]<<endl;
	fclose(fr);

	int sockfd;
	struct sockaddr_in servaddr;

	//获得计算结果申请的socket
	struct sockaddr_in saddr;
	struct ifaddrs * ifAddrStruct = NULL;
	void * tmpAddrPtr = NULL;
	server_socket_id = socket(AF_INET,SOCK_STREAM,0);
	bzero((void*)&saddr,sizeof(saddr));
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
	saddr.sin_port = htons(SERVER_PORT_NUMBER);

	
	
	if(bind(server_socket_id,(struct sockaddr *)&saddr,sizeof(saddr)) != 0)
	{
		cout<<"bind error"<<endl;
		exit(-1);
	}

	if(listen(server_socket_id,COMPUTER_NUMBER) != 0)
	{
		cout<<"listen error"<<endl;
		exit(-1);
	}

	pthread_t tid;
	pthread_create(&tid,NULL,thread_function,NULL);

	for(int i = 0;i < COMPUTER_NUMBER;i++)
	{
		cout<<"i:"<<i<<endl;
		sockfd = socket(AF_INET,SOCK_STREAM,0);
		bzero(&servaddr,sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(PORT_NUMBER);
		//bcopy((void*)hp->h_addr,(void*)&servaddr.sin_addr,hp->h_length);
		servaddr.sin_addr.s_addr = inet_addr(ip[i]);
		//cout<<inet_ntoa(servaddr.sin_addr)<<endl;

		cout<<"address:"<<inet_ntoa(servaddr.sin_addr)<<endl;
		cout<<"port:"<<PORT_NUMBER<<endl;

		if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)
		{
			cout<<"connect error"<<endl;
			exit(-1);
		}
		FILE * fw = fdopen(sockfd,"w");
		fprintf(fw,"%d\n",n);
		fprintf(fw,"%d\n",p[i].statues.size());
		for(int j = 0;j < p[i].statues.size();j++)
		{
			fprintf(fw,"%d %d %d %d\n",p[i].statues[j].point_1.x,p[i].statues[j].point_1.y,p[i].statues[j].point_2.x,p[i].statues[j].point_2.y);
		}
		fclose(fw);
	}

	pthread_join(tid,NULL);
}

void * thread_function(void * p)
{
	int fd;
	int computer_number = COMPUTER_NUMBER;
	long count_array[COMPUTER_NUMBER];

	for(int i = 0;i < COMPUTER_NUMBER;i++)
		count_array[i] = 0;

	for(int i = 0;i < COMPUTER_NUMBER;i++)
	{
		cout<<"thread running"<<endl;
		if((fd = accept(server_socket_id,NULL,NULL)) < 0)
		{
			cout<<"accept error"<<endl;
			exit(-1);
		}
		FILE * fr = fdopen(fd,"r");
		fscanf(fr,"%ld",&count_array[i]);
		fclose(fr);
	}

	long sum = 0;
	for(int i = 0;i < COMPUTER_NUMBER;i++)
	{
		cout<<i+1<<":"<<count_array[i]<<endl;
		sum = sum + count_array[i];
	}

	cout<<"there are "<<sum<<" kinds of different ways"<<endl;
}
