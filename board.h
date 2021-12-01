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
	//Constructors and destructor
	Board(int rows, int columns, int num_mines);
	Board(int rows, int columns, int num_mines, std::string seed);
	~Board();

	//Print board to terminal
	void print_board();
	void print_count();

	//Board behavior
	bool handle_action(Action* act);
	MoveResult make_move(int i, int j);
	void mark_mine(int i, int j);

	//Square queries (safe for bot/player)
	bool is_known(int i, int j);
	bool is_safe(int i, int j);
	bool is_marked_mine(int i, int j);
	bool square_has_state(int i, int j, State* states, int num_states);
	int get_count(int i, int j);

	//Board accessor methods
	int get_rows();
	int get_cols();
	int get_mines();
	Bot* get_bot();

private:
	//Various initialization and cleanup methods
	void free_and_reset();
	void reset_board();
	void reset_board(std::string seed);
	void board_from_seed(std::string seed);
	std::string compress_seed(std::string seed);
	std::string decompress_seed(std::string seed);

	//Various methods
	void update_mines_as_cross();
	void print_stats();
	int simulate(int num_iterations);

	//Board setup values
	int m_rows;
	int m_cols;
	int m_mines;
	std::string m_seed;

	//Board state
	State** m_board;
	int** m_counts;
	char** m_board_display;

	//Board state
	int mines_marked;
	int squares_revealed;
	int move_count;
	bool active;
	Bot m_bot;
};

#endif //BOARD_H