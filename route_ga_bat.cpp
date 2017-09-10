#include <time.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>

#define null NULL

using namespace std;

const int INF = 100000 + 7;

const int MapSize = 30;
const int MapNodeAmount = 30;

const int SolutionAmount = 400; //keep half for best solution
const int IteratorTimes = 10000;

const int UnchangedStepTolerant = 500;

const char* DataFile = "test_data.dat";
const char* ResultFile = "result.txt";

int unchanged_count = 0;

FILE *fout = null;

struct Point
{
    int x, y;
    Point(){}
    Point(int _x, int _y):x(_x), y(_y){}
};

Point passNode[MapNodeAmount];

Point startPoint(0, 0), endPoint(MapSize - 1, MapSize - 1);

int point_map[MapSize][MapSize];

int point_distance(const Point& a, const Point& b)
{
    return abs(a.x - b.x) + abs(a.y - b.y);
    //return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

struct Solution
{
    int NodeID[MapNodeAmount];
    int cache_value = -1;
    int value()
    {
        if(cache_value == -1){
            int sum = point_distance(startPoint, passNode[NodeID[0]]);
            for(int i = 1; i < MapNodeAmount; i ++)
                sum += point_distance(passNode[NodeID[i - 1]], passNode[NodeID[i]]);
            sum += point_distance(passNode[NodeID[MapNodeAmount - 1]], endPoint);
            cache_value = sum;
        }
        return cache_value;
    }
    void initialize()
    {
        for(int i = 0; i < MapNodeAmount; i ++)
            NodeID[i] = i;
        random_shuffle(NodeID, NodeID + MapNodeAmount);
        cache_value = -1;
    }
    int hashCode()
    {
        int ans = 0;
        for(int i = 0; i < MapNodeAmount; i ++)
            ans = 31 * ans + 2 * NodeID[i];
        return ans;
    }
    void copy(const Solution& solution)
    {
        for(int i = 0; i < MapNodeAmount; i ++)
            NodeID[i] = solution.NodeID[i];
        cache_value = -1;
    }
    Solution& operator=(const Solution& solution)
    {
        copy(solution);
    }
    Solution(const Solution& solution)
    {
        copy(solution);
    }
    Solution(){}
};

struct Chromosome
{
    Solution solution;

    void initialize()
    {
        solution.initialize();
    }

    int value()
    {
        return solution.value();
    }

    void print()
    {
        printf("Cost: %d\n", value());
    }

    bool operator<(Chromosome& r)
    {
        return value() < r.value();
    }
};

Chromosome solutions[SolutionAmount];

void mutation(int target)
{
    int count = 2;
    int origin = rand() % SolutionAmount;
    int r1, r2;
    while(count --)
    {
        r1 = rand() % MapNodeAmount;
        r2 = rand() % MapNodeAmount;
        for(int i = 0; i < MapNodeAmount; i ++)
        {
            solutions[target].solution.NodeID[i] = solutions[origin].solution.NodeID[i];
        }
        swap(solutions[target].solution.NodeID[r1], solutions[target].solution.NodeID[r2]);
    }
    solutions[target].solution.cache_value = -1;
}

void crossover(int target)
{
    int left = rand() % (SolutionAmount / 2), right = rand() % (SolutionAmount / 2);
    int rad1 = rand() % MapNodeAmount, rad2 = rand() % MapNodeAmount;

    int ptn = 0, ctn = 0;
    if(rad1 > rad2) swap(rad1, rad2);
    bool table[MapNodeAmount] = {false};
    for(int i = 0; i < MapNodeAmount; i ++)
        table[i] = false;
    for(int i = rad1; i < rad2; i ++)
    {
        solutions[target].solution.NodeID[ptn ++] = solutions[left].solution.NodeID[i];
        table[solutions[left].solution.NodeID[i]] = true;
    }
    while(ptn < MapNodeAmount)
    {
        if(!table[solutions[right].solution.NodeID[ctn]])
            solutions[target].solution.NodeID[ptn ++] = solutions[right].solution.NodeID[ctn];
        ctn ++;
    }
    solutions[target].solution.cache_value = -1;
}


void nextGeneration()
{
    for(int i = SolutionAmount / 2; i < SolutionAmount; i ++)
    {
        if(rand() % 2)
            crossover(i);
        else
            mutation(i);
    }
    sort(solutions, solutions + SolutionAmount);
}

void initialize_map()
{
    for(int i = 0; i < MapSize; i ++)
    {
        for(int j = 0; j < MapSize; j ++)
        {
            point_map[i][j] = 0;
        }
    }
    for(int i = 0; i < MapNodeAmount; i ++)
    {
        point_map[passNode[i].x][passNode[i].y] = 1;
    }
}

void store_data()
{
    FILE *fp = fopen(DataFile, "wb");
    fwrite(passNode, sizeof(Point), MapNodeAmount, fp);
    fclose(fp);
}

void load_data()
{
    FILE *fp = fopen(DataFile, "rb");
    if(!fp) perror("[ERR]Cannot open datafile\n");
    fread(passNode, sizeof(Point), MapNodeAmount, fp);
    fclose(fp);
}

void open_resultFile()
{
    fout = fopen(ResultFile, "w");
}

void close_resultFile()
{
    fclose(fout);
}
void initialize()
{
    srand(time(0));
    /*
    for(int i = 0; i < MapNodeAmount; i ++)
    {
        passNode[i].x = rand() % MapSize;
        passNode[i].y = rand() % MapSize;
    }
    store_data();
    */
    for(int i = 0; i < SolutionAmount; i ++)
        solutions[i].initialize();
    unchanged_count = 0;

    load_data();
}

void print_map()
{
    for(int i = 0; i < MapSize; i ++)
    {
        for(int j = 0; j < MapSize; j ++)
        {
            if(i == startPoint.x && j == startPoint.y) printf("X");
            else if(i == endPoint.x && j == endPoint.y) printf("X");
            else switch(point_map[i][j])
            {
                case 0: printf(" ");break;
                case 1: printf("X");break;
                case 2: printf("+");break;
                case 3: printf("-");break;
                case 4: printf("-");break;
                case 5: printf("|");break;
                case 6: printf("|");break;
            }
            printf(" ");
        }
        printf("\n");
    }
    printf("\n");
}

void draw_edge(Point &a, Point &b)
{
    int min_x = a.x, min_y = a.y;
    int max_x = b.x, max_y = b.y;
    int ctn = 0;
    int x_symbol = 6, y_symbol = 4;
    if(min_x > max_x) {swap(min_x, max_x); ctn ++; x_symbol = 5;}
    if(min_y > max_y) {swap(min_y, max_y); ctn ++; y_symbol = 3;}
    int y_base = ctn % 2 ? max_x : min_x;
    for(int i = min_x; i <= max_x; i ++)
        point_map[i][max_y] = point_map[i][max_y] ? point_map[i][max_y] : x_symbol;
    for(int i = min_y; i <= max_y; i ++)
        point_map[y_base][i] = point_map[y_base][i] ? point_map[y_base][i] : y_symbol;
}

void draw_route(Solution solution)
{
    draw_edge(startPoint, passNode[solution.NodeID[0]]);
    for(int i = 1; i < MapNodeAmount; i ++)
        draw_edge(passNode[solution.NodeID[i - 1]], passNode[solution.NodeID[i]]);
    draw_edge(passNode[solution.NodeID[MapNodeAmount - 1]], endPoint);
}

void draw_map(Solution solution)
{
    fprintf(fout, "%d\r\n", solution.value());
    return;
    initialize_map();
    draw_route(solution);
    print_map();
    printf("Route Cost:%d\n", solution.value());
}

int find_min_next_point(bool* visited, Point& source)
{
    int min = INF, index = -1, tmp;
    for(int i = 0; i < MapNodeAmount; i ++)
    {
        if(visited[i]) continue;
        tmp = point_distance(passNode[i], source);
        if(min > tmp)
        {
            min = tmp;
            index = i;
        }
    }
    if(index != -1) visited[index] = true;
    return index;
}

void commence_genetic()
{
    int iterator_count = 0, last_best_value = solutions[0].value();
    while(iterator_count < IteratorTimes && unchanged_count < UnchangedStepTolerant)
    {
        nextGeneration();
        iterator_count ++;
        if(last_best_value == solutions[0].value())
            unchanged_count ++;
        else
            unchanged_count = 0;
        last_best_value = solutions[0].value();
    }
    draw_map(solutions[0].solution);
    //printf("IteratorRatio:%.2f%\n", 100.0 * (iterator_count - unchanged_count) / IteratorTimes);


}

Solution get_greedy_solution()
{
    bool visited[MapNodeAmount];
    for(int i = 0; i < MapNodeAmount; i ++)
        visited[i] = false;

    Solution greedy_solution;

    greedy_solution.NodeID[0] = find_min_next_point(visited, startPoint);
    for(int i = 1; i < MapNodeAmount; i ++)
        greedy_solution.NodeID[i] = find_min_next_point(visited, passNode[greedy_solution.NodeID[i - 1]]);

    return greedy_solution;
}

void commence_greedy()
{
    Solution s = get_greedy_solution();
    //draw_map(s);
}

void solution_framework()
{
    solutions[0].solution = get_greedy_solution();
    sort(solutions, solutions + SolutionAmount);
    commence_genetic();
}

int main()
{
    int generate_count = 100;
    open_resultFile();
    for(int i = 0; i < generate_count; i ++)
    {
        initialize();
        //commence_greedy();
        solution_framework();
        printf("Complete Percentage:%.2f%%\n", 100.0 * (i + 1) / generate_count);
    }
    close_resultFile();
    return 0;
}
