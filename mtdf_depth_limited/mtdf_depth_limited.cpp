// mtdf_depth_limited.cpp : Defines the entry point for the application.
//
// negamax_depth_limited.cpp : Defines the entry point for the application.
//

#include "mtdf_depth_limited.h"
#include <vector>
#include <array>
#include <algorithm>
#include <map> 
#include <time.h>
#include <chrono>
#include <random>
using namespace std;

template <typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
    if (!v.empty()) {
        out << '[';
        std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
        out << "\b\b]";
    }
    return out;
}

template <class T, std::size_t N>
ostream& operator<<(ostream& o, const array<T, N>& arr)
{
    copy(arr.cbegin(), arr.cend(), ostream_iterator<T>(o, " "));
    return o;
}

struct Move {
    int i;
    int j;
    int score;
};

struct CacheNode {
    int score;
    int depth;
    int Flag;
};

map<int, int> StateCache;
map<int, CacheNode> Cache;
int StateCachePuts = 0;
bool move_sorter(Move const& move1, Move const& move2) {
    return move1.score > move2.score;
}
int CacheHits = 0;
int CacheCutoffs = 0;
int CachePuts = 0;
int MaximumDepth = 0;
int StateCacheHits = 0;
Move bestMove;

int GameBoard[15][15] = {
    //0  1  2  3  4  5  6  7  8  9  0  1  2  3  4      
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //2
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //3
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //4
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //5
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //6
    { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, //7
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //8
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //9
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //10
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //11
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //12
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //13
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //14
};
int fc = 0;
const int Rows = 15;
const int Columns = 15;
const int WIN_DETECTED = numeric_limits<int>::min() + 1;
const int LiveOne = 10;
const int DeadOne = 1;
const int LiveTwo = 100;
const int DeadTwo = 10;
const int LiveThree = 1000;
const int DeadThree = 100;
const int LiveFour = 10000;
const int DeadFour = 1000;
const int Five = 100000;
int evaluateblock(int blocks, int pieces) {
    if (blocks == 0) {
        switch (pieces) {
        case 1:
            return LiveOne;
        case 2:
            return LiveTwo;
        case 3:
            return LiveThree;
        case 4:
            return LiveFour;
        default:
            return Five;
        }
    }
    else if (blocks == 1) {
        switch (pieces) {
        case 1:
            return DeadOne;
        case 2:
            return DeadTwo;
        case 3:
            return DeadThree;
        case 4:
            return DeadFour;
        default:
            return Five;
        }
    }
    else {
        if (pieces >= 5) {
            return Five;
        }
        else {
            return 0;
        }
    }
}

int eval_board(int Board[Rows][Columns], int pieceType, array<int, 4> const& restrictions) {
    int score = 0;
    int min_r = restrictions[0];
    int min_c = restrictions[1];
    int max_r = restrictions[2];
    int max_c = restrictions[3];
    for (int row = min_r; row < max_r + 1; row++) {
        for (int column = min_c; column < max_c + 1; column++) {
            if (Board[row][column] == pieceType) {
                int block = 0;
                int piece = 1;
                // left
                if (column == 0 || Board[row][column - 1] != 0) {
                    block++;
                }
                // pieceNum
                for (column++; column < Columns && Board[row][column] == pieceType; column++) {
                    piece++;
                }
                // right
                if (column == Columns || Board[row][column] != 0) {
                    block++;
                }
                score = score + evaluateblock(block, piece);
            }
        }
    }

    for (int column = min_c; column < max_c + 1; column++) {
        for (int row = min_r; row < max_r + 1; row++) {
            if (Board[row][column] == pieceType) {
                int block = 0;
                int piece = 1;
                // left
                if (row == 0 || Board[row - 1][column] != 0) {
                    block++;
                }
                // pieceNum
                for (row++; row < Rows && Board[row][column] == pieceType; row++) {
                    piece++;
                }
                // right
                if (row == Rows || Board[row][column] != 0) {
                    block++;
                }
                score += evaluateblock(block, piece);
            }
        }
    }

    for (int n = min_r; n < (max_c - min_c + max_r); n += 1) {
        int r = n;
        int c = min_c;
        while (r >= min_r && c <= max_c) {
            if (r <= max_r) {
                if (Board[r][c] == pieceType) {
                    int block = 0;
                    int piece = 1;
                    // left
                    if (c == 0 || r == Rows - 1 || Board[r + 1][c - 1] != 0) {
                        block++;
                    }
                    // pieceNum
                    r--;
                    c++;
                    for (; r >= 0 && Board[r][c] == pieceType; r--) {
                        piece++;
                        c++;
                    }
                    // right
                    if (r < 0 || c == Columns || Board[r][c] != 0) {
                        block++;
                    }
                    score += evaluateblock(block, piece);
                }
            }
            r -= 1;
            c += 1;
        }
    }

    for (int n = min_r - (max_c - min_c); n <= max_r; n++) {
        int r = n;
        int c = min_c;
        while (r <= max_r && c <= max_c) {
            if (r >= min_r && r <= max_r) {
                if (Board[r][c] == pieceType) {
                    int  block = 0;
                    int piece = 1;
                    // left
                    if (c == 0 || r == 0 || Board[r - 1][c - 1] != 0) {
                        block++;
                    }
                    // pieceNum
                    r++;
                    c++;
                    for (; r < Rows && Board[r][c] == pieceType; r++) {
                        piece++;
                        c++;
                    }
                    // right
                    if (r == Rows || c == Columns || Board[r][c] != 0) {
                        block++;
                    }
                    score += evaluateblock(block, piece);
                }
            }
            r += 1;
            c += 1;
        }

    }
    return score;
}

array<array<int, 9>, 4> get_directions(int Board[15][15], int x, int y) {
    array<int, 9> a;
    array<int, 9> b;
    array<int, 9> c;
    array<int, 9> d;
    int a_i = 0;
    int b_i = 0;
    int c_i = 0;
    int d_i = 0;

    for (int i = -4; i < 5; i++) {
        if (x + i >= 0 && x + i <= Rows - 1) {
            a[a_i] = Board[x + i][y];
            a_i++;
            if (y + i >= 0 && y + i <= Columns - 1) {
                b[b_i] = Board[x + i][y + i];
                b_i++;
            }
        }
        if (y + i >= 0 && y + i <= Columns - 1) {
            c[c_i] = Board[x][y + i];
            c_i++;
            if (x - i >= 0 && x - i <= Rows - 1) {
                d[d_i] = Board[x - i][y + i];
                d_i++;
            }
        }

    }
    if (a_i != 9) {
        a[a_i] = 2;
    }
    if (b_i != 9) {
        b[b_i] = 2;
    }
    if (c_i != 9) {
        c[c_i] = 2;
    }
    if (d_i != 9) {
        d[d_i] = 2;
    }

    array<array<int, 9>, 4> Directions = { a,b,c,d };
    return Directions;
}


bool check_directions(array<int, 9> const& arr) {
    int size = 9;
    for (int i = 0; i < size - 4; i++) {
        if (arr[i] != 0) {
            if (arr[i] == 2 || arr[i + 1] == 2 || arr[i + 2] == 2 || arr[i + 3] == 2 || arr[i + 4] == 2) {
                return false;
            }
            if (arr[i] == arr[i + 1] && arr[i] == arr[i + 2] && arr[i] == arr[i + 3] && arr[i] == arr[i + 4]) {
                return true;
            }
        }
    }
    return false;
}

bool checkwin(int Board[15][15], int x, int y) {
    array<array<int, 9>, 4> Directions = get_directions(Board, x, y);
    for (int i = 0; i < 4; i++) {

        if (check_directions(Directions[i])) {
            return true;
        }
    }
    return false;
}


bool remoteCell(int Board[15][15], int r, int c) {
    for (int i = r - 2; i <= r + 2; i++) {
        if (i < 0 || i >= Rows) continue;
        for (int j = c - 2; j <= c + 2; j++) {
            if (j < 0 || j >= Columns) continue;
            if (Board[i][j] != 0) return false;
        }
    }
    return true;
}
array<int, 4> Get_restrictions(int Board[15][15]) {
    int min_r = numeric_limits<int>::max() - 1;
    int min_c = numeric_limits<int>::max() - 1;
    int max_r = numeric_limits<int>::min() + 1;
    int max_c = numeric_limits<int>::min() + 1;
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Columns; j++) {
            if (Board[i][j] != 0) {
                min_r = min(min_r, i);
                min_c = min(min_c, j);
                max_r = max(max_r, i);
                max_c = max(max_c, j);
            }
        }
    }
    if (min_r - 2 < 0) {
        min_r = 2;
    }
    if (min_c - 2 < 0) {
        min_c = 2;
    }
    if (max_r + 2 >= Rows) {
        max_r = Rows - 3;
    }
    if (max_c + 2 >= Columns) {
        max_c = Columns - 3;
    }
    array<int, 4> restrictions = { min_r, min_c, max_r, max_c };
    return restrictions;
}

array<int, 4> Change_restrictions(array<int, 4> const& restrictions, int i, int j) {
    int min_r = restrictions[0];
    int min_c = restrictions[1];
    int max_r = restrictions[2];
    int max_c = restrictions[3];
    if (i < min_r) {
        min_r = i;
    }
    else if (i > max_r) {
        max_r = i;
    }
    if (j < min_c) {
        min_c = j;
    }
    else if (j > max_c) {
        max_c = j;
    }
    if (min_r - 2 < 0) {
        min_r = 2;
    }
    if (min_c - 2 < 0) {
        min_c = 2;
    }
    if (max_r + 2 >= Rows) {
        max_r = Rows - 3;
    }
    if (max_c + 2 >= Columns) {
        max_c = Columns - 3;
    }
    array<int, 4> new_restrictions = { min_r, min_c, max_r, max_c };
    return new_restrictions;
}

int get_seq(int y, int e) {
    if (y + e == 0) {
        return 0;
    }
    if (y != 0 && e == 0) {
        return y;
    }
    if (y == 0 && e != 0) {
        return -e;
    }
    if (y != 0 && e != 0) {
        return 17;
    }
}

int evalff(int seq) {
    switch (seq) {
    case 0:
        return 7;
    case 1:
        return 35;
    case 2:
        return 800;
    case 3:
        return 15000;
    case 4:
        return 800000;
    case -1:
        return 15;
    case -2:
        return 400;
    case -3:
        return 1800;
    case -4:
        return 100000;
    case 17:
        return 0;
    }
}
int evaluate_state(int Board[15][15], int player, int hash, array<int, 4> const& restrictions) {
    int black_score = eval_board(Board, -1, restrictions);
    int white_score = eval_board(Board, 1, restrictions);
    int score = 0;
    if (player == -1) {
        score = (black_score - white_score);
    }
    else {
        score = (white_score - black_score);
    }
    StateCache[hash] = score;
    StateCachePuts++;
    return score;
}


int evaluate_direction(array<int, 9> const& direction_arr, int player) {
    int score = 0;
    int arr_size = direction_arr.size();
    for (int i = 0; (i + 4) < arr_size; i++) {
        int you = 0;
        int enemy = 0;
        if (direction_arr[i] == 2) {
            return score;
        }
        for (int j = 0; j <= 4; j++) {
            if (direction_arr[i + j] == 2) {
                return score;
            }
            if (direction_arr[i + j] == player) {
                you++;
            }
            else if (direction_arr[i + j] == -player) {
                enemy++;
            }
        }
        score += evalff(get_seq(you, enemy));
        if ((score >= 800000)) {
            return WIN_DETECTED;
        }
    }
    return score;
}



int evalute_move(int Board[15][15], int x, int y, int player) {
    int score = 0;
    array<array<int, 9>, 4> Directions = get_directions(Board, x, y);
    int temp_score;
    for (int i = 0; i < 4; i++) {
        temp_score = evaluate_direction(Directions[i], player);
        if (temp_score == WIN_DETECTED) {
            return WIN_DETECTED;
        }
        else {
            score += temp_score;
        }
    }
    return score;
}

vector<Move> BoardGenerator(array<int, 4> const& restrictions, int Board[15][15], int player) {
    vector<Move> availSpots_score; //c is j  r is i;
    int  min_r = restrictions[0];
    int min_c = restrictions[1];
    int max_r = restrictions[2];
    int max_c = restrictions[3];
    for (int i = min_r - 2; i <= max_r + 2; i++) {
        for (int j = min_c - 2; j <= max_c + 2; j++) {
            if (Board[i][j] == 0 && !remoteCell(Board, i, j)) {
                Move move;
                move.i = i;
                move.j = j;
                move.score = evalute_move(Board, i, j, player);
                if (move.score == WIN_DETECTED) {
                    vector<Move> winning_move = { move };
                    return winning_move;
                }
                availSpots_score.push_back(move);
            }
        }
    }
    sort(availSpots_score.begin(), availSpots_score.end(), move_sorter);
    //  return availSpots_score.slice(0,20)
    return availSpots_score;
}
int Table[Rows][Columns][2];
mt19937 mt_rand(time(0));
void Table_init() {
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Columns; j++) {
            Table[i][j][0] = mt_rand(); //1
            Table[i][j][1] = mt_rand(); //2
        }
    }
}

int hash_board(int board[15][15]) {
    int h = 0;
    int p;
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Columns; j++) {
            int Board_value = board[i][j];
            if (Board_value != 0) {
                if (Board_value == -1) {
                    p = 0;
                }
                else {
                    p = 1;
                }
                h = h ^ Table[i][j][p];
            }
        }
    }
    return h;
}

int  update_hash(int hash, int player, int row, int col) {
    if (player == -1) {
        player = 0;
    }
    else {
        player = 1;
    }
    hash = hash ^ Table[row][col][player];
    return hash;
}

int negamax(int newBoard[15][15], int player, int depth, int a, int b, int hash, array<int, 4> const& restrictions, int last_i, int last_j) {
    const int alphaOrig = a;
    if ((Cache.count(hash)) && (Cache[hash].depth >= depth)) { //if exists
        CacheHits++;
        int score = Cache[hash].score;
        if (Cache[hash].Flag == 0) {
            CacheCutoffs++;
            return score;
        }
        if (Cache[hash].Flag == -1) {
            a = max(a, score);
        }
        else if (Cache[hash].Flag == 1) {
            b = min(b, score);
        }
        if (a >= b) {
            CacheCutoffs++;
            return score;
        }
    }
    fc++;
    if (checkwin(newBoard, last_i, last_j)) {
        return -2000000 + (MaximumDepth - depth);
    }
    if (depth == 0) {
        if (StateCache.count(hash)) { //if exists
            StateCacheHits++;
            return StateCache[hash];
        }
        return evaluate_state(newBoard, player, hash, restrictions);
    }
    vector<Move> availSpots = BoardGenerator(restrictions, newBoard, player);

    int availSpots_size = availSpots.size();
    if (availSpots_size == 0) {
        return 0;
    }


    int i, j;
    int newHash;
    int bestvalue = numeric_limits<int>::min() + 1;
    int value;
    for (int y = 0; y < availSpots_size; y++) {
        i = availSpots[y].i;
        j = availSpots[y].j;

        newHash = update_hash(hash, player, i, j);
        newBoard[i][j] = player;
        array<int, 4> new_restrictions = Change_restrictions(restrictions, i, j);
        value = -negamax(newBoard, -player, depth - 1, -b, -a, newHash, new_restrictions, i, j);
        newBoard[i][j] = 0;
        if (value > bestvalue) {
            bestvalue = value;
            if (depth == MaximumDepth) {
                bestMove = { i,j,value };
               /* cout << "best move" << endl;
                cout << bestMove.i << " " << bestMove.j << " " << bestMove.score << endl;*/
            }
        }
        a = max(a, value);
        if (a >= b) {
            break;
        }
    }
    //availSpots.clear();
    CachePuts++;
    CacheNode cache_node;

    cache_node.score = bestvalue;
    cache_node.depth = depth;
    if (bestvalue <= alphaOrig) {
        cache_node.Flag = 1;
    }
    else if (bestvalue >= b) {
        cache_node.Flag = -1;
    }
    else {
        cache_node.Flag = 0;
    }
    Cache[hash] = cache_node;
    return bestvalue;
}

Move mtdf(int Board[15][15], int f, int d, array<int, 4> const& restrictions) {
    int g = f;
    int upperbound = numeric_limits<int>::max() - 1;
    int lowerbound = numeric_limits<int>::min() + 1;
    int b;
    Move last_succesful;
    do {
        if (g ==  lowerbound) {
            b = g + 1;
        }
        else {
            b = g;
        }
   /*     if (TIMEOUT()) {
            return "stop";
        }*/
        int result = negamax(Board, 1, d, b - 1, b, hash_board(Board), restrictions, 0, 0);
            /*       if (TIMEOUT()) {
                       return "stop";
                   }*/
            g = result;
        last_succesful = bestMove;

        if (g < b) {
            upperbound = g;
        }
        else {
            lowerbound = g;
        }
    } while (lowerbound < upperbound);
        return last_succesful;
}

Move iterative_mtdf(int depth) {
    int i = 2;
    array<int, 4 > x = { 0, 0, Rows - 1, Columns - 1 };
    int guess = evaluate_state(GameBoard, 1, hash_board(GameBoard), x);
    cout << "Guess for best score: " << guess << endl;
    while (i !=  depth + 2) {
        MaximumDepth = i;
        guess = (mtdf(GameBoard, guess, i, Get_restrictions(GameBoard))).score;
            cout <<"Depth: "<< i << endl;
            cout << "Best Move:" << bestMove.i << " " << bestMove.j << endl;
            cout << "Score: " << bestMove.score << endl;
        //    let t11 = performance.now();
        //console.log((t11 - t00) / 1000)
            if (guess > 1999900) {
                break;
            }
        i += 2;
    }
    return bestMove;
}


int main()
{
    MaximumDepth = 8;
    int depth = 8;

    int player = 1;
    Table_init();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    
    //int res = negamax(GameBoard, player, depth, numeric_limits<int>::min() + 1, numeric_limits<int>::max() - 1, hash_board(GameBoard) - 1, Get_restrictions(GameBoard), 0, 0);
    bestMove = iterative_mtdf(depth);
    Cache.clear();
    StateCache.clear();
    cout << "Best Move:" << bestMove.i << " " << bestMove.j << endl;
    cout << "Score: " << bestMove.score << endl;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    cout << "fc: " << fc << endl;
    cout << "CacheHits: " << CacheHits << endl;
    cout << "CacheCutoffs: " << CacheCutoffs << endl;
    cout << "CachePuts: " << CachePuts << endl;
    cout << "StateCacheHits: " << StateCacheHits << endl;
    cout << "StateCachePuts: " << StateCachePuts << endl;

    system("pause");
    return 0;
}
