#ifndef UTIL_H
#define UTIL_H

#include <unordered_set>
#include <unordered_map>

#define NUM_THREADS 4

using namespace std;
class Board;

enum State {
	UNREVEALED_MINE,
	KNOWN_MINE,
	UNREVEALED_SAFE,
	KNOWN_SAFE
};

enum ActionType {
	USER_DEFINED_MOVE,
	NEXT_MOVE,
	RESET,
	PRINT_COUNTS,
};
enum MoveResult {
	WIN,
	CONTINUE,
	LOSS,
};

enum Option {
	ROWS,
	COLUMNS,
	MINES,
	MAX_EDGE_SIZE,
	NO_OPT,
};

struct Action {
	ActionType type;
	void* info;
};

struct PairHashStruct {
	inline size_t operator()(const pair<int, int>& p) const
	{
		return ((p.first + p.second) * (p.first + p.second + 1) / 2) + p.second;
	}
};

struct AdjacencyOrderingNode {
	pair<int, int> coord;
	unordered_set<pair<int, int>, PairHashStruct> current_squares;
	unordered_set<pair<int, int>, PairHashStruct> visited;
	vector<AdjacencyOrderingNode*> children;
	AdjacencyOrderingNode* parent;
};
struct MapStruct {
	unordered_set<pair<int, int>, PairHashStruct>* set;
	unordered_map<pair<int, int>, int, PairHashStruct>* map;
};

const static int DIRECTIONS[8][2]{
	{0,1},
	{0,-1},
	{1,0},
	{-1,0},
	{1,1},
	{-1,1},
	{1,-1},
	{-1,-1}
};

int hash_pair(unordered_set<pair<int, int>, PairHashStruct>::iterator p);
int hash_pair(unordered_set<pair<int, int>, PairHashStruct>::iterator p);
int hash_set(unordered_set<pair<int, int>, PairHashStruct> set);

bool is_subset(const unordered_set<pair<int, int>, PairHashStruct> set, const unordered_set<pair<int, int>, PairHashStruct> sub);
void execute_callback(Board* b, int i, int j, void (*callback)(int i, int j, Board*, void* arg), void* arg);

void mark_as_known_mine(int i, int j, Board* board, void* p);

void count_unknown_spaces(int i, int j, Board* board, void* count);

void count_known_mines(int i, int j, Board* board, void* count);

void count_mines(int i, int j, Board* board, void* count);

void append_to_stack(int i, int j, Board* board, void* st);

void append_to_vector(int i, int j, Board* board, void* vt);

void append_known_to_vector(int i, int j, Board* board, void* vt);

void insert_into_map(int i, int j, Board* board, void* map);
void decrement_count(int i, int j, Board* board, void* map);

void build_adjacency_set(int i, int j, Board* board, void* map);

void build_interior_interior_set(int i, int j, Board* board, void* maps);

void count_flags(int i, int j, Board* board, void* c);

template <class T>
void print(T** arr, int m_rows, int m_cols);

#endif