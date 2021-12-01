#ifndef BOT_H
#define BOT_H

#include<vector>
#include "util.h"

class Board;

class Bot {
public:
	MoveResult select_next_move();
	void set_board(Board*);
	~Bot();
	Bot();

	void reset();
	void set_edge_search_limit(int size);
	void set_edge_subset_approximation(bool approximate);

private:
	bool check_queue_empty();
	void single_square_search();
	void edge_search();
	std::vector<std::vector<std::pair<int, int>>*>* get_edges();
	MoveResult guess_random_square();
	int get_adjacent_count(int i, int j);
	double update_probabilities(std::vector<std::pair<int, int>>* edge);
	double update_probabilities_precise(std::vector<std::pair<int, int>>* edge);
	double update_probabilities_sectioned(std::vector<std::pair<int, int>>* edge);
	void free();

	Board* board;
	std::vector<std::pair<int, int>> move_queue;
	int m_rows;
	int m_cols;
	int m_mines;
	double** m_probabilities;
	MoveResult last_result;
	int MAX_SIZE;
	bool edge_subset_approximation;
};

#endif //BOT_H