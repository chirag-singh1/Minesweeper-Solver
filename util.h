#ifndef UTIL_H
#define UTIL_H

#include <unordered_set>
#include <unordered_map>

#define NUM_THREADS 4

using namespace std;
class Board;

enum State { //Potential states for each square
	UNREVEALED_MINE,
	KNOWN_MINE,
	UNREVEALED_SAFE,
	KNOWN_SAFE
};

enum ActionType { //Potential actions to be handled by board
	USER_DEFINED_MOVE,
	NEXT_MOVE,
	RESET,
	PRINT_COUNTS,
	SIMULATE,
};
enum MoveResult { //Results of each move
	WIN,
	CONTINUE,
	LOSS,
};

enum Option { //User-defined options
	ROWS,
	COLUMNS,
	MINES,
	MAX_EDGE_SIZE,
	NO_OPT,
};

struct Action { //Package action with type and flexible instruction pointer
	ActionType type;
	void* info;
};

struct PairHashStruct { //Hash function for pair of integers
	inline size_t operator()(const pair<int, int>& p) const
	{
		return ((p.first + p.second) * (p.first + p.second + 1) / 2) + p.second;
	}
};

struct AdjacencyOrderingNode { //Node for edge subset tree
	pair<int, int> coord;
	unordered_set<pair<int, int>, PairHashStruct> current_squares;
	unordered_set<pair<int, int>, PairHashStruct> visited;
	vector<AdjacencyOrderingNode*> children;
	AdjacencyOrderingNode* parent;
};
struct MapStruct { //Package two maps for use with callback format
	unordered_set<pair<int, int>, PairHashStruct>* set;
	unordered_map<pair<int, int>, int, PairHashStruct>* map;
};

const static int DIRECTIONS[8][2]{ //Helper array for callbacks
	{0,1},
	{0,-1},
	{1,0},
	{-1,0},
	{1,1},
	{-1,1},
	{1,-1},
	{-1,-1}
};

//Hash functions
int hash_pair(unordered_set<pair<int, int>, PairHashStruct>::iterator p);
int hash_set(unordered_set<pair<int, int>, PairHashStruct> set);

//General utility functions
void execute_callback(Board* b, int i, int j, void (*callback)(int i, int j, Board*, void* arg), void* arg);
bool is_subset(const unordered_set<pair<int, int>, PairHashStruct> set, const unordered_set<pair<int, int>, PairHashStruct> sub);
void mark_as_known_mine(int i, int j, Board* board, void* p);

//Counting functions
void count_unknown_spaces(int i, int j, Board* board, void* count);
void count_known_mines(int i, int j, Board* board, void* count);
void count_mines(int i, int j, Board* board, void* count);
void decrement_count(int i, int j, Board* board, void* map);

//Appending functions
void append_to_stack(int i, int j, Board* board, void* st);
void append_to_vector(int i, int j, Board* board, void* vt);
void append_known_to_vector(int i, int j, Board* board, void* vt);
void insert_into_map(int i, int j, Board* board, void* map);
void build_adjacency_set(int i, int j, Board* board, void* map);
void build_interior_interior_set(int i, int j, Board* board, void* maps);

#endif