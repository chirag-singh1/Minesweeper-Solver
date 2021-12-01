#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <vector>
#include <unordered_set>
#include "bot.h"

enum State;
enum MoveResult;
struct Action;
struct SetHashStruct;
struct PairHashStruct;

class Board {
public:
	Board(int rows, int columns, int num_mines);
	Board(int rows, int columns, int num_mines, std::string seed);
	~Board();
	void print_board();
	void print_count();
	bool handle_action(Action* act);

	bool square_has_state(int i, int j, State* states, int num_states);
	int get_count(int i, int j);
	void mark_mine(int i, int j);
	MoveResult make_move(int i, int j);

	bool is_known(int i, int j);
	bool is_safe(int i, int j);
	bool is_marked_mine(int i, int j);

	int get_rows();
	int get_cols();
	int get_mines();

	int simulate(int num_iterations);
	Bot* get_bot();

private:
	void free_and_reset();
	void reset_board();
	void reset_board(std::string seed);
	void board_from_seed(std::string seed);
	void update_mines_as_cross();
	void print_stats();

	std::string compress_seed(std::string seed);
	std::string decompress_seed(std::string seed);

	int m_rows;
	int m_cols;
	int m_mines;
	std::string m_seed;
	State** m_board;
	int** m_counts;
	char** m_board_display;

	int mines_marked;
	int squares_revealed;
	int move_count;
	bool active;

	Bot m_bot;

};

#endif //BOARD_H