#include <iostream>
#include <vector>

using namespace std;

int count = 0;

bool is_safe(vector<vector<int> > solution,int x,int y)//判断能否在(x,y)放置皇后
{
    int n = solution.size();
    for(int i = 0;i < y;i++)//判断当前位置所在的列有没有皇后
        if(solution[x][i] == 1)
            return false;

    for(int i = x-1,j = y-1;i >= 0 && j >= 0;i--,j--)//判断左上角有没有皇后
        if(solution[i][j] == 1)
            return false;

    for(int i = x+1,j = y-1;i < n && j >= 0;i++,j--)//判断左下角有没有皇后
        if(solution[i][j] == 1)
            return false;

    return true;
}

void solve(vector<vector<int> > solution,int n)
{
    if(n == 2)
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


int main()
{
	for(int i = 8;i < 21;i++)
	{
		solve_n_queens(i);
		cout<<i<<":"<<count<<endl;
		count = 0;
	}
}
