#include <iostream>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include "board.h"
#include "util.h"

//Hashes pair of integers
int hash_pair(unordered_set<pair<int, int>, PairHashStruct>::iterator p) {
	return ((p->first + p->second) * (p->first + p->second + 1) / 2) + p->second;
}

//Hashes set of pair of integers (linear time with length of set)
int hash_set(unordered_set<pair<int, int>, PairHashStruct> set)
{
	int tot = 0;
	for (unordered_set<pair<int, int>, PairHashStruct>::iterator p = set.begin(); p != set.end(); p++) {
		tot ^= hash_pair(p);
	}
	return tot;
}

//Calculates if a set is a subset of another (linear time)
bool is_subset(const unordered_set<pair<int, int>, PairHashStruct> set, const unordered_set<pair<int, int>, PairHashStruct> sub) {
	if (set.size() < sub.size()) {
		return false;
	}
	for (pair<int, int> p : sub) {
		if (set.find(p) == set.end()) {
			return false;
		}
	}
	return true;
}

//Executes a function on each square adjacent to a specific square
//Provides void* arg as an arbitrary pointer to be used by the callback as necessary
void execute_callback(Board* b, int i, int j, void (*callback)(int i, int j, Board*, void* arg), void* arg) {
	for (int k = 0; k < 8; k++) {
		if (i + DIRECTIONS[k][0] >= 0 && i + DIRECTIONS[k][0] < b->get_rows() && j + DIRECTIONS[k][1] >= 0 && j + DIRECTIONS[k][1] < b->get_cols()) {
			callback(i + DIRECTIONS[k][0], j + DIRECTIONS[k][1], b, arg);
		}
	}
}

//Marks all squares adjacent to a specific square as a known mine
void mark_as_known_mine(int i, int j, Board* board, void* p) {
	if (!board->is_known(i, j)) {
		board->mark_mine(i, j);
	}
}

//Counts all adjacent squares with unknown state
void count_unknown_spaces(int i, int j, Board* board, void* count) {
	int* c = (int*)count;
	if (!board->is_known(i, j)) {
		*c += 1;
	}
}

//Counts all adjacent squares known to be mines
void count_known_mines(int i, int j, Board* board, void* count) {
	int* c = (int*)count;
	if (board->is_marked_mine(i, j)) {
		*c += 1;
	}
}

//Counts total number of adjacent mines (revealed and unrevealed) adjacent to a square
//Not used by bot
void count_mines(int i, int j, Board* board, void* count) {
	int* c = (int*)count;
	State mines[2]{ UNREVEALED_MINE, KNOWN_MINE };
	if (board->square_has_state(i, j, mines, 2)) {
		*c += 1;
	}
}

//Decrements count for each adjacent safe square
void decrement_count(int i, int j, Board* board, void* map) {
	unordered_map<pair<int, int>, int, PairHashStruct>* m = (unordered_map<pair<int, int>, int, PairHashStruct>*) map;
	if (board->is_safe(i, j)) {
		(*m)[make_pair(i, j)] -= 1;
	}
}

//Pushes all adjacent safe squares to stack
void append_to_stack(int i, int j, Board* board, void* st) {
	if (!board->is_safe(i, j)) {
		stack<pair<int, int>>* s = (stack<pair<int, int>>*)st;
		s->push(make_pair(i, j));
	}
}

//Appends all adjacent unknown squares to vector
void append_to_vector(int i, int j, Board* board, void* vt) {
	vector<pair<int, int>>* v = (vector<pair<int, int>>*) vt;
	if (!board->is_known(i, j)) {
		v->push_back(make_pair(i, j));
	}
}

//Appends all adjacent known squares to vector
void append_known_to_vector(int i, int j, Board* board, void* vt) {
	vector<pair<int, int>>* v = (vector<pair<int, int>>*) vt;
	if (board->is_known(i, j)) {
		v->push_back(make_pair(i, j));
	}
}

//Inserts all adjacent squares into map as key value pair of pair to number of adjacent mines
void insert_into_map(int i, int j, Board* board, void* map) {
	unordered_map<pair<int, int>, int, PairHashStruct>* m = (unordered_map<pair<int, int>, int, PairHashStruct>*) map;
	if (board->is_safe(i, j)) {
		(*m)[make_pair(i, j)] = board->get_count(i, j);
	}
}

//Inserts all adjacent unknown squares to set
void build_adjacency_set(int i, int j, Board* board, void* map) {
	unordered_set<pair<int, int>, PairHashStruct>* m = (unordered_set<pair<int, int>, PairHashStruct>*) map;
	if (!board->is_known(i, j)) {
		(*m).insert(make_pair(i, j));
	}
}

//Inserts all adjacent known squares to set if they are not in the map keyset
void build_interior_interior_set(int i, int j, Board* board, void* maps) {
	MapStruct* m = (MapStruct*)maps;
	if (board->is_known(i, j) && m->map->find(make_pair(i, j)) != m->map->end()) {
		m->set->insert(make_pair(i, j));
	}

}