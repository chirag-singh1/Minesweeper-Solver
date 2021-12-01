#include "board.h"
#include "util.h"
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <stack>
#include <unordered_map>
#include <omp.h>
#include <math.h>

using namespace std;

//Utility method to print out board state
template <class T>
void print(T** arr, int m_rows, int m_cols) {
	for (int i = 0; i < m_rows; i++) {
		for (int j = 0; j < m_cols; j++) {
			cout << '[' << arr[i][j] << ']';
		}
		cout << endl;
	}
}

//Constructs board with random seed
Board::Board(int rows, int columns, int num_mines)
	: m_rows(rows), m_cols(columns), m_mines(num_mines)
{
	srand((unsigned)time(NULL));
	reset_board();
	m_bot.set_board(this);
}

//Constructs board with given specific seed
Board::Board(int rows, int columns, int num_mines, string seed)
	: m_rows(rows), m_cols(columns), m_mines(num_mines)
{
	srand((unsigned)time(NULL));
	reset_board(decompress_seed(seed));
	m_bot.set_board(this);
}

//Cleanup
Board::~Board() {
	for (int i = 0; i < m_rows; i++) {
		delete[] m_board[i];
		delete[] m_counts[i];
		delete[] m_board_display[i];
	}
	delete[] m_board;
	delete[] m_counts;
	delete[] m_board_display;
}

//Print board as-viewable to the player
void Board::print_board() {
	print(m_board_display, m_rows, m_cols);
}

//Print all counts (unused, but useful for debugging)
void Board::print_count() {
	print(m_counts, m_rows, m_cols);
}

//Deal with some kind of user input
bool Board::handle_action(Action* act) {
	if (act->type == USER_DEFINED_MOVE) {
		pair<int, int> p = *((pair<int, int>*) act->info);
		if (p.first >= 0 && p.first < m_rows && p.second >= 0 && p.second < m_cols) {
			make_move(p.first, p.second);
		}
	}
	if (act->type == NEXT_MOVE) {
		if (active) {
			m_bot.select_next_move();
		}
		else {
			return false;
		}
	}
	if (act->type == RESET) {
		free_and_reset();
		m_bot.set_board(this);
	}
	if (act->type == PRINT_COUNTS) {
		print_stats();
	}
	if (act->type == SIMULATE) {
		int temp = *((int*)act->info);
		delete act->info;
		simulate(temp);
	}
	return true;
}

//Make a move on the board
//Returns result of the move
MoveResult Board::make_move(int i, int j) {
	move_count += 1;
	cout << "Making move at " << i << "," << j << endl;
	if (m_board[i][j] == UNREVEALED_MINE || m_board[i][j] == KNOWN_MINE) { //Making move on mine, game lost
		cout << "MINE EXPLODED ON " << i << ", " << j << endl;
		update_mines_as_cross();
		active = false;
		return LOSS;
	}
	stack<pair<int, int>> s;
	s.push(make_pair(i, j));

	while (!s.empty()) { //Search recursively
		pair<int, int> p = s.top();
		s.pop();
		if (m_board[p.first][p.second] != KNOWN_SAFE) { //Set each square to safe
			m_board[p.first][p.second] = KNOWN_SAFE;
			squares_revealed += 1;
			m_board_display[p.first][p.second] = m_counts[p.first][p.second] + '0';
			if (m_counts[p.first][p.second] == 0) { //Append all adjacent squares with no adjacent mines to stack
				execute_callback(this, p.first, p.second, &append_to_stack, &s);
			}
		}
	}

	if (squares_revealed == m_rows * m_cols - m_mines) { //Game won if all safe squares revealed
		cout << "Game won in " << move_count << " moves" << endl;
		cout << "Press R to reset" << endl;
		active = false;
		return WIN;
	}
	return CONTINUE;
}

//Marks a mine as a known mine
//Note that there is no check of whether or not this is accurate, just like in the normal game
void Board::mark_mine(int i, int j) {
	m_board_display[i][j] = 'F';
	m_board[i][j] = KNOWN_MINE;
	mines_marked++;
}

//Accessors
bool Board::is_known(int i, int j) {
	return m_board[i][j] == KNOWN_MINE || m_board[i][j] == KNOWN_SAFE;
}

bool Board::is_safe(int i, int j) {
	return m_board[i][j] == KNOWN_SAFE;
}

bool Board::is_marked_mine(int i, int j) {
	return m_board[i][j] == KNOWN_MINE;
}

bool Board::square_has_state(int i, int j, State* states, int num_states) {
	for (int k = 0; k < num_states; k++) {
		if (m_board[i][j] == states[k]) {
			return true;
		}
	}
	return false;
}

int Board::get_count(int i, int j) {
	return m_counts[i][j];
}

int Board::get_cols() {
	return m_cols;
}

int Board::get_rows() {
	return m_rows;
}

int Board::get_mines() {
	return m_mines;
}

Bot* Board::get_bot() {
	return &m_bot;
}

//Simulate series of games
int Board::simulate(int num_iterations)
{
	cout << "Simulating " << num_iterations << " games" << endl;
	int count = 0;
	for (int i = 0; i < num_iterations; i++) {
		MoveResult res;
		do {
			res = m_bot.select_next_move();
		} while (res == CONTINUE);

		if (res == WIN) {
			count++;
		}
		free_and_reset();
		m_bot.set_board(this);
	}
	return count;
}

//Cleanup board and call to start new game with random seed
void Board::free_and_reset() {
	for (int i = 0; i < m_rows; i++) {
		delete[] m_board[i];
		delete[] m_counts[i];
		delete[] m_board_display[i];
	}
	delete[] m_board;
	delete[] m_counts;
	delete[] m_board_display;
	reset_board();
}

//Generate random seed and call to initialize board with that seed
void Board::reset_board() {
	std::string seed(m_rows * m_cols, '0');

	if (m_mines >= m_rows * m_cols) {
		cout << "Error: bad number of mines" << endl;
		return;
	}

	int count = 0;
	int random;
	while (count < m_mines) {
		random = rand() % (m_rows * m_cols);
		if (random != 0 && seed[random] != '1') {
			seed[random] = '1';
			count += 1;
		}
	}

	reset_board(seed);
}

//Initialize board with given seed
void Board::reset_board(string seed) {
	//Allocate all memory
	m_board = new State * [m_rows];
	m_counts = new int* [m_rows];
	m_board_display = new char* [m_rows];
	for (int i = 0; i < m_rows; i++) {
		m_board[i] = new State[m_cols];
		m_counts[i] = new int[m_cols];
		m_board_display[i] = new char[m_cols];
		for (int j = 0; j < m_cols; j++) {
			m_board[i][j] = UNREVEALED_SAFE;
			m_board_display[i][j] = ' ';
		}
	}

	//Call to initialze board with set
	board_from_seed(seed);
	m_seed = compress_seed(seed);
	cout << "Starting game with seed: " << m_seed << endl;

	//Reset state variables
	m_bot.reset();
	squares_revealed = 0;
	mines_marked = 0;
	move_count = 0;
	active = true;
}

//Sets up initial board state from seed
void Board::board_from_seed(string seed) {
	for (int i = 0; i < seed.length(); i++) {
		if (seed[i] == '1') {
			m_board[i / m_cols][i % m_cols] = UNREVEALED_MINE;
		}
	}

	for (int i = 0; i < m_rows; i++) {
		for (int j = 0; j < m_cols; j++) {
			m_counts[i][j] = 0;
			execute_callback(this, i, j, &count_mines, &m_counts[i][j]);
		}
	}

}

//Compressing seed by runs (sets of consecutive values)
string Board::compress_seed(string seed) {
	string output("");
	if (seed.length() < 1) {
		return output;
	}
	char curr_char = seed[0];
	int curr_start = 0;
	for (int i = 1; i < seed.length(); i++) {
		if(seed[i] != curr_char || i - curr_start == 9) {
			output += curr_char;
			output += '0'+i - curr_start;
			curr_start = i;
			curr_char = seed[i];
		}
	}
	output += curr_char;
	output += seed.length() - curr_start + '0';
	return output;
}

//Decompress string compressed above
string Board::decompress_seed(string seed) {
	string output("");
	if (seed.length() % 2 != 0) {
		return output;
	}
	for (int i = 0; i < seed.length(); i+=2) {
		for (int j = 0; j < seed[i + 1] - '0'; j++) {
			output += seed[i];
		}
	}
	return output;
}

//Set all mines as an X for display
void Board::update_mines_as_cross() {
	for (int i = 0; i < m_rows; i++) {
		for (int j = 0; j < m_cols; j++) {
			if (m_board[i][j] == UNREVEALED_MINE || m_board[i][j] == KNOWN_MINE) {
				m_board_display[i][j] = 'X';
			}
		}
	}
}

//Print out tracked stats of the game
void Board::print_stats() {
	cout << "Marked mines: " << mines_marked << endl;
	cout << "Revealed squares: " << squares_revealed << endl;
	cout << "Unknown squares: " << m_rows * m_cols - squares_revealed - mines_marked<< endl;
	cout << "Mines remaining: " << m_mines - mines_marked << endl;
	cout << "Moves: " << move_count << endl;
}