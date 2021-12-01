#include <iostream>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include "board.h"
#include "util.h"

int hash_pair(unordered_set<pair<int, int>, PairHashStruct>::iterator p) {
	return ((p->first + p->second) * (p->first + p->second + 1) / 2) + p->second;
}

int hash_set(unordered_set<pair<int, int>, PairHashStruct> set)
{
	int tot = 0;
	for (unordered_set<pair<int, int>, PairHashStruct>::iterator p = set.begin(); p != set.end(); p++) {
		tot ^= hash_pair(p);
	}
	return tot;
}

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

void execute_callback(Board* b, int i, int j, void (*callback)(int i, int j, Board*, void* arg), void* arg) {
	for (int k = 0; k < 8; k++) {
		if (i + DIRECTIONS[k][0] >= 0 && i + DIRECTIONS[k][0] < b->get_rows() && j + DIRECTIONS[k][1] >= 0 && j + DIRECTIONS[k][1] < b->get_cols()) {
			callback(i + DIRECTIONS[k][0], j + DIRECTIONS[k][1], b, arg);
		}
	}
}

void mark_as_known_mine(int i, int j, Board* board, void* p) {
	if (!board->is_known(i, j)) {
		board->mark_mine(i, j);
	}
}

void count_unknown_spaces(int i, int j, Board* board, void* count) {
	int* c = (int*)count;
	if (!board->is_known(i, j)) {
		*c += 1;
	}
}

void count_known_mines(int i, int j, Board* board, void* count) {
	int* c = (int*)count;
	if (board->is_marked_mine(i, j)) {
		*c += 1;
	}
}

void count_mines(int i, int j, Board* board, void* count) {
	int* c = (int*)count;
	State mines[2]{ UNREVEALED_MINE, KNOWN_MINE };
	if (board->square_has_state(i, j, mines, 2)) {
		*c += 1;
	}
}

void append_to_stack(int i, int j, Board* board, void* st) {
	if (!board->is_safe(i, j)) {
		stack<pair<int, int>>* s = (stack<pair<int, int>>*)st;
		s->push(make_pair(i, j));
	}
}

void append_to_vector(int i, int j, Board* board, void* vt) {
	vector<pair<int, int>>* v = (vector<pair<int, int>>*) vt;
	if (!board->is_known(i, j)) {
		v->push_back(make_pair(i, j));
	}
}

void append_known_to_vector(int i, int j, Board* board, void* vt) {
	vector<pair<int, int>>* v = (vector<pair<int, int>>*) vt;
	if (board->is_known(i, j)) {
		v->push_back(make_pair(i, j));
	}
}

void insert_into_map(int i, int j, Board* board, void* map) {
	unordered_map<pair<int, int>, int, PairHashStruct>* m = (unordered_map<pair<int, int>, int, PairHashStruct>*) map;
	if (board->is_safe(i, j)) {
		(*m)[make_pair(i, j)] = board->get_count(i, j);
	}
}

void decrement_count(int i, int j, Board* board, void* map) {
	unordered_map<pair<int, int>, int, PairHashStruct>* m = (unordered_map<pair<int, int>, int, PairHashStruct>*) map;
	if (board->is_safe(i, j)) {
		(*m)[make_pair(i, j)] -= 1;
	}
}

void build_adjacency_set(int i, int j, Board* board, void* map) {
	unordered_set<pair<int, int>, PairHashStruct>* m = (unordered_set<pair<int, int>, PairHashStruct>*) map;
	if (!board->is_known(i, j)) {
		(*m).insert(make_pair(i, j));
	}
}

void build_interior_interior_set(int i, int j, Board* board, void* maps) {
	MapStruct* m = (MapStruct*)maps;
	if (board->is_known(i, j) && m->map->find(make_pair(i, j)) != m->map->end()) {
		m->set->insert(make_pair(i, j));
	}

}

void count_flags(int i, int j, Board* board, void* c) {
	int* count = (int*)c;
	if (board->is_marked_mine(i, j)) {
		*count += 1;
	}
}