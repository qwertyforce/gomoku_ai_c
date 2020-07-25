// CMakeProject1.cpp : Defines the entry point for the application.
//

#include "CMakeProject1.h"
#include <vector>
#include <array>
#include <algorithm>
#include <map> 
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
map<int, int> StateCache;
int cch_pts = 0;
bool move_sorter(Move const& move1, Move const& move2) {
    return move1.score > move2.score;
}

int GameBoard[15][15] = {
    //0 1  2  3  4  5  6  7  8  9  0  1  2  3  4      
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 }, //1
    { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, //2
    { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 }, //3
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //4
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //5
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //6
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //7
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //8
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //9
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //10
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //11
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //12
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //13
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //14
};
const int aiPlayer = 1;
const int huPlayer = -1;
int fc = 0;
const int FiguresToWin = 5;
const int Rows = 15;
const int Columns = 15;
const int WIN_DETECTED = -9999999999;
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


int eval_board(int Board[Rows][Columns], int pieceType, array<int,4> restrictions) {
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
                if (column ==  0 || Board[row][column - 1] !=  0) {
                    block++;
                }
                // pieceNum
                for (column++; column < Columns && Board[row][column] ==  pieceType; column++) {
                    piece++;
                }
                // right
                if (column ==  Columns || Board[row][column] !=  0) {
                    block++;
                }
                score = score +  evaluateblock(block, piece);
            }
        }
    }

    for (int column = min_c; column < max_c + 1; column++) {
        for (int row = min_r; row < max_r + 1; row++) {
            if (Board[row][column] == pieceType) {
                int block = 0;
                int piece = 1;
                // left
                if (row ==  0 || Board[row - 1][column] !=  0) {
                    block++;
                }
                // pieceNum
                for (row++; row < Rows && Board[row][column] ==  pieceType; row++) {
                    piece++;
                }
                // right
                if (row ==  Rows || Board[row][column] !=  0) {
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
                if (Board[r][c] ==  pieceType) {
                    int block = 0;
                    int piece = 1;
                    // left
                    if (c ==  0 || r ==  Rows - 1 || Board[r + 1][c - 1] !=  0) {
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
                    if (r < 0 || c ==  Columns || Board[r][c] !=  0) {
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
                    if (c == 0 || r ==  0 || Board[r - 1][c - 1] !=  0) {
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
                    if (r ==  Rows || c ==  Columns || Board[r][c] !=  0) {
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

bool check_directions(vector<int> arr) {
    int size = arr.size();
    for (int i = 0; i < size - 4; i++) {
        if (arr[i] !=  0) {
            if (arr[i] ==  arr[i+1] && arr[i] ==  arr[i+2] && arr[i] ==  arr[i+3] && arr[i] ==  arr[i+4]) {
                return true;
            }
        }       
    }
    return false;
}
vector <vector<int>> get_directions(int Board[15][15], int x, int y) {
    vector <vector<int>> Directions = { {},{},{},{} };
    for (int i = -4; i < 5; i++) {
        if (x + i >= 0 && x + i <= Rows - 1) {
            Directions[0].push_back(Board[x + i][y]);
                if (y + i >= 0 && y + i <= Columns - 1) {
                    Directions[2].push_back(Board[x + i][y + i]);
                }
        }
        if (y + i >= 0 && y + i <= Columns - 1) {
            Directions[1].push_back(Board[x][y + i]);
                if (x - i >= 0 && x - i <= Rows - 1) {
                    Directions[3].push_back(Board[x - i][y + i]);
                }
        }

    }
    return Directions;
}

bool checkwin(int Board[15][15], int x, int y) {
   vector<vector<int>> Directions = get_directions(Board, x, y);
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
            if (Board[i][j] !=  0) return false;
        }
    }
    return true;
}
array<int,4> Get_restrictions(int Board[15][15]) {
    int min_r = numeric_limits<int>::max();
    int min_c = numeric_limits<int>::max();
    int max_r = numeric_limits<int>::min();
    int max_c = numeric_limits<int>::min();
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Columns; j++) {
            if (Board[i][j] !=  0) {
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
    array<int, 4> myarray;

    array<int,4> restrictions = { min_r, min_c, max_r, max_c };
    return restrictions;
}

auto Change_restrictions(int restrictions[], int i, int j) {
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
    restrictions[0] = min_r;
    restrictions[1] = min_c;
    restrictions[2] = max_r;
    restrictions[3] = max_c;
    return restrictions;
}

int get_seq(int y, int e) {
    if (y + e ==  0) {
        return 0;
    }
    if (y !=  0 && e == 0) {
        return y;
    }
    if (y ==  0 && e != 0) {
        return -e;
    }
    if (y !=  0 && e !=  0) {
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
int evaluate_state(int Board[15][15], int player, int hash,array<int,4> restrictions) {
    int  black_score = eval_board(Board, -1, restrictions);
    int white_score = eval_board(Board, 1, restrictions);
    int score = 0;
    if (player == -1) {
        score = -(black_score - white_score);
    }
    else {
        score = -(white_score - black_score);
    }
    StateCache[hash] = score;
    cch_pts++;
    return score;
}
int evaluate_direction(vector<int> direction_arr, int player) {
    int score = 0;
    int arr_size = direction_arr.size();
    for (int i = 0; (i + 4) < arr_size; i++) {
        int you = 0;
        int enemy = 0;
        for (int j = 0; j <= 4; j++) {
            if (direction_arr[i + j] ==  player) {
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
    vector<vector<int>> Directions = get_directions(Board, x, y);
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

vector<Move> BoardGenerator(array<int,4> restrictions, int Board[15][15], int player) {
    vector<Move> availSpots_score; //c is j  r is i;
    int  min_r = restrictions[0];
    int min_c = restrictions[1];
    int max_r = restrictions[2];
    int max_c = restrictions[3];
    for (int i = min_r - 2; i <= max_r + 2; i++) {
        for (int j = min_c - 2; j <= max_c + 2; j++) {
            if (Board[i][j] ==  0 && !remoteCell(Board, i, j)) {
                Move move;
                move.i = i;
                move.j = j;
                move.score = evalute_move(Board, i, j, player);
                    if (move.score ==  WIN_DETECTED) {
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

int main()
{    
    array<int, 4> asd = Get_restrictions(GameBoard);
    cout << asd << endl;
    int board_ev_score = eval_board(GameBoard, 1, asd);
    cout << board_ev_score << endl;
 /*   auto board = BoardGenerator(asd, GameBoard, 1);
    for (const auto& move : board) {
        cout << move.i << ' ' << move.j << ' '  << move.score << endl;
    }*/

    
    
    
    /*vector<int> abdc = { 1,2,3 };*/
    /*auto abc = get_directions(GameBoard,0,0);
    cout << GameBoard[0][0] << endl;*/

   /* bool res = checkwin(GameBoard, 0, 0);
    cout << res << endl;*/
    //vector<Move> abcde;
    //Move m1;
    //m1.i = 1;
    //m1.j = 1;
    //m1.score = 1;
    //Move m2;
    //m2.i = 1;
    //m2.j = 1;
    //m2.score = 2;
    //abcde.push_back(m1);
    //abcde.push_back(m2);
    //sort(abcde.begin(), abcde.end(), move_sorter);
    //cout << abcde[0].score << endl;

    //for (int i = 0; i < 4; i++) {
    //   
    //    for (int x : abc[i]) {
    //        cout << x << endl;
    //    }
    //}
   

	return 0;
}
