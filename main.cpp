// minesweeper.cpp : Defines the entry point for the application.
//

#include <iostream>
#include "board.h"
#include "bot.h"
#include "util.h"
#include <string>
#include <string.h>
#include <ctype.h>
#include <csignal>

using namespace std;

Action* act;
Board* b;

void signal_handler(int signum) {
	cout << "Interrupt signal received, cleaning up" << endl;

	if (act != nullptr) {
		if (act->info != nullptr) {
			delete act->info;
		}
		delete act;
		if (b != nullptr) {
			delete b;
		}
	}
	cout << "Cleanup complete, exiting" << endl;
	exit(0);
}

bool parse_coords(std::string in, Action* act) {
	int count = 0;
	std::string str_1("");
	std::string str_2("");
	for (int i = 0; i < in.length(); i++) {
		if (!(isdigit(in[i]) || (count == 0 && in[i] == ' '))) {
			return false;
		}
		if (count == 0) {
			if (in[i] == ' ') {
				count += 1;
			}
			else {
				str_1 += in[i];
			}
		}
		else {
			str_2 += in[i];
		}
	}
	act->type = USER_DEFINED_MOVE;
	pair<int, int>* p = new pair<int, int>;
	p->first = stoi(str_1);
	p->second = stoi(str_2);
	act->info = p;
	return true;
}

bool parse_input(std::string in, Action* act) {
	if (act->info != nullptr) {
		delete act->info;
		act->info = nullptr;
	}
	if (in.length() == 0) {
		act->type = NEXT_MOVE;
		act->info = nullptr;
		return true;
	}
	if (parse_coords(in, act)) {
		return true;
	}
	if (in == "n" || in == "next") {
		act->type = NEXT_MOVE;
		act->info = nullptr;
		return true;
	}
	if (in == "r" || in == "reset") {
		act->type = RESET;
		act->info = nullptr;
		return true;
	}
	if (in == "s" || in == "stats") {
		act->type = PRINT_COUNTS;
		act->info = nullptr;
		return true;
	}
	return false;
}

void set_value(char* arg, int &val) {
	try {
		std::string s = arg;
		val = stoi(arg);
	}
	catch (const std::exception& e) {
		cout << "Invalid argument" << endl;
		exit(EINVAL);
	}
}

int main(int argc, char** argv)
{
	signal(SIGINT, signal_handler);
	int rows=9;
	int cols=9;
	int mines=10;
	int max_edge_size=10;
	bool subset_approximation = true;
	string seed;
	Option curr_option = NO_OPT;
	for (int i = 1; i < argc; i++) {		
		if (curr_option != NO_OPT) {
			if (curr_option == ROWS) {
				set_value(argv[i], rows);
			}
			else if (curr_option == COLUMNS) {
				set_value(argv[i], cols);
			}
			else if (curr_option == MINES) {
				set_value(argv[i], mines);
			}
			else if (curr_option == MAX_EDGE_SIZE) {
				set_value(argv[i], max_edge_size);
			}
			curr_option = NO_OPT;
		}
		else {
			if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--rows") == 0) {
				curr_option = ROWS;
			}
			else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--columns") == 0) {
				curr_option = COLUMNS;
			}
			else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mines") == 0) {
				curr_option = MINES;
			}
			else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--edge_size") == 0) {
				curr_option = MAX_EDGE_SIZE;
			}
			else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--disable_subset_approximations") == 0) {
				subset_approximation = false;
			}
			else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i], "--help") == 0) {
				cout << "Start the minesweeper solver." << endl;
				cout << "Options:" << endl;
				cout << "	--rows (-r) [int]: Set the number of rows" << endl;
				cout << "	--cols (-m) [int]: Set the number of columns" << endl;
				cout << "	--mines (-e) [int]: Set the number of mines" << endl;
				cout << "	--edge_size (-s) [int]: Set the maximum number of squares searched without approximation" << endl;
				cout << "	--disable_subset_approximations (-d): Disable subset approximation for large edges" << endl;
				cout << "Commands:" << endl;
				cout << "\tnext (n, enter): Play the next best move" << endl;
				cout << "\treset (r): Start a new game" << endl;
				cout << "\tquit (q): Close the program" << endl;
				cout << "\tstats (s): View relevant stats" << endl;
				cout << "\t[int] [int]: Make the move manually at the specified square" << endl;
				return 0;
			}
			else if (strchr(argv[i], 'x') != NULL || strchr(argv[i], 'X') != NULL) {
				int j = 0;
				while (argv[i][j] != 'x' && argv[i][j] != 'X') {
					j++;
				}
				try {
					std::string arg = argv[i];
					rows = stoi(arg.substr(0, j));
					cols = stoi(arg.substr(j + 1, arg.length()-j));
				}
				catch (const std::exception& e) {
					cout << "Invalid row and column value" << endl;
					return EINVAL;
				}
			}
			else {
				cout << "Invalid argument " << argv[i] << endl;
				cout << "Run with -h flag for help" << endl;
				return EINVAL;
			}
		}
	}

	if (subset_approximation) {
		cout << "Initializing board with " << rows << " rows, " << cols << " columns, " << mines << " mines, maximum edge size of " << max_edge_size << " and subset approximation enabled" << endl;
	}
	else {
		cout << "Initializing board with " << rows << " rows, " << cols << " columns, " << mines << " mines, maximum edge size of " << max_edge_size<< " and subset approximation disabled" << endl;
	}
	
	if (seed.length() > 0) {
		b = new Board(rows, cols, mines, seed);
	}
	else {
		b = new Board(rows, cols, mines);
	}
	b->get_bot()->set_edge_search_limit(max_edge_size);
	b->get_bot()->set_edge_subset_approximation(subset_approximation);
		
	std::string user_in;
	act = new Action;
	act->info = nullptr;
	b->print_board();
	while (user_in != "quit" && user_in != "q") {
		getline(cin, user_in);
		if (parse_input(user_in, act)) {
			if (b->handle_action(act)) {
				b->print_board();
			}
		}
		else {
			cout << "Invalid input, try inputting \"help\" for help" << endl;
		}
	}
	if (act->info != nullptr) {
		delete act->info;
	}
	delete act;
	delete b;
	cout << "Cleanup done" << endl;
	return 0;
}
