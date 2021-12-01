#ifndef BOT_H
#define BOT_H

#include<vector>
#include "util.h"

class Board;

class Bot {
public:
	//Initialization and cleanup
	void reset();
	~Bot();
	Bot();

	//Set bot characteristics
	void set_board(Board*);
	void set_edge_search_limit(int size);
	void set_edge_subset_approximation(bool approximate);

	//Key method: select next move
	MoveResult select_next_move();

private:
	//Cleanup
	void free();

	//General logical methods
	bool check_queue_empty();
	void single_square_search();
	MoveResult guess_random_square();

	//Edge search methods
	void edge_search();
	std::vector<std::vector<std::pair<int, int>>*>* get_edges();
	double update_probabilities(std::vector<std::pair<int, int>>* edge);
	double update_probabilities_precise(std::vector<std::pair<int, int>>* edge);
	double update_probabilities_sectioned(std::vector<std::pair<int, int>>* edge);

	//Board and board info
	Board* board;
	int m_rows;
	int m_cols;
	int m_mines;

	//Settings
	int MAX_SIZE;
	bool edge_subset_approximation;

	//State variables
	std::vector<std::pair<int, int>> move_queue;
	double** m_probabilities;
	MoveResult last_result;
};

#endif //BOT_H