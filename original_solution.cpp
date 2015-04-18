#include <iostream>
#include <vector>
#include <sys/time.h>

using namespace std;

int count = 0;

bool is_safe(vector<vector<int> > solution,int x,int y)
{
    int n = solution.size();
    for(int i = 0;i < y;i++)
        if(solution[x][i] == 1)
            return false;

    for(int i = x-1,j = y-1;i >= 0 && j >= 0;i--,j--)
        if(solution[i][j] == 1)
            return false;

    for(int i = x+1,j = y-1;i < n && j >= 0;i++,j--)
        if(solution[i][j] == 1)
            return false;

    return true;
}

void solve(vector<vector<int> > solution,int n)
{
    if(n == solution.size())
    {
		count++;
        for(int i = 0;i < solution.size();i++)
        {
            for(int j = 0;j < solution.size();j++)
                cout<<solution[i][j]<<" ";
            cout<<endl;
        }
        cout<<endl;
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

int main()
{
    cout << "please input how many queens are there?" << endl;
    int n;
	struct timeval start,finish;
	float timeuse;
    cin>>n;

	gettimeofday(&start,NULL);
    solve_n_queens(n);
	gettimeofday(&finish,NULL);
	timeuse = 1000000*(finish.tv_sec - start.tv_sec) + (finish.tv_usec - start.tv_usec);
	cout<<(timeuse/1000000)<<" seconds"<<endl;

	cout<<"there is "<<count<<" ways"<<endl;
    return 0;
}

