
//
// Disclaimer:
// ----------
//
// This code will work only if you selected window, graphics and audio.
//
// Note that the "Run Script" build phase will copy the required frameworks
// or dylibs to your application bundle so you can execute it on any OS X
// computer.
//
// Your resource files (images, sounds, fonts, ...) are also copied to your
// application bundle. To get the path to these resources, use the helper
// function `resourcePath()` from ResourcePath.hpp
//

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <random>
#include <vector>
#include <iostream>
#include <math.h>

#include "ResourcePath.hpp"

float height=2400;
float width=1300;
float view_height=1100;
float view_width=1000;
int size=40;
int center=size/2;
float x_0=80;
float y_0=150;
int fire_time=0;
int burning_cost=10;
int water_cost=15;
int dead_cost=40;

double prob=0.15;
using namespace std;
using namespace sf;

std::mt19937 gen32 (time(0));
std::uniform_real_distribution<double> dis(0.0, 1.0);

enum States {Empty,Alive,Burning,Dead,Water};

class Cell {
public:
    RectangleShape image;
    States state;
    bool isWater;
    short pos_x;
    short pos_y;
    Cell* operator = (Cell* c2)
    {
        this->state=c2->state;
        this->image=c2->image;
        this->pos_x=c2->pos_x;
        this->pos_y=c2->pos_y;
        return this;
    }
};

class Cell_light {
public:
    States state;
    short pos_x;
    short pos_y;
    Cell_light* operator = (Cell_light* c2)
    {
        this->state=c2->state;
        this->pos_x=c2->pos_x;
        this->pos_y=c2->pos_y;
        return this;
    }
};

vector<Cell*> burning;
vector<Cell*> dying;
vector<Cell*> extinguish;
vector<Cell*> recovering;

int sgn(double x)
{
    if(x>0)
        return 1;
    else
        return 0;
}

double burning_prob(int num, int water)
{
    if(num==0)
    {
        return 0.000001;
    }
    double total=0.35*sgn(num)+0.2*(num-1)-0.3*water;
    return total;
}

double ext_prob(int water, int num)
{
    if(water==0)
    {
        return 0;
    }
    else
    {
        //double total=0.3-
        double total = 0.8-0.2*num;
        return total;
    }
}

double dead_prob(int water, int num)
{
    //double total=0.05-0.06*num;
    double total=0.2-0.05*num;
    if(total<0)
    {
        return 0.002;
    }
    else
    {
        return total;
    }
}

double recovery_prob(int alive, int burn)
{
    double total;
    if(burn>0)
        return 0;
    else
        total=0.0002+0.01*alive;
    return total;
}

//Generating all possible states
vector<Cell_light**> generate_states(Cell** &arr, int size)
{
    vector<Cell_light**> base;
    vector<Cell_light**> all_states;
    Cell_light** temp;
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            if(base.empty())
            {
                Cell_light** temp2 = new Cell_light*[size];
                for(int q=0;q<size;q++)
                {
                    temp2[q]=new Cell_light[size];
                }
                base.push_back(temp2);
            }
            if(arr[i][j].state==Empty)
            {
                for(Cell_light** count:base)
                {
                    count[i][j].state=Empty;
                    count[i][j].pos_x=i;
                    count[i][j].pos_y=j;
                }
            }
            else
            {
                while(!base.empty())
                {
                    temp=base.back();
                    for(int w=0;w<3;w++)
                    {
                        Cell_light** temp2 = new Cell_light*[size];
                        for(int q=0;q<size;q++)
                        {
                            temp2[q]=new Cell_light[size];
                        }
                        for(int p=0;p<size;p++)
                        {
                            for(int o=0;o<size;o++)
                            {
                                temp2[p][o]=temp[p][o];
                                temp2[p][o].pos_x=p;
                                temp2[p][o].pos_y=o;
                            }
                        }
                        switch (w) {
                            case 0:
                                temp2[i][j].state=Alive;
                                break;
                            case 1:
                                temp2[i][j].state=Burning;
                                break;
                            case 2:
                                temp2[i][j].state=Dead;
                                break;
                            default:
                                break;
                        }
                        all_states.push_back(temp2);
                    }
                    delete [] temp;
                    base.pop_back();
                }
                base=all_states;
                all_states.clear();
            }
        }
    }
    bool flag=true;
    vector<Cell_light**>::iterator it1;
    it1=base.begin();
    for(int c=0;c<base.size();c++)
    {
        flag=true;
        for(int i=0;i<size;i++)
        {
            for(int j=0;j<size;j++)
            {
                if(base[c][i][j].state==Burning)
                {
                    flag=false;
                    i=size;
                    j=size;
                }
            }
        }
        if(!flag)
        {
            all_states.push_back(base[c]);
            /*for(int i=0;i<size;i++)
            {
                for(int j=0;j<size;j++)
                {
                    switch (all_states[c][i][j].state) {
                        case 0:
                            cout<<"Empty"<<"\t";
                            break;
                        case 1:
                            cout<<"Alive"<<"\t";
                            break;
                        case 2:
                            cout<<"Burn"<<"\t";
                            break;
                        case 3:
                            cout<<"Dead"<<"\t";
                            break;
                        default:
                            break;
                    }
                }
                cout<<"\n";
            }
            cout<<"\n";*/
        }
    }
    return all_states;
}

vector<Cell_light**> gen_local_states(Cell** &arr, int size, int x, int y)
{
    if(arr[x][y].state==Empty)
    {
        vector<Cell_light**> base;
        Cell_light** temp = new Cell_light*[3];
        for(int i=0;i<3;i++)
        {
            temp[i]=new Cell_light[3];
            for(int j=0;j<3;j++)
            {
                temp[i][j].state=Empty;
            }
        }
        base.push_back(temp);
       // delete [] temp;
        return base;
    }
    vector<Cell_light**> base;
    vector<Cell_light**> all_states;
    Cell_light** temp = new Cell_light*[3];
    for(int i=0;i<3;i++)
    {
        temp[i]=new Cell_light[3];
    }
    temp[0][0].state=Empty;
    temp[2][2].state=Empty;
    temp[0][2].state=Empty;
    temp[2][0].state=Empty;
    base.push_back(temp);
    temp=base.back();
    for(int w=0;w<3;w++)
    {
        Cell_light** temp2 = new Cell_light*[3];
        for(int q=0;q<3;q++)
        {
            temp2[q]=new Cell_light[3];
        }
        for(int p=0;p<3;p++)
        {
            for(int o=0;o<3;o++)
            {
                temp2[p][o]=temp[p][o];
                temp2[p][o].pos_x=p;
                temp2[p][o].pos_y=o;
            }
        }
        switch (w) {
            case 0:
                temp2[1][1].state=Alive;
                break;
            case 1:
                temp2[1][1].state=Burning;
                break;
            case 2:
                temp2[1][1].state=Dead;
                break;
            default:
                break;
        }
        all_states.push_back(temp2);
    }
    delete [] temp;
    base.pop_back();
    base=all_states;
    all_states.clear();
    if(x-1>=0)
    {
        if(arr[x-1][y].state==Empty)
        {
            for(Cell_light** cur : base)
            {
                cur[0][1].state=Empty;
            }
        }
        else
        {
            while(!base.empty())
            {
                temp=base.back();
                for(int w=0;w<3;w++)
                {
                    Cell_light** temp2 = new Cell_light*[3];
                    for(int q=0;q<3;q++)
                    {
                        temp2[q]=new Cell_light[3];
                    }
                    for(int p=0;p<3;p++)
                    {
                        for(int o=0;o<3;o++)
                        {
                            temp2[p][o]=temp[p][o];
                            temp2[p][o].pos_x=p;
                            temp2[p][o].pos_y=o;
                        }
                    }
                    switch (w) {
                        case 0:
                            temp2[0][1].state=Alive;
                            break;
                        case 1:
                            temp2[0][1].state=Burning;
                            break;
                        case 2:
                            temp2[0][1].state=Dead;
                            break;
                        default:
                            break;
                    }
                    all_states.push_back(temp2);
                }
                delete [] temp;
                base.pop_back();
            }
            base=all_states;
            all_states.clear();
        }
    }
    else
    {
        for(Cell_light** cur : base)
        {
            cur[0][1].state=Empty;
        }
    }
    if(x+1<size)
    {
        if(arr[x+1][y].state==Empty)
        {
            for(Cell_light** cur : base)
            {
                cur[2][1].state=Empty;
            }
        }
        else
        {
            while(!base.empty())
            {
                temp=base.back();
                for(int w=0;w<3;w++)
                {
                    Cell_light** temp2 = new Cell_light*[3];
                    for(int q=0;q<3;q++)
                    {
                        temp2[q]=new Cell_light[3];
                    }
                    for(int p=0;p<3;p++)
                    {
                        for(int o=0;o<3;o++)
                        {
                            temp2[p][o]=temp[p][o];
                            temp2[p][o].pos_x=p;
                            temp2[p][o].pos_y=o;
                        }
                    }
                    switch (w) {
                        case 0:
                            temp2[2][1].state=Alive;
                            break;
                        case 1:
                            temp2[2][1].state=Burning;
                            break;
                        case 2:
                            temp2[2][1].state=Dead;
                            break;
                        default:
                            break;
                    }
                    all_states.push_back(temp2);
                }
                delete [] temp;
                base.pop_back();
            }
            base=all_states;
            all_states.clear();
        }
    }
    else
    {
        for(Cell_light** cur : base)
        {
            cur[2][1].state=Empty;
        }
    }
    if(y-1>=0)
    {
        if(arr[x][y-1].state==Empty)
        {
            for(Cell_light** cur : base)
            {
                cur[1][0].state=Empty;
            }
        }
        else
        {
            while(!base.empty())
            {
                temp=base.back();
                for(int w=0;w<3;w++)
                {
                    Cell_light** temp2 = new Cell_light*[3];
                    for(int q=0;q<3;q++)
                    {
                        temp2[q]=new Cell_light[3];
                    }
                    for(int p=0;p<3;p++)
                    {
                        for(int o=0;o<3;o++)
                        {
                            temp2[p][o]=temp[p][o];
                            temp2[p][o].pos_x=p;
                            temp2[p][o].pos_y=o;
                        }
                    }
                    switch (w) {
                        case 0:
                            temp2[1][0].state=Alive;
                            break;
                        case 1:
                            temp2[1][0].state=Burning;
                            break;
                        case 2:
                            temp2[1][0].state=Dead;
                            break;
                        default:
                            break;
                    }
                    all_states.push_back(temp2);
                }
                delete [] temp;
                base.pop_back();
            }
            base=all_states;
            all_states.clear();
        }
    }
    else
    {
        for(Cell_light** cur : base)
        {
            cur[1][0].state=Empty;
        }
    }
    if(y+1<size)
    {
        if(arr[x][y+1].state==Empty)
        {
            for(Cell_light** cur : base)
            {
                cur[1][2].state=Empty;
            }
        }
        else
        {
            while(!base.empty())
            {
                temp=base.back();
                for(int w=0;w<3;w++)
                {
                    Cell_light** temp2 = new Cell_light*[3];
                    for(int q=0;q<3;q++)
                    {
                        temp2[q]=new Cell_light[3];
                    }
                    for(int p=0;p<3;p++)
                    {
                        for(int o=0;o<3;o++)
                        {
                            temp2[p][o]=temp[p][o];
                            temp2[p][o].pos_x=p;
                            temp2[p][o].pos_y=o;
                        }
                    }
                    switch (w) {
                        case 0:
                            temp2[1][2].state=Alive;
                            break;
                        case 1:
                            temp2[1][2].state=Burning;
                            break;
                        case 2:
                            temp2[1][2].state=Dead;
                            break;
                        default:
                            break;
                    }
                    all_states.push_back(temp2);
                }
                delete [] temp;
                base.pop_back();
            }
            base=all_states;
            all_states.clear();
        }
    }
    else
    {
        for(Cell_light** cur : base)
        {
            cur[1][2].state=Empty;
        }
    }
    
    bool flag=true;
    vector<Cell_light**>::iterator it1;
    it1=base.begin();
    for(int c=0;c<base.size();c++)
    {
        flag=true;
        for(int i=0;i<3;i++)
        {
            for(int j=0;j<3;j++)
            {
                if(base[c][i][j].state==Burning)
                {
                    flag=false;
                    i=size;
                    j=size;
                }
            }
        }
        if(!flag)
        {
            all_states.push_back(base[c]);
            /*for(int i=0;i<size;i++)
            {
                for(int j=0;j<size;j++)
                {
                    switch (all_states[c][i][j].state) {
                        case 0:
                            cout<<"Empty"<<"\t";
                            break;
                        case 1:
                            cout<<"Alive"<<"\t";
                            break;
                        case 2:
                            cout<<"Burn"<<"\t";
                            break;
                        case 3:
                            cout<<"Dead"<<"\t";
                            break;
                        default:
                            break;
                    }
                }
                cout<<"\n";
            }
            cout<<"\n";*/
        }
    }
    return all_states;
}

// Generating all possible strategies
vector<short**> generate_strats(Cell** &arr)
{
    vector<short**> strats;
    vector<short**> new_strats;
    short** temp;
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            if(strats.empty())
            {
                short** temp2 = new short*[size];
                for(int k=0;k<size;k++)
                {
                    temp2[k] = new short[size];
                }
                strats.push_back(temp2);
            }
            if(arr[i][j].state==Empty)
            {
                for(short** count:strats)
                {
                    count[i][j]=0;
                }
            }
            else
            {
                while(!strats.empty())
                {
                    temp=strats.back();
                    for(int w=0;w<2;w++)
                    {
                        short** temp2 = new short*[size];
                        for(int q=0;q<size;q++)
                        {
                            temp2[q]=new short[size];
                        }
                        for(int p=0;p<size;p++)
                        {
                            for(int o=0;o<size;o++)
                            {
                                temp2[p][o]=temp[p][o];
                            }
                        }
                        switch (w) {
                            case 0:
                                temp2[i][j]=0;
                                break;
                            case 1:
                                temp2[i][j]=1;
                                break;
                            default:
                                break;
                        }
                        new_strats.push_back(temp2);
                    }
                    delete [] temp;
                    strats.pop_back();
                }
                strats=new_strats;
                new_strats.clear();
            }
        }
    }
    return strats;
}

vector<short**> gen_local_strats()
{
    int size=3;
    vector<short**> strats;
    for(int i=0;i<2;i++)
    {
        short** temp = new short*[size];
        for(int k=0;k<size;k++)
        {
            temp[k] = new short[size];
            for(int l=0;l<size;l++)
            {
                temp[k][l]=0;
            }
        }
        if(i==0)
        {
            temp[1][1]=0;
        }
        else
        {
            temp[1][1]=1;
        }
        strats.push_back(temp);
        //delete [] temp;
    }
    return strats;
}

// Loss function for each cell
double loss_func(int strategy, States state, int alive, int burning)
{
    /*int total = 5*strategy;
    switch (state) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            total+=15;
            break;
        case 3:
            total+=3;
            break;
        default:
            break;
    }
    return total;*/
    switch (state) {
        case Alive:
            return strategy*water_cost+burning_prob(burning, strategy)*burning_cost;//+(1-burning_prob(burning, strategy))*strategy*water_cost;
            break;
        case Burning:
            return strategy*water_cost+dead_prob(strategy, burning+alive)*dead_cost+(1-(dead_prob(strategy, burning+alive)+ext_prob(strategy, burning)))*burning_cost/2.0;
            break;
        case Dead:
            return strategy*water_cost+(1-recovery_prob(alive, burning))*(dead_cost/2.0);
            break;
        case Empty:
            return 0;
            break;
        default:
            break;
    }
}

// Calculating losses for every state
vector<double> losses(vector<Cell_light**> &states, vector<short**> &strategies)
{
    int size=3;
    vector<double> lost;
    int count_b;
    int count_a;
    double loss=0;
    for (int c=0;c<states.size();c++)
    {
        for(int i=0;i<size;i++)
        {
            for(int j=0;j<size;j++)
            {
                if(states[c][i][j].state!=Empty)
                {
                    count_b=0;
                    count_a=0;
                    if(i-1>=0)
                    {
                        if(states[c][i-1][j].state==Burning)
                        {
                            count_b++;
                        }
                        if(states[c][i-1][j].state==Alive)
                        {
                            count_a++;
                        }
                    }
                    if(i+1<size)
                    {
                        if(states[c][i+1][j].state==Burning)
                        {
                            count_b++;
                        }
                        if(states[c][i+1][j].state==Alive)
                        {
                            count_a++;
                        }
                    }
                    if(j-1>=0)
                    {
                        if(states[c][i][j-1].state==Burning)
                        {
                            count_b++;
                        }
                        if(states[c][i][j-1].state==Alive)
                        {
                            count_a++;
                        }
                    }
                    if(j+1<size)
                    {
                        if(states[c][i][j+1].state==Burning)
                        {
                            count_b++;
                        }
                        if(states[c][i][j+1].state==Alive)
                        {
                            count_a++;
                        }
                    }
                    loss+=loss_func(strategies[c][i][j], states[c][i][j].state, count_a, count_b);
                }
            }
        }
        lost.push_back(loss);
        loss=0;
    }
    return lost;
}

// Calculating ergodic probabilities
double* ergodic_prob(vector<Cell_light**> &temp, vector<short**> &strategies)
{
    int size=3;
    double* pi = new double[temp.size()];
    double total=1;
    double sum=0;
    int count_b=0;
    int count_a=0;
    for(int c=0;c<temp.size();c++)
    {
        total=1;
        for(int i=0;i<size;i++)
        {
            for(int j=0;j<size;j++)
            {
                count_b=0;    //counter for around burning cells
                count_a=0;    //counter for around alive cells
                if(i-1>=0)
                {
                    if(temp[c][i-1][j].state==Burning)
                    {
                        count_b++;
                    }
                    if(temp[c][i-1][j].state==Alive)
                    {
                        count_a++;
                    }
                }
                if(i+1<size)
                {
                    if(temp[c][i+1][j].state==Burning)
                    {
                        count_b++;
                    }
                    if(temp[c][i+1][j].state==Alive)
                    {
                        count_a++;
                    }
                }
                if(j-1>=0)
                {
                    if(temp[c][i][j-1].state==Burning)
                    {
                        count_b++;
                    }
                    if(temp[c][i][j-1].state==Alive)
                    {
                        count_a++;
                    }
                }
                if(j+1<size)
                {
                    if(temp[c][i][j+1].state==Burning)
                    {
                        count_b++;
                    }
                    if(temp[c][i][j+1].state==Alive)
                    {
                        count_a++;
                    }
                }
                if(temp[c][i][j].state==Alive)
                {
                        total=total*(burning_prob(count_b, strategies[c][i][j]))/(1-burning_prob(count_b, strategies[c][i][j]));
                }
                if(temp[c][i][j].state==Burning)
                {
                    total=total*(dead_prob(strategies[c][i][j], count_b+count_a)+ext_prob(strategies[c][i][j], count_b))/(1-(dead_prob(strategies[c][i][j], count_b+count_a)+ext_prob(strategies[c][i][j], count_b)));
                }
                if(temp[c][i][j].state==Dead)
                {
                    if(count_b==0)
                    {
                        total=total*recovery_prob(count_a, count_b)/(1-recovery_prob(count_a, count_b));
                    }
                }
            }
        }
        pi[c]=total;
        sum=sum+total;
    }
    for(int i=0;i<temp.size();i++)
    {
        pi[i]=pi[i]/sum;
    }
    return pi;
}

// Calculating expected loss
double exp_loss(double* &prob, vector<double> &losses)
{
    double total=0;
    for(int i=0;i<losses.size();i++)
    {
        total+=prob[i]*losses[i];
    }
    return total;
}

// Calculating transition probability for two strats and strategy
double transition(Cell_light** &first, Cell_light** &second, short** &strategy)
{
    int size=3;
    int count;
    int count_a;
    double temp=1;
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            if(first[i][j].state!=Empty)
            {
                count=0;    //counter for around burning cells
                count_a=0;  //counter for around alive cells
                if(i-1>=0)
                {
                    if(first[i-1][j].state==Burning)
                    {
                        count++;
                    }
                    if(first[i-1][j].state==Alive)
                    {
                        count_a++;
                    }
                }
                if(i+1<size)
                {
                    if(first[i+1][j].state==Burning)
                    {
                        count++;
                    }
                    if(first[i+1][j].state==Alive)
                    {
                        count_a++;
                    }
                }
                if(j-1>=0)
                {
                    if(first[i][j-1].state==Burning)
                    {
                        count++;
                    }
                    if(first[i][j-1].state==Alive)
                    {
                        count_a++;
                    }
                }
                if(j+1<size)
                {
                    if(first[i][j+1].state==Burning)
                    {
                        count++;
                    }
                    if(first[i][j+1].state==Alive)
                    {
                        count_a++;
                    }
                }
                if(first[i][j].state==second[i][j].state)       //same state
                {
                switch (first[i][j].state) {
                    case 1: //if alive
                        temp=temp*(1-burning_prob(count, strategy[i][j]));
                        break;
                    case 2: //if burning
                        temp=temp*(1-(dead_prob(strategy[i][j], count+count_a)+ext_prob(strategy[i][j], count)));
                        break;
                    case 3: //if dead
                        temp=temp*(1-recovery_prob(count_a, count));
                        break;
                    default:
                        break;
                    }
                }
                else        //state changes
                {
                    switch (first[i][j].state) {
                        case 1: //if alive
                        {
                            switch (second[i][j].state) {
                                case 2:     //alive to burning
                                    temp=temp*burning_prob(count, strategy[i][j]);
                                    break;
                                case 3:     // alive to dead is impossible
                                    temp=0;
                                    j=size;
                                    i=size;
                                    break;
                                    
                                default:
                                    break;
                            }
                        }
                            break;
                        case 2: //if burning
                        {
                            switch (second[i][j].state) {
                                case 1:     //burning to alive
                                    temp=temp*ext_prob(strategy[i][j], count);
                                    break;
                                case 3:     //burning to dead
                                    temp=temp*dead_prob(strategy[i][j], count+count_a);
                                    break;
                                default:
                                    break;
                            }
                        }
                            break;
                        case 3: //if dead
                        {
                            switch (second[i][j].state) {
                                case 1:     //dead to alive
                                    temp=temp*recovery_prob(count_a, count);
                                    break;
                                case 2:     //dead to burning - impossible
                                    temp=0;
                                    i=size;
                                    j=size;
                                    break;
                                default:
                                    break;
                            }
                        }
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    return temp;
}

// Generating transition cores
double** core_gen(vector<Cell_light**> &states, vector<short**> &strategies)
{
    double** cores = new double*[states.size()];
    for(int i=0;i<states.size();i++)
    {
        cores[i]=new double[states.size()];
    }
    Cell_light** first;
    Cell_light** second;
    for(int i=0;i<states.size();i++)        //loop for all possible generated states
    {
        for(int j=0;j<states.size();j++)
        {
            first=states[i];
            second=states[j];
            cores[i][j]=transition(first, second, strategies[i]);   //calculating transition probability
        }
    }
    return cores;
}

double* Gauss(double** &system, int size)
{
    /*
    for(int k=0;k<=size;k++)
    {
        for(int l=0;l<=size;l++)
        {
            cout<<system[k][l]<<"\t";
        }
        cout<<"\n";
    }
    cout<<"\n";
     */
    double max=-__DBL_MAX__;
    int index=0;
    double swap=0;
    double coef;
    for(int i=0;i<size;i++)     //for each diagonal element
    {
        swap = 0;
        index=0;
        max=-__DBL_MAX__;
        for(int j=i;j<=size;j++)    // finding max coefficient in column
        {
            if(abs(system[j][i])>max)
            {
                max=abs(system[j][i]);
                index=j;
                
            }
        }
        for(int z=i;z<=size;z++)    // swapping lines with the line with max coefficient
        {
            swap=system[i][z];
            system[i][z]=system[index][z];
            system[index][z]=swap;
        }
        if(system[i][i]!=1)        // normalizing  line
        {
            coef=system[i][i];
            for(int j=i;j<=size;j++)
            {
                system[i][j]=system[i][j]/coef;
            }
        }
        for(int j=0;j<=size;j++)    // making zeros in column
        {
            if((system[j][i]!=0)&&(i!=j))
            {
                coef=system[j][i];
                for(int z=i;z<=size;z++)
                {
                    system[j][z]=system[j][z]-coef*system[i][z];
                }
            }
        }
        /*
        for(int k=0;k<=size;k++)
        {
            for(int l=0;l<=size;l++)
            {
                cout<<system[k][l]<<"\t";
            }
            cout<<"\n";
        }
        cout<<"\n";
         */
    }
    double* result = new double[size];
    for(int i=0;i<size;i++)
    {
        result[i]=system[i][size];
    }
    return result;
}

double* ergodic_prob2(double** &cores, int num)
{
    double** result = new double*[num+1];
    for(int i=0;i<=num;i++)
    {
        result[i]=new double[num+1];
    }
    for(int i=0;i<num;i++)
    {
        for(int j=0;j<num;j++)
        {
            if(i==j)
            {
                result[i][j]=cores[i][j]-1;
            }
            else
            {
                result[i][j]=cores[j][i];
            }
        }
    }
    for(int i=0;i<num;i++)
    {
        result[num][i]=1;
        result[i][num]=0;
    }
    result[num][num]=1;
    double* ergodic = new double[num];
    ergodic=Gauss(result, num);
    return ergodic;
}

int find_state(Cell** &arr, vector<Cell_light**> &states)
{
    bool flag = true;
    int found=0;
    for(int i=0;i<states.size();i++)
    {
        flag=true;
        for(int k=0;k<size;k++)
        {
            for(int l=0;l<size;l++)
            {
                if(arr[k][l].state!=states[i][k][l].state)
                {
                    flag=false;
                    k=size;
                    l=size;
                }
            }
        }
        if(flag)
        {
            found=i;
            i=states.size();
        }
    }
    if(!flag)
    {
        return -1;
    }
    else
    {
        return found;
    }
}

int find_local_state(Cell** &arr, vector<Cell_light**> &states)
{
    int size=3;
    bool flag = true;
    int found=0;
    for(int i=0;i<states.size();i++)
    {
        flag=true;
        for(int k=0;k<size;k++)
        {
            for(int l=0;l<size;l++)
            {
                if(arr[k][l].state!=states[i][k][l].state)
                {
                    flag=false;
                    k=size;
                    l=size;
                }
            }
        }
        if(flag)
        {
            found=i;
            i=states.size();
        }
    }
    if(!flag)
    {
        return -1;
    }
    else
    {
        return found;
    }
}

double* ergodic_prob3(Cell** &arr, vector<Cell_light**> &states, vector<short**> &strats)
{
    double* counter = new double[states.size()];
    for(int i=0;i<states.size();i++)
    {
        counter[i]=0;
    }
    double total=0;
    int c=0;
    double* result = new double[states.size()];
    bool flag=false;
    while(!flag)
    {
        for(int i=0;i<size;i++)
        {
            for(int j=0;j<size;j++)
            {
                int count=0;    //counter for around burning cells
                int count_a=0;  //counter for around alive cells
                for(int q=0;q<3;q++)    //checking for burning cells around
                {
                    for(int w=0;w<3;w++)
                    {
                        if(i-1>=0)
                        {
                            if(arr[i-1][j].state==Burning)
                            {
                                count++;
                            }
                            if(arr[i-1][j].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(i+1<size)
                        {
                            if(arr[i+1][j].state==Burning)
                            {
                                count++;
                            }
                            if(arr[i+1][j].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(j-1>=0)
                        {
                            if(arr[i][j-1].state==Burning)
                            {
                                count++;
                            }
                            if(arr[i][j-1].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(j+1<size)
                        {
                            if(arr[i][j+1].state==Burning)
                            {
                                count++;
                            }
                            if(arr[i][j+1].state==Alive)
                            {
                                count_a++;
                            }
                        }
                    }
                }
                if(arr[i][j].state==Alive)  //for alive cells
                {
                    if(dis(gen32)<=burning_prob(count, (int)arr[i][j].isWater)) //checking prob for burning
                    {
                        burning.push_back(&arr[i][j]);
                    }
                }
                else
                {
                    if(arr[i][j].state==Burning)    //for burning cells
                    {
                        double prob=dis(gen32);
                        if(prob<=dead_prob((int)arr[i][j].isWater, count+count_a))    //checking if dead
                        {
                            dying.push_back(&arr[i][j]);
                        }
                        else
                        {
                            if((prob>dead_prob((int)arr[i][j].isWater, count+count_a))&&(prob<=dead_prob((int)arr[i][j].isWater, count+count_a)+ext_prob((int)arr[i][j].isWater, count)))       //cheking for extinguish
                            {
                                extinguish.push_back(&arr[i][j]);
                            }
                        }
                    }
                    else
                    {
                        if(arr[i][j].state==Dead)   //for dead cells
                        {
                            if(dis(gen32)<=recovery_prob(count_a, count))   //checking if dead
                            {
                                recovering.push_back(&arr[i][j]);
                            }
                        }
                    }
                }
            }
        }
        Cell* temp;
        while(!burning.empty())
        {
            temp=burning.back();
            temp->state=Burning;
            temp->isWater=false;
            burning.pop_back();
        }
        while(!dying.empty())
        {
            temp=dying.back();
            temp->state=Dead;
            temp->isWater=false;
            dying.pop_back();
        }
        while(!extinguish.empty())
        {
            temp=extinguish.back();
            temp->state=Alive;
            temp->isWater=false;
            extinguish.pop_back();
        }
        while(!recovering.empty())
        {
            temp=recovering.back();
            temp->state=Alive;
            temp->isWater=false;
            recovering.pop_back();
        }
        
        c=find_state(arr, states);
        if(c!=-1)
        {
            counter[c]=counter[c]+1;
            total=total+1;
            
            for(int i=0;i<size;i++)
            {
                for(int j=0;j<size;j++)
                {
                    arr[i][j].isWater=strats[c][i][j];
                }
            }
        }
        
        flag=true;
        for(int i=0;i<states.size();i++)
        {
            if(counter[i]<30)
            {
                i=states.size();
                flag=false;
            }
        }
        if(total>100000000)
        {
            flag=true;
        }
    }
    for(int i=0;i<states.size();i++)
    {
        result[i]=counter[i]/total;
    }
    cout<<(int)total<<"\n\n";
    return result;
}

// Solving system for (3.11)-(3.12)
double* finding_V(double** &cores, double* &pi,vector<double> &temp_losses, double &exp_loss, int size)
{
    double** all_sys = new double*[size+1];
    for(int i=0;i<=size;i++)
    {
        all_sys[i]=new double[size+1];
    }
    
    //Filling the system
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            if(i==j)
            {
                all_sys[i][j]=(1-cores[i][j]);
            }
            else
            {
                if(cores[i][j]!=0)
                {
                    all_sys[i][j]=-cores[i][j];
                }
                else
                {
                    all_sys[i][j]=0;
                }
            }
        }
        all_sys[i][size]=temp_losses[i]-exp_loss;
        //all_sys[i][size]=temp_losses[i]-temp_losses[i]*pi[i];
    }
    for(int i=0;i<size;i++)
    {
        all_sys[size][i]=pi[i];
    }
    all_sys[size][size]=0;
    
    // Solving system with Gauss method
    double* result = new double[size];
    result=Gauss(all_sys,size);
    return result;
}

// Updating cores for optimizimg
double* core_update(Cell_light** &base, vector<Cell_light**> &other, double* &core, short** &strategy)
{
    double* result = new double [other.size()];
    int count;
    int count_a;
    double temp=1;
    for(int c=0;c<other.size();c++)
    {
        if(core[c]!=0)
        {
            result[c]=transition(base, other[c], strategy);
        }
        else
        {
            result[c]=0;
        }
    }
    return result;
}

// Optimizing the strategy
vector<short**> optimizing(vector<Cell_light**> &states, vector<short**> &all_strats, vector<short**> &strats, double** &cores, double* &v_value, double expected, bool &flag)
{
    int size=3;
    vector<short**> result;
    bool flag_stop=true;
    bool dead_flag;
    double r_loss=0;
    double v_new=0;
    double v_old=0;
    int count_a;
    int count_b;
    vector<vector<short**>> sets(states.size(), vector<short**>());
    for(int c=0;c<states.size();c++)        // loop for each state
    {
        for(int s=0;s<all_strats.size();s++)    // loop for each strat
        {
            /*
            for(int k=0;k<size;k++)
            {
                for(int l=0;l<size;l++)
                {
                    switch (states[c][k][l].state) {
                        case Alive:
                            cout<<"Alive"<<"\t";
                            break;
                        case Burning:
                            cout<<"Burn"<<"\t";
                            break;
                        case Dead:
                            cout<<"Dead"<<"\t";
                            break;
                        case Empty:
                            cout<<"Empty"<<"\t";
                            break;
                        default:
                            break;
                    }
                }
                cout<<"\n";
            }
            cout<<"\n";
            for(int k=0;k<size;k++)
            {
                for(int l=0;l<size;l++)
                {
                    cout<<all_strats[s][k][l]<<"\t";
                }
                cout<<"\n";
            }
            cout<<"\n";
            */
            r_loss=0;
            v_new=0;
            v_old=0;
            for(int i=0;i<size;i++)         // calculating "r"
            {
                for(int j=0;j<size;j++)
                {
                    if((all_strats[s][i][j]==1)&&(states[c][i][j].state==Dead))
                    {
                        dead_flag=true;
                        i=size;
                        j=size;
                    }
                    else
                    {
                        count_b=0;
                        count_a=0;
                        if(i-1>=0)
                        {
                            if(states[c][i-1][j].state==Burning)
                            {
                                count_b++;
                            }
                            if(states[c][i-1][j].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(i+1<size)
                        {
                            if(states[c][i+1][j].state==Burning)
                            {
                                count_b++;
                            }
                            if(states[c][i+1][j].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(j-1>=0)
                        {
                            if(states[c][i][j-1].state==Burning)
                            {
                                count_b++;
                            }
                            if(states[c][i][j-1].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(j+1<size)
                        {
                            if(states[c][i][j+1].state==Burning)
                            {
                                count_b++;
                            }
                            if(states[c][i][j+1].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if((states[c][i][j].state==Alive)&&(count_b==0)&&(all_strats[s][i][j]==1))
                        {
                            dead_flag=true;
                            i=size;
                            j=size;
                        }
                        else
                        {
                            r_loss+=loss_func(all_strats[s][i][j], states[c][i][j].state, count_a, count_b);
                        }
                    }
                }
            }
            if(!dead_flag)
            {
                double* new_core=new double[states.size()];
                new_core=core_update(states[c], states, cores[c], all_strats[s]);   // updating transition core for state
                for(int i=0;i<states.size();i++)        // calculating sum with "v"
                {
                    v_new+=new_core[i]*v_value[i];
                }
                for(int i=0;i<states.size();i++)        // calculating sum with "v"
                {
                    v_old+=cores[c][i]*v_value[i];
                }
                delete[] new_core;
                //if(r_loss+v_new<losses(states, strats)[c]+v_old)
                if(r_loss+v_new<expected+v_value[c])    //checking for optimizing
                {
                    sets[c].push_back(all_strats[s]);
                }
            }
            else
            {
                dead_flag=false;
            }
        }
        if(!sets[c].empty())    // if there are better strategies, update them
        {
            result.push_back(sets[c].back());
            strats[c]=sets[c].back();
            sets[c].clear();
            flag_stop=false;
        }
        else
        {
            result.push_back(strats[c]);    // leave old strategy
        }
    }
    if(flag_stop)
    {
        flag = true;        //  end of optimizing
    }
    return result;
}

int main(int, char const**)
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(2600, 1300), "Wild forest");
    
    //Visuals
    
        //textures
        sf::Texture green;
        green.loadFromFile(resourcePath() + "green.png");
        sf::Texture red;
        red.loadFromFile(resourcePath() + "red.png");
        sf::Font font;
        font.loadFromFile(resourcePath() + "sansation.ttf");
        
        //User oriented
        sf::Text text("Global", font);
        text.setCharacterSize(100);
        text.setStyle(sf::Text::Bold);
        text.setFillColor(sf::Color::White);
        text.move(sf::Vector2f(470,10));
        
        sf::Text text3("Global\nScore", font);
        text3.setCharacterSize(60);
        text3.setStyle(sf::Text::Bold);
        text3.setFillColor(sf::Color::White);
        text3.move(sf::Vector2f(1120,330));
        
        sf::Text text5("0", font);
        text5.setCharacterSize(60);
        text5.setStyle(sf::Text::Bold);
        text5.setFillColor(sf::Color::White);
        text5.move(sf::Vector2f(1140,500));
        
        sf::RectangleShape border1;
        border1.setSize(sf::Vector2f(1000, 1100));
        border1.setOutlineColor(sf::Color::White);
        border1.setFillColor(sf::Color::Transparent);
        border1.setOutlineThickness(2);
        border1.setPosition(80, 150);
        
        sf::RectangleShape border3;
        border3.setSize(sf::Vector2f(300, 100));
        border3.setOutlineColor(sf::Color::White);
        border3.setFillColor(sf::Color::Transparent);
        border3.setOutlineThickness(4);
        border3.setPosition(1129, 490);
        
        //Computer oriented
        sf::Text text2("\tLocal", font);
        text2.setCharacterSize(100);
        text2.setStyle(sf::Text::Bold);
        text2.setFillColor(sf::Color::White);
        text2.move(sf::Vector2f(1750,10));
        
        sf::Text text4("\t\tLocal\n\t\tScore", font);
        text4.setCharacterSize(60);
        text4.setStyle(sf::Text::Bold);
        text4.setFillColor(sf::Color::White);
        text4.move(sf::Vector2f(1160,930));
        
        sf::Text text6("0", font);
        text6.setCharacterSize(60);
        text6.setStyle(sf::Text::Bold);
        text6.setFillColor(sf::Color::White);
        text6.move(sf::Vector2f(1140,820));
        
        sf::RectangleShape border2;
        border2.setSize(sf::Vector2f(1000, 1100));
        border2.setOutlineColor(sf::Color::White);
        border2.setFillColor(sf::Color::Transparent);
        border2.setOutlineThickness(2);
        border2.setPosition(1480, 150);
        
        sf::RectangleShape border4;
        border4.setSize(sf::Vector2f(300, 100));
        border4.setOutlineColor(sf::Color::White);
        border4.setFillColor(sf::Color::Transparent);
        border4.setOutlineThickness(4);
        border4.setPosition(1130, 810);
    
    // User map
    Cell **arr = new Cell*[size];
    for(int i=0;i<size;i++)
    {
        arr[i] = new Cell[size];
    }
    
    //Computer map
    Cell **arr2 = new Cell*[size];
    for(int i=0;i<size;i++)
    {
        arr2[i] = new Cell[size];
    }
    
    
    Cell **arr3 = new Cell*[size];
    for(int i=0;i<size;i++)
    {
        arr3[i] = new Cell[size];
    }
    
    
    
    float step1=view_width/size;
    float step2=view_height/size;
    
    int total_user=0;
    int total_computer=0;
    
    //Generating forest
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            //User
            arr[i][j].image.setPosition(x_0+step1*j, y_0+step2*i);
            arr[i][j].image.setSize(sf::Vector2f(step1,step2));
            arr[i][j].image.setOutlineColor(sf::Color::Black);
            arr[i][j].image.setOutlineThickness(1);
            arr[i][j].pos_x=i;
            arr[i][j].pos_y=j;
            //Computer
            arr2[i][j].image.setPosition(1400+x_0+step1*j, y_0+step2*i);
            arr2[i][j].image.setSize(sf::Vector2f(step1,step2));
            arr2[i][j].image.setOutlineColor(sf::Color::Black);
            arr2[i][j].image.setOutlineThickness(1);
            arr2[i][j].pos_x=i;
            arr2[i][j].pos_y=j;
            if(dis(gen32)<=prob)
            {
                //User
                arr[i][j].image.setFillColor(sf::Color(222,184,135,150));
                arr[i][j].state=Empty;
                //Computer
                arr2[i][j].image.setFillColor(sf::Color(222,184,135,150));
                arr2[i][j].state=Empty;
                
                
                arr3[i][j].image.setFillColor(sf::Color(222,184,135,150));
                arr3[i][j].state=Empty;
            }
            else
            {
                //User
                arr[i][j].image.setFillColor(sf::Color(0,100,0,250));
                arr[i][j].state=Alive;
                //Computer
                arr2[i][j].image.setFillColor(sf::Color(0,100,0,250));
                arr2[i][j].state=Alive;
                
                arr3[i][j].image.setFillColor(sf::Color(0,100,0,250));
                arr3[i][j].state=Alive;
            }
        }
    }
    
    
    // For local strategies:
    
    // Generating local states for each cell
    vector<vector<vector<Cell_light**> > > all_loc_states (size,vector<vector<Cell_light**> >(size,vector <Cell_light**>()));
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            all_loc_states[i][j]=gen_local_states(arr, size, i, j);
        }
    }
    
    // Generating local strategies for each cell
    vector<vector<vector<short**> > > all_loc_strats (size,vector<vector<short**> >(size,vector <short**>()));
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            all_loc_strats[i][j]=gen_local_strats();
        }
    }
    
    vector<vector<vector<short**> > > opt_loc_strats (size,vector<vector<short**> >(size,vector <short**>()));
    
    // Setting all start strategies as zero
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            for(int s=0;s<all_loc_states[i][j].size();s++)
            {
                short** cur = new short*[size];
                for(int q=0;q<size;q++)
                {
                    cur[q] = new short[size];
                }
                opt_loc_strats[i][j].push_back(cur);
                for(int k=0;k<size;k++)
                {
                    for(int l=0;l<size;l++)
                    {
                        opt_loc_strats[i][j][s][k][l]=0;
                    }
                }
            }
        }
    }
    
    // Optimizing
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            bool loc_flag=false;
            vector<double> loc_temp_losses;
            double* loc_ergodic = new double[all_loc_states[i][j].size()];
            double loc_expect;
            double** loc_cores = new double*[all_loc_states[i][j].size()];
            for(int q=0;q<all_loc_states[i][j].size();q++)
            {
                loc_cores[q]=new double[all_loc_states[i][j].size()];
            }
            double* loc_v_func;
            double loc_prev_expect=__DBL_MAX__;
            
            while(!loc_flag)
            {
                // Calculating losses for the current optimal strategy
                loc_temp_losses=losses(all_loc_states[i][j],opt_loc_strats[i][j]);
                
                //Generating transition cores for current strategy
                loc_cores=core_gen(all_loc_states[i][j], opt_loc_strats[i][j]);
            
                //Calculating ergodic probs
                loc_ergodic=ergodic_prob(all_loc_states[i][j], opt_loc_strats[i][j]);
                //ergodic=ergodic_prob2(cores,all_states.size());
                //ergodic = ergodic_prob3(arr, all_states, opt_strats);

                //Calculating expected losses
                loc_expect=exp_loss(loc_ergodic, loc_temp_losses);
                
                // Calculating system v_func for 3.11 - 3.12
                loc_v_func=finding_V(loc_cores, loc_ergodic, loc_temp_losses, loc_expect, all_loc_states[i][j].size());
                
                // Optimizing our current strategy
                opt_loc_strats[i][j]=optimizing(all_loc_states[i][j], all_loc_strats[i][j], opt_loc_strats[i][j], loc_cores, loc_v_func, loc_expect, loc_flag);
                
                //expect=exp_loss(ergodic, temp_losses);

                if(loc_prev_expect<=loc_expect)
                {
                    loc_flag=true;
                }
                else
                {
                    loc_prev_expect=loc_expect;
                }
            }
        }
    }
    
    /*
    // For global strategies
    
    // Generating all possible states
    vector<Cell_light**> all_states;
    all_states=generate_states(arr, size);

    // Generating all posible strats
    vector<short**> all_strats;
    all_strats=generate_strats(arr);
    
    vector<short**> all_Neyman_strats;
    all_Neyman_strats=generate_strats(arr);
    
    vector<short**> opt_strats;
    // Taking first strategy as all 0
    for(int i=0;i<all_states.size();i++)
    {
        short** temp = new short*[size];
        for(int k=0;k<size;k++)
        {
            temp[k]=new short[size];
            for(int w=0;w<size;w++)
            {
                temp[k][w]=0;
            }
        }
        opt_strats.push_back(temp);
    }
    
    // Optimizing procedure
    bool flag=false;
    vector<double> temp_losses;
    double* ergodic = new double[all_states.size()];
    double expect;
    double** cores = new double*[all_states.size()];
    for(int i=0;i<all_states.size();i++)
    {
        cores[i]=new double[all_states.size()];
    }
    double* v_func;
    double prev_expect=__DBL_MAX__;
    
    while(!flag)
    {
        // Calculating losses for the current optimal strategy
        temp_losses=losses(all_states,opt_strats);
        
        //Generating transition cores for current strategy
        cores=core_gen(all_states, opt_strats);

        //Calculating ergodic probs
        ergodic=ergodic_prob(all_states, opt_strats);
        //ergodic=ergodic_prob2(cores,all_states.size());
        //ergodic = ergodic_prob3(arr, all_states, opt_strats);

        //Calculating expected losses
        expect=exp_loss(ergodic, temp_losses);
        
        // Calculating system v_func for 3.11 - 3.12
        v_func=finding_V(cores, ergodic, temp_losses, expect, all_states.size());
        
        // Optimizing our current strategy
        opt_strats=optimizing(all_states, all_strats, opt_strats, cores, v_func, expect, flag);
        
        //expect=exp_loss(ergodic, temp_losses);

        if(prev_expect<=expect)
        {
            flag=true;
        }
        else
        {
            prev_expect=expect;
        }
    }
    */
    int count=0;
    int count_a=0;
    Cell* temp;
    Cell** cur = new Cell*[size];
    for(int k=0;k<size;k++)
    {
        cur[k] = new Cell[size];
    }
    
    int total_3=0;
    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Escape pressed: exit
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
            //Starting a fire
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
            {
                std::uniform_int_distribution<int> dis2(0, size-1);
                int x=dis2(gen32);
                int y=dis2(gen32);
                if(arr[x][y].state!=Empty)
                {
                    //User
                    arr[x][y].image.setFillColor(Color::Red);
                    arr[x][y].state=Burning;
                    total_user=total_user+burning_cost;
                    text5.setString(to_string(total_user));
    
                    //Computer
                    arr2[x][y].image.setFillColor(Color::Red);
                    arr2[x][y].state=Burning;
                    total_computer=total_computer+burning_cost;
                    text6.setString(to_string(total_user));
                }
            }
            //Watering
            if (event.type == sf::Event::MouseButtonPressed)
            {
                int x =(int)(sf::Mouse::getPosition(window)).x;
                int y =(int)(sf::Mouse::getPosition(window)).y;
                x=(int)((float)(x-x_0)/step1);
                y=(int)((float)(y-y_0)/step2);
                if(arr[y][x].isWater)
                {
                    arr[y][x].image.setTexture(NULL);
                    if(arr[y][x].state==Alive)
                    {
                        arr[y][x].image.setFillColor(sf::Color(0,100,0,250));
                    }
                    else
                    {
                        if(arr[y][x].state==Burning)
                        {
                            arr[y][x].image.setFillColor(sf::Color::Red);
                        }
                    }
                    arr[y][x].isWater=false;
                    if((arr[y][x].state!=Dead)&&(arr[y][x].state!=Empty))
                    {
                        total_user=total_user-water_cost;
                        text5.setString(to_string(total_user));
                    }
                }
                else
                {
                    if(arr[y][x].state==Alive)
                    {
                        arr[y][x].image.setTexture(&green,true);
                        arr[y][x].image.setFillColor(sf::Color::White);
                    }
                    else
                    {
                        if(arr[y][x].state==Burning)
                        {
                            arr[y][x].image.setTexture(&red,true);
                            arr[y][x].image.setFillColor(sf::Color::White);
                        }
                    }
                    arr[y][x].isWater = true;
                    if((arr[y][x].state!=Dead)&&(arr[y][x].state!=Empty))
                    {
                        total_user=total_user+water_cost;
                        text5.setString(to_string(total_user));
                    }
                }
            }
            //Proceed a step
            
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Right)
            {
               //while(fire_time<200000)
               //{
                
                
                // FOR LEFT SCREEN
                
                
                for(int i=0;i<size;i++)
                {
                    for(int j=0;j<size;j++)
                    {
                        count=0;    //counter for around burning cells
                        count_a=0;  //counter for around alive cells
                        if(i-1>=0)
                        {
                            if(arr[i-1][j].state==Burning)
                            {
                                count++;
                            }
                            if(arr[i-1][j].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(i+1<size)
                        {
                            if(arr[i+1][j].state==Burning)
                            {
                                count++;
                            }
                            if(arr[i+1][j].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(j-1>=0)
                        {
                            if(arr[i][j-1].state==Burning)
                            {
                                count++;
                            }
                            if(arr[i][j-1].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(j+1<size)
                        {
                            if(arr[i][j+1].state==Burning)
                            {
                                count++;
                            }
                            if(arr[i][j+1].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(arr[i][j].state==Alive)  //for alive cells
                        {
                            if(dis(gen32)<=burning_prob(count, (int)arr[i][j].isWater)) //checking prob for burning
                            {
                                burning.push_back(&arr[i][j]);
                                total_user=total_user+burning_cost;
                            }
                            else
                            {
                                arr[i][j].image.setTexture(NULL);
                                arr[i][j].image.setFillColor(sf::Color(0,100,0,250));
                                arr[i][j].isWater=false;
                            }
                        }
                        else
                        {
                            if(arr[i][j].state==Burning)    //for burning cells
                            {
                                double prob=dis(gen32);
                                if(prob<=dead_prob((int)arr[i][j].isWater, count+count_a))    //checking if dead
                                {
                                    dying.push_back(&arr[i][j]);
                                    total_user=total_user+dead_cost;
                                }
                                else
                                {
                                    if((prob>dead_prob((int)arr[i][j].isWater, count+count_a))&&(prob<=(dead_prob((int)arr[i][j].isWater, count+count_a)+ext_prob((int)arr[i][j].isWater, count))))       //cheking for extinguish
                                    {
                                        extinguish.push_back(&arr[i][j]);
                                    }
                                    else
                                    {
                                        arr[i][j].image.setTexture(NULL);
                                        arr[i][j].image.setFillColor(Color::Red);
                                        arr[i][j].isWater=false;
                                        total_user=total_user+burning_cost/2.0;
                                    }
                                }
                            }
                            else
                            {
                                if(arr[i][j].state==Dead)   //for dead cells
                                {
                                    if(dis(gen32)<=recovery_prob(count_a, count))   //checking if alive
                                    {
                                        recovering.push_back(&arr[i][j]);
                                    }
                                    else
                                    {
                                        total_user=total_user+dead_cost/2.0;
                                    }
                                }
                            }
                        }
                    }
                }
                while(!burning.empty())
                {
                    temp=burning.back();
                    temp->state=Burning;
                    temp->image.setFillColor(Color::Red);
                    temp->image.setTexture(NULL);
                    temp->isWater=false;
                    burning.pop_back();
                }
                while(!dying.empty())
                {
                    temp=dying.back();
                    temp->state=Dead;
                    temp->image.setTexture(NULL);
                    temp->image.setFillColor(Color::Black);
                    temp->isWater=false;
                    dying.pop_back();
                }
                while(!extinguish.empty())
                {
                    temp=extinguish.back();
                    temp->state=Alive;
                    temp->image.setTexture(NULL);
                    temp->image.setFillColor(sf::Color(0,100,0,250));
                    temp->isWater=false;
                    extinguish.pop_back();
                }
                while(!recovering.empty())
                {
                    temp=recovering.back();
                    temp->state=Alive;
                    temp->image.setTexture(NULL);
                    temp->image.setFillColor(sf::Color(0,100,0,250));
                    temp->isWater=false;
                    recovering.pop_back();
                }
                /*
                    for(int i=0;i<size;i++)
                    {
                        for(int j=0;j<size;j++)
                        {
                            if(arr[i][j].state==Burning)
                            {
                                if(i-1>=0)
                                {
                                    if((arr[i-1][j].state==Alive)&&(arr[i-1][j].isWater==false))
                                    {
                                        arr[i-1][j].isWater=true;
                                        arr[i-1][j].image.setTexture(&green,true);
                                        arr[i-1][j].image.setFillColor(sf::Color::White);
                                        total_user=total_user+water_cost;
                                    }
                                }
                                if(i+1<size)
                                {
                                    if((arr[i+1][j].state==Alive)&&(arr[i+1][j].isWater==false))
                                    {
                                        arr[i+1][j].isWater=true;
                                        arr[i+1][j].image.setTexture(&green,true);
                                        arr[i+1][j].image.setFillColor(sf::Color::White);
                                        total_user=total_user+water_cost;
                                    }

                                }
                                if(j-1>=0)
                                {
                                    if((arr[i][j-1].state==Alive)&&(arr[i][j-1].isWater==false))
                                    {
                                        arr[i][j-1].isWater=true;
                                        arr[i][j-1].image.setTexture(&green,true);
                                        arr[i][j-1].image.setFillColor(sf::Color::White);
                                        total_user=total_user+water_cost;
                                    }

                                }
                                if(j+1<size)
                                {
                                    if((arr[i][j+1].state==Alive)&&(arr[i][j+1].isWater==false))
                                    {
                                        arr[i][j+1].isWater=true;
                                        arr[i][j+1].image.setTexture(&green,true);
                                        arr[i][j+1].image.setFillColor(sf::Color::White);
                                        total_user=total_user+water_cost;
                                    }
                                }
                            }
                        }
                    }
                    
                    //For global strats
                    
                    int c=find_state(arr, all_states);
                    if(c!=-1)
                    {
                        for(int i=0;i<size;i++)
                        {
                            for(int j=0;j<size;j++)
                            {
                                arr[i][j].isWater=opt_strats[c][i][j];
                                if(arr[i][j].isWater)
                                {
                                    switch (arr[i][j].state) {
                                        case Alive:
                                            arr[i][j].image.setTexture(&green,true);
                                            arr[i][j].image.setFillColor(sf::Color::White);
                                            total_user=total_user+water_cost;
                                            break;
                                        case Burning:
                                            arr[i][j].image.setTexture(&red,true);
                                            arr[i][j].image.setFillColor(sf::Color::White);
                                            total_user=total_user+water_cost;
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        for(int i=0;i<size;i++)
                        {
                            for(int j=0;j<size;j++)
                            {
                                arr[i][j].isWater=0;
                                switch (arr[i][j].state) {
                                    case Alive:
                                        arr[i][j].image.setTexture(NULL);
                                        arr[i][j].image.setFillColor(sf::Color(0,100,0,250));
                                        break;
                                    case Dead:
                                        arr[i][j].image.setTexture(NULL);
                                        arr[i][j].image.setFillColor(Color::Black);
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                
            
                */
                
                // FOR RIGHT SCREEN
                
                for(int i=0;i<size;i++)
                {
                    for(int j=0;j<size;j++)
                    {
                        count=0;    //counter for around burning cells
                        count_a=0;  //counter for around alive cells
                        if(i-1>=0)
                        {
                            if(arr2[i-1][j].state==Burning)
                            {
                                count++;
                            }
                            if(arr2[i-1][j].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(i+1<size)
                        {
                            if(arr2[i+1][j].state==Burning)
                            {
                                count++;
                            }
                            if(arr2[i+1][j].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(j-1>=0)
                        {
                            if(arr2[i][j-1].state==Burning)
                            {
                                count++;
                            }
                            if(arr2[i][j-1].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(j+1<size)
                        {
                            if(arr2[i][j+1].state==Burning)
                            {
                                count++;
                            }
                            if(arr2[i][j+1].state==Alive)
                            {
                                count_a++;
                            }
                        }
                        if(arr2[i][j].state==Alive)  //for alive cells
                        {
                            if(dis(gen32)<=burning_prob(count, (int)arr2[i][j].isWater)) //checking prob for burning
                            {
                                burning.push_back(&arr2[i][j]);
                                total_computer=total_computer+burning_cost;
                            }
                            else
                            {
                                arr2[i][j].image.setTexture(NULL);
                                arr2[i][j].image.setFillColor(sf::Color(0,100,0,250));
                                arr2[i][j].isWater=false;
                            }
                        }
                        else
                        {
                            if(arr2[i][j].state==Burning)    //for burning cells
                            {
                                double prob=dis(gen32);
                                if(prob<=dead_prob((int)arr2[i][j].isWater, count+count_a))    //checking if dead
                                {
                                    dying.push_back(&arr2[i][j]);
                                    total_computer=total_computer+dead_cost;
                                }
                                else
                                {
                                    if((prob>dead_prob((int)arr2[i][j].isWater, count+count_a))&&(prob<=(dead_prob((int)arr2[i][j].isWater, count+count_a)+ext_prob((int)arr2[i][j].isWater, count))))       //cheking for extinguish
                                    {
                                        extinguish.push_back(&arr2[i][j]);
                                    }
                                    else
                                    {
                                        arr2[i][j].image.setTexture(NULL);
                                        arr2[i][j].image.setFillColor(Color::Red);
                                        arr2[i][j].isWater=false;
                                        total_computer=total_computer+burning_cost/2.0;
                                    }
                                }
                            }
                            else
                            {
                                if(arr2[i][j].state==Dead)   //for dead cells
                                {
                                    if(dis(gen32)<=recovery_prob(count_a, count))   //checking if alive
                                    {
                                        recovering.push_back(&arr2[i][j]);
                                    }
                                    else
                                    {
                                        total_computer=total_computer+dead_cost/2.0;
                                    }
                                }
                            }
                        }
                    }
                }
                while(!burning.empty())
                {
                    temp=burning.back();
                    temp->state=Burning;
                    temp->image.setFillColor(Color::Red);
                    temp->image.setTexture(NULL);
                    temp->isWater=false;
                    burning.pop_back();
                }
                while(!dying.empty())
                {
                    temp=dying.back();
                    temp->state=Dead;
                    temp->image.setTexture(NULL);
                    temp->image.setFillColor(Color::Black);
                    temp->isWater=false;
                    dying.pop_back();
                }
                while(!extinguish.empty())
                {
                    temp=extinguish.back();
                    temp->state=Alive;
                    temp->image.setTexture(NULL);
                    temp->image.setFillColor(sf::Color(0,100,0,250));
                    temp->isWater=false;
                    extinguish.pop_back();
                }
                while(!recovering.empty())
                {
                    temp=recovering.back();
                    temp->state=Alive;
                    temp->image.setTexture(NULL);
                    temp->image.setFillColor(sf::Color(0,100,0,250));
                    temp->isWater=false;
                    recovering.pop_back();
                }
                
                //For local strats
                
                for(int i=0;i<size;i++)
                {
                    for(int j=0;j<size;j++)
                    {
                        cur[0][0].state=Empty;
                        cur[2][2].state=Empty;
                        cur[0][2].state=Empty;
                        cur[2][0].state=Empty;
                        if(i-1>=0)
                        {
                            cur[0][1].state=arr2[i-1][j].state;
                        }
                        else
                        {
                            cur[0][1].state=Empty;
                        }
                        if (i+1<size) {
                            cur[2][1].state=arr2[i+1][j].state;
                        }
                        else
                        {
                            cur[2][1].state=Empty;
                        }
                        if (j-1>=0) {
                            cur[1][0].state=arr2[i][j-1].state;
                        }
                        else
                        {
                            cur[1][0].state=Empty;
                        }
                        if (j+1<size) {
                            cur[1][2].state=arr2[i][j+1].state;
                        }
                        else
                        {
                            cur[1][2].state=Empty;
                        }
                        cur[1][1].state=arr2[i][j].state;
                        int c=find_local_state(cur, all_loc_states[i][j]);
                        if(c!=-1)
                        {
                            arr2[i][j].isWater=opt_loc_strats[i][j][c][1][1];
                            if(arr2[i][j].isWater)
                            {
                                switch (arr2[i][j].state){
                                    case Alive:
                                        arr2[i][j].image.setTexture(&green,true);
                                        arr2[i][j].image.setFillColor(sf::Color::White);
                                        total_user=total_user+water_cost;
                                        break;
                                    case Burning:
                                        arr2[i][j].image.setTexture(&red,true);
                                        arr2[i][j].image.setFillColor(sf::Color::White);
                                        total_user=total_user+water_cost;
                                        break;
                                    default:
                                        break;
                                        }
                            }
                        }
                        else
                        {
                            arr2[i][j].isWater=0;
                            switch (arr2[i][j].state) {
                                case Alive:
                                    arr2[i][j].image.setTexture(NULL);
                                    arr2[i][j].image.setFillColor(sf::Color(0,100,0,250));
                                    break;
                                case Dead:
                                    arr2[i][j].image.setTexture(NULL);
                                    arr2[i][j].image.setFillColor(Color::Black);
                                    break;
                                default:
                                    break;
                            }
                        }
                        //delete[] cur;
                    }
                }
                 
                /*
                   for(int i=0;i<size;i++)
                   {
                       for(int j=0;j<size;j++)
                       {
                           count=0;    //counter for around burning cells
                           count_a=0;  //counter for around alive cells
                           if(i-1>=0)
                           {
                               if(arr3[i-1][j].state==Burning)
                               {
                                   count++;
                               }
                               if(arr3[i-1][j].state==Alive)
                               {
                                   count_a++;
                               }
                           }
                           if(i+1<size)
                           {
                               if(arr3[i+1][j].state==Burning)
                               {
                                   count++;
                               }
                               if(arr3[i+1][j].state==Alive)
                               {
                                   count_a++;
                               }
                           }
                           if(j-1>=0)
                           {
                               if(arr3[i][j-1].state==Burning)
                               {
                                   count++;
                               }
                               if(arr3[i][j-1].state==Alive)
                               {
                                   count_a++;
                               }
                           }
                           if(j+1<size)
                           {
                               if(arr3[i][j+1].state==Burning)
                               {
                                   count++;
                               }
                               if(arr3[i][j+1].state==Alive)
                               {
                                   count_a++;
                               }
                           }
                           if(arr3[i][j].state==Alive)  //for alive cells
                           {
                               if(dis(gen32)<=burning_prob(count, (int)arr3[i][j].isWater)) //checking prob for burning
                               {
                                   burning.push_back(&arr3[i][j]);
                                   total_3=total_3+burning_cost;
                               }
                               else
                               {
                                   arr3[i][j].image.setTexture(NULL);
                                   arr3[i][j].image.setFillColor(sf::Color(0,100,0,250));
                                   arr3[i][j].isWater=false;
                               }
                           }
                           else
                           {
                               if(arr3[i][j].state==Burning)    //for burning cells
                               {
                                   double prob=dis(gen32);
                                   if(prob<=dead_prob((int)arr3[i][j].isWater, count+count_a))    //checking if dead
                                   {
                                       dying.push_back(&arr3[i][j]);
                                       total_3=total_3+dead_cost;
                                   }
                                   else
                                   {
                                       if((prob>dead_prob((int)arr3[i][j].isWater, count+count_a))&&(prob<=(dead_prob((int)arr3[i][j].isWater, count+count_a)+ext_prob((int)arr3[i][j].isWater, count))))       //cheking for extinguish
                                       {
                                           extinguish.push_back(&arr3[i][j]);
                                       }
                                       else
                                       {
                                           arr3[i][j].image.setTexture(NULL);
                                           arr3[i][j].image.setFillColor(Color::Red);
                                           arr3[i][j].isWater=false;
                                           total_3=total_3+burning_cost/2.0;
                                       }
                                   }
                               }
                               else
                               {
                                   if(arr3[i][j].state==Dead)   //for dead cells
                                   {
                                       if(dis(gen32)<=recovery_prob(count_a, count))   //checking if alive
                                       {
                                           recovering.push_back(&arr3[i][j]);
                                       }
                                       else
                                       {
                                           total_3=total_3+dead_cost/2.0;
                                       }
                                   }
                               }
                           }
                       }
                   }
                   while(!burning.empty())
                   {
                       temp=burning.back();
                       temp->state=Burning;
                       temp->image.setFillColor(Color::Red);
                       temp->image.setTexture(NULL);
                       temp->isWater=false;
                       burning.pop_back();
                   }
                   while(!dying.empty())
                   {
                       temp=dying.back();
                       temp->state=Dead;
                       temp->image.setTexture(NULL);
                       temp->image.setFillColor(Color::Black);
                       temp->isWater=false;
                       dying.pop_back();
                   }
                   while(!extinguish.empty())
                   {
                       temp=extinguish.back();
                       temp->state=Alive;
                       temp->image.setTexture(NULL);
                       temp->image.setFillColor(sf::Color(0,100,0,250));
                       temp->isWater=false;
                       extinguish.pop_back();
                   }
                   while(!recovering.empty())
                   {
                       temp=recovering.back();
                       temp->state=Alive;
                       temp->image.setTexture(NULL);
                       temp->image.setFillColor(sf::Color(0,100,0,250));
                       temp->isWater=false;
                       recovering.pop_back();
                   }
                   for(int i=0;i<size;i++)
                   {
                       for(int j=0;j<size;j++)
                       {
                           if(arr3[i][j].state==Burning)
                           {
                               arr3[i][j].isWater=true;
                               arr3[i][j].image.setTexture(&red,true);
                               arr3[i][j].image.setFillColor(sf::Color::White);
                               total_3=total_3+water_cost;
                           }
                       }
                   }
                */
                fire_time++;
                text5.setString(to_string(total_user));
                text6.setString(to_string(total_computer));
               // text.setString("T = "+to_string(fire_time));
            }
                   window.clear(sf::Color::Black);
                    
                    for(int i=0;i<size;i++)
                    {
                        for(int j=0;j<size;j++)
                        {
                            window.draw(arr[i][j].image);
                            window.draw(arr2[i][j].image);
                        }
                    }
                    window.draw(border1);
                    window.draw(border2);
                    window.draw(border3);
                    window.draw(border4);
                    window.draw(text);
                    window.draw(text2);
                    window.draw(text3);
                    window.draw(text4);
                    window.draw(text5);
                    window.draw(text6);

                    // Update the window
                    window.display();
            }

        // Clear screen
        window.clear(sf::Color::Black);
        
        for(int i=0;i<size;i++)
        {
            for(int j=0;j<size;j++)
            {
                window.draw(arr[i][j].image);
                window.draw(arr2[i][j].image);
            }
        }
        window.draw(border1);
        window.draw(border2);
        window.draw(border3);
        window.draw(border4);
        window.draw(text);
        window.draw(text2);
        window.draw(text3);
        window.draw(text4);
        window.draw(text5);
        window.draw(text6);

        // Update the window
        window.display();
    }
    return EXIT_SUCCESS;
}
