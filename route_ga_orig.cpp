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

const int UnchangedStepTolerant = 1000;

const char* DataFile = "test_data.dat";

long select_time = 0, crossover_time = 0, mutation_time = 0, evaluate_time = 0;

int unchanged_count = 0;    //counter for unchanged steps

struct Point
{
    int x, y;
    Point(){}
    Point(int _x, int _y):x(_x), y(_y){}
};

Point passNode[MapNodeAmount], startPoint(0, 0), endPoint(MapSize - 1, MapSize - 1);

int point_map[MapSize][MapSize];

int point_distance(const Point& a, const Point& b)
{
    return abs(a.x - b.x) + abs(a.y - b.y);
}

struct Solution
{
    int NodeID[MapNodeAmount];
    int cache_value;
    int value()
    {
        if(cache_value == -1){
            long start_time = clock();
            int sum = point_distance(startPoint, passNode[NodeID[0]]);
            for(int i = 1; i < MapNodeAmount; i ++)
                sum += point_distance(passNode[NodeID[i - 1]], passNode[NodeID[i]]);
            sum += point_distance(passNode[NodeID[MapNodeAmount - 1]], endPoint);
            evaluate_time += clock() - start_time;
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
        return *this;
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

int select()
{
    long tmp_start_time = clock();
    int ans = rand() % (SolutionAmount / 2);
    select_time += clock() - tmp_start_time;
    return ans;
}

int part_select()
{
    return rand() % MapNodeAmount;
}

void mutation(int target)
{
    int origin = select();
    int r1 = part_select();
    int r2 = part_select();
    for(int i = 0; i < MapNodeAmount; i ++)
    {
        solutions[target].solution.NodeID[i] = solutions[origin].solution.NodeID[i];
    }
    swap(solutions[target].solution.NodeID[r1], solutions[target].solution.NodeID[r2]);
    solutions[target].solution.cache_value = -1;
}

void crossover(int target)
{
    int left = select(), right = select();
    int rad1 = part_select(), rad2 = part_select();
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
    long start = clock();
    for(int i = SolutionAmount / 2; i < 3 * SolutionAmount / 4; i ++)
        crossover(i);
    crossover_time += clock() - start;
    start = clock();
    for(int i = SolutionAmount * 3 / 4; i < SolutionAmount; i ++)
        mutation(i);
    mutation_time += clock() - start;
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

void initialize_random_data()
{
    for(int i = 0; i < MapNodeAmount; i ++)
    {
        passNode[i].x = rand() % MapSize;
        passNode[i].y = rand() % MapSize;
    }
    store_data();
}

void initialize()
{
    srand(time(0));
    select_time = crossover_time = mutation_time = evaluate_time = 0;
    //initialize_random_data();
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
                case 3: printf("-");break;  //left arrow
                case 4: printf("-");break;  //right arrow
                case 5: printf("|");break;  //up arrow
                case 6: printf("|");break;  //down arrow
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
    initialize_map();
    draw_route(solution);
    print_map();
    for(int i = 0; i < MapNodeAmount; i ++)
        printf("%d\t", solution.NodeID[i]);
    printf("\nRoute Cost:%d\n", solution.value());
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

void solution_framework()
{
    solutions[0].solution = get_greedy_solution();
    sort(solutions, solutions + SolutionAmount);
    commence_genetic();
}

int main()
{
    initialize();
    solution_framework();
    printf("Select Time:%d\nCrossover Time:%d\nMutation Time:%d\nEvaluation Time:%d\n", select_time, crossover_time, mutation_time, evaluate_time);
    return 0;
}
