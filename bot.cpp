#include "bot.h"
#include "board.h"
#include "util.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <vector>
#include <math.h>

using namespace std;

//Reset bot to initial state
void Bot::reset() {
	if (m_probabilities != nullptr) {
		free();
	}
	last_result = CONTINUE;
	move_queue.clear();
}

//Constructor for default values
Bot::Bot(){
	m_probabilities = nullptr;
	MAX_SIZE = 10;
}

//Destructor
Bot::~Bot() {
	free();
}

//Set board pointer, copy frequently accessed values to this object
void Bot::set_board(Board* b) {
	board = b;
	m_rows = b->get_rows();
	m_cols = b->get_cols();
	m_mines = b->get_mines();

	m_probabilities = new double* [m_rows];
	for (int i = 0; i < m_rows; i++) {
		m_probabilities[i] = new double[m_cols];
	}
}

//Set maximum edge length
void Bot::set_edge_search_limit(int size) {
	MAX_SIZE = size;
}

//Enable/disable approximation
void Bot::set_edge_subset_approximation(bool approximate) {
	edge_subset_approximation = approximate;
}

//Main method: search for the next optimal move
MoveResult Bot::select_next_move() {
	if (check_queue_empty()) return last_result; //See if existing safe move exists
	cout << "No existing move in queue, beginning single square search" << endl;
	single_square_search(); //Search for safe move/mark flags with single square information
	if (check_queue_empty()) return last_result;
	cout << "No single square found, beginning edge search" << endl;
	edge_search(); //Use edge-based search 
	if (check_queue_empty()) return last_result;
	cout << "Guessing" << endl;
	return guess_random_square(); //Guess based on probabilities/corner-edge heuristic
}

//Free all memory for probability array
void Bot::free() {
	for (int i = 0; i < m_rows; i++) {
		delete[] m_probabilities[i];
	}
	delete[] m_probabilities;
}

//Check if there are any safe moves queued, making the move if it exists
//This allows us to search in batches, using the safe moves from each batch over several moves
bool Bot::check_queue_empty() {
	while (!move_queue.empty()) {
		pair<int, int> p = move_queue.at(0);
		move_queue.erase(move_queue.begin());
		if (!board->is_known(p.first, p.second)) {
			cout << "Safe move found" << endl;
			last_result = board->make_move(p.first, p.second);
			return true;
		}
	}
	return false;
}

//Search for safe squares and mines using only those square's constraints
//Extremely effective when large edges are revealed at decreasing the frequency of expensive edge searches
void Bot::single_square_search() {
	for (int i = 0; i < m_rows; i++) { //Iterate over each square
		for (int j = 0; j < m_cols; j++) {
			if (board->is_safe(i, j)) {
				int known_mines = 0;
				int open_spaces = 0;
				execute_callback(board, i, j, &count_known_mines, &known_mines); //Count number of known mines adjacent to square
				execute_callback(board, i, j, &count_unknown_spaces, &open_spaces); //Count number of open spaces adjacent to square

				if (board->get_count(i, j) == open_spaces + known_mines) { //Each open square is a mine, mark them
					execute_callback(board, i, j, &mark_as_known_mine, nullptr);
				}
				if (board->get_count(i, j) == known_mines) { //No possible mines, square is safe so add to queue
					execute_callback(board, i, j, &append_to_vector, &move_queue);
				}
			}
		}
	}
}

//Guesses best probability move available
//Probabilities are not updated each move, but will occur before any guess (due to edge search)
//Uses heuristic of being closest to the edges/corners to try to avoid 50/50 guesses near end of game
MoveResult Bot::guess_random_square() {
	int top = 0, bottom = m_rows - 1, left = 0, right = m_cols - 1;
	int dir = 0;
	double min_probability = -1.0;
	pair<int, int> best_guess;

	//Spiral traversal
	while (top <= bottom && left <= right) {
		if (dir == 0) {
			for (int i = left; i <= right; i++) { //left to right
				if (!board->is_known(top, i) && (min_probability < 0 || m_probabilities[top][i] < min_probability)) {
					best_guess = make_pair(top, i);
					min_probability = m_probabilities[top][i];
				}
			}
			top++;
		}
		else if (dir == 1) {
			for (int i = top; i <= bottom; i++) { //Top to bottom
				if (!board->is_known(i, right) && (min_probability < 0 || m_probabilities[i][right] < min_probability)) {
					best_guess = make_pair(i, right);
					min_probability = m_probabilities[i][right];
				}
			}
			right -= 1;
		}
		else if (dir == 2) {
			for (int i = right; i >= left; i--) { //Right to left 
				if (!board->is_known(bottom, i) && (min_probability < 0 || m_probabilities[bottom][i] < min_probability)) {
					best_guess = make_pair(bottom, i);
					min_probability = m_probabilities[bottom][i];
				}
			}
			bottom -= 1;
		}
		else {
			for (int i = bottom; i >= top; i--) { //Left to top
				if (!board->is_known(i, left) && (min_probability < 0 || m_probabilities[i][left] < min_probability)) {
					best_guess = make_pair(i, left);
					min_probability = m_probabilities[i][left];
				}
			}
			left += 1;
		}

		dir = (dir + 1) % 4;
	}
	cout << "Best probability move: " << (1 - min_probability) * 100 << "%" << endl;
	return board->make_move(best_guess.first, best_guess.second);
}

//Indepth search of each edge for all possibilities with given constraints
//Will select best probability move and find any safe squares/guaranteed mines
//May approximate for long edges
void Bot::edge_search() {
	vector<vector<pair<int, int>>*>* edges = get_edges();

	double count_tot = 0;
	double count_known = 0;
	double count_edge = 0;
	for (int i = 0; i < m_rows; i++) { //Count number of unknown squares and number of marked mines
		for (int j = 0; j < m_cols; j++) {
			if (board->is_marked_mine(i, j)) {
				count_known += 1;
			}
			else if (!board->is_known(i, j)) {
				count_tot += 1;
			}
		}
	}
	for (int i = 0; i < edges->size(); i++) { //Count total number of squares in edges
		count_edge += (*edges)[i]->size();
	}

	double edge_mines = 0;
	for (int i = 0; i < edges->size(); i++) { //Update probabilities of each edge
		edge_mines += update_probabilities((*edges)[i]);
	}

	unordered_set<pair<int, int>, PairHashStruct> in_edges;
	for (vector<pair<int, int>>* v : *edges) { //Copy edges to set for faster search
		for (pair<int, int> p : *v) {
			in_edges.insert(p);
		}
	}

	for (int i = 0; i < m_rows; i++) { //Set all probabilities to probability of non-edge mine
		for (int j = 0; j < m_cols; j++) {
			if (in_edges.find(make_pair(i, j)) == in_edges.end()) {
				if (m_mines - count_known - edge_mines == count_tot - count_edge) { //More primitive approximation
					m_probabilities[i][j] = (m_mines - count_known) / (count_tot);
				}
				else { //Better approximation
					m_probabilities[i][j] = (m_mines - count_known - edge_mines) / (count_tot - count_edge);
				}
			}
		}
	}

	for (int i = 0; i < m_rows; i++) { //Mark all known mines, add all safe edges
		for (int j = 0; j < m_cols; j++) {
			if (m_probabilities[i][j] == 0.0) {
				move_queue.push_back(make_pair(i, j));
			}
			if (m_probabilities[i][j] == 1.0 && !board->is_known(i, j)) {
				board->mark_mine(i, j);
			}
		}
	}

	while (!edges->empty()) { //Cleanup
		delete edges->back();
		edges->pop_back();
	}
	delete edges;
}

//Returns vector of vector of pairs, each vector of pairs representing an edge
//In this context, an edge is any set of unknown squares sharing a common set of constraints
vector<vector<pair<int, int>>*>* Bot::get_edges() { //Returns vector of edges (each edge is a vector of pairs representing a square along that edge)
	unordered_set<pair<int, int>, PairHashStruct> visited;
	vector<vector<pair<int, int>>*>* vec = new vector<vector<pair<int, int>>*>;
	vector<pair<int, int>> search_queue;

	for (int i = 0; i < m_rows; i++) {
		for (int j = 0; j < m_cols; j++) { //Iterate over each square
			pair<int, int> p = make_pair(i, j);
			if (visited.find(p) == visited.end() && !board->is_known(i, j)) { //Only consider non-visited and unknown squares
				search_queue.clear();
				search_queue.push_back(p);
				vector<pair<int, int>>* edge = new vector<pair<int, int>>;
				while (!search_queue.empty()) { //Loop through search queue
					pair<int, int> t = search_queue.at(0); //Pop first square in queue
					search_queue.erase(search_queue.begin());
					if (visited.find(make_pair(t.first, t.second)) == visited.end()) { //Check that square hasn't been put in current edge already
						vector<pair<int, int>> v;
						execute_callback(board, t.first, t.second, &append_known_to_vector, &v);
						if (v.size() != 0) {
							for (vector<pair<int, int>>::iterator itr = v.begin(); itr != v.end(); itr++) { //Copy adjacent squares into search queue
								if (visited.find(make_pair(itr->first, itr->second)) == visited.end()) {
									execute_callback(board, itr->first, itr->second, &append_to_vector, &search_queue);
								}
								visited.insert(make_pair(itr->first, itr->second));
							}
							edge->push_back(t);
						}
						visited.insert(t); //Mark each visited square
					}
				}
				if (!edge->empty()) { //Add edge to list of edges
					vec->push_back(edge);
				}
				else {
					delete edge;
				}
				visited.insert(p); //Mark as visited
			}
		}
	}

	return vec;
}

//Redirect call to update probabilities to appropriate method
double Bot::update_probabilities(vector<pair<int, int>>* edge) {
	cout << "Updating probability for edge: ";
	for (pair<int, int> p : *edge) {
		cout << "(" << p.first << ", " << p.second << ")";
	}
	cout << endl;

	if (edge_subset_approximation && edge->size() >= MAX_SIZE) {
		cout << "Edge too large, using subset edge search" << endl;
		return update_probabilities_sectioned(edge);
	}
	else {
		return update_probabilities_precise(edge);
	}

}

//Brute force algorithm for calculating edge probabilities
//Guaranteed optimal results, but runs in exponential time and struggles with large enough edges
double Bot::update_probabilities_precise(vector<pair<int, int>>* edge) { //Precisely calculates probabilities for small edges
	int* good_count = new int[NUM_THREADS * edge->size()];
	int success_count[NUM_THREADS] = { 0 };
	int flag_count = 0;
	double mine_count = 0;

	unordered_map<pair<int, int>, int, PairHashStruct> adjacent_counts; //For each edge square, store number of adjacent squares (known squares) bordering the edge 
	for (int i = 0; i < edge->size(); i++) {
		pair<int, int> p = (*edge)[i];
		execute_callback(board, p.first, p.second, &insert_into_map, &adjacent_counts);
	}

	for (pair<pair<int, int>, int> p : adjacent_counts) { //For each adjacent square, count the number of mines in the edge
		flag_count = 0;
		execute_callback(board, p.first.first, p.first.second, &count_known_mines, &flag_count);
		adjacent_counts[p.first] -= flag_count;
	}

	int count_possibilities = 0;

	for (int i = 0; i < edge->size(); i++) { //Reset probabilities of edge squares
		pair<int, int> p = (*edge)[i];
		m_probabilities[p.first][p.second] = 0;
	}

	for (int i = 0; i < (int)pow(2, edge->size()); i++) { //Check each possibility (exponential time)
		unordered_map<pair<int, int>, int, PairHashStruct> possibility_counts;
		for (pair<pair<int, int>, int> p : adjacent_counts) { //Deep copy of adjacent_count
			possibility_counts[p.first] = p.second;
		}

		for (int j = 0; j < edge->size(); j++) { //Subtract counts if the current possibility has edge square as flag
			if (((i >> j) & 1) == 1) {
				execute_callback(board, (*edge)[j].first, (*edge)[j].second, &decrement_count, &possibility_counts);
			}
		}

		bool good_possibility = true;
		for (pair<pair<int, int>, int> p : possibility_counts) { //Check each count
			if (p.second != 0) {
				good_possibility = false;
			}
		}

		if (good_possibility) { //Good possibility
			count_possibilities += 1;
			for (int j = 0; j < edge->size(); j++) { //Increment probability for flag squares on this possibility
				if (((i >> j) & 1) == 1) {
					pair<int, int> p = (*edge)[j];
					m_probabilities[p.first][p.second] += 1;
					mine_count += 1;
				}
			}
		}
	}

	cout << count_possibilities << " possibilities found for edge" << endl;

	for (int i = 0; i < edge->size(); i++) { //Adjust probabilities for number of possibilities
		pair<int, int> p = (*edge)[i];
		m_probabilities[p.first][p.second] /= count_possibilities;
		cout << p.first << "," << p.second << ": " << m_probabilities[p.first][p.second] << endl;
	}

	delete[] good_count;

	return mine_count / count_possibilities;
}

//Approximation of optimal edge probabilities by splitting constraints for each edge into a subset of constraints
//Find possibilities for edge squares constrained by each subset
//Struggles with intersection of multiple subsets (for edges squares relevant to more than one subset)
double Bot::update_probabilities_sectioned(vector<pair<int, int>>* edge) {
	int* good_count = new int[NUM_THREADS * edge->size()];
	int success_count[NUM_THREADS] = { 0 };
	int flag_count = 0;

	unordered_map<pair<int, int>, int, PairHashStruct> adjacent_counts; //Get constraints
	for (int i = 0; i < edge->size(); i++) {
		pair<int, int> p = (*edge)[i];
		execute_callback(board, p.first, p.second, &insert_into_map, &adjacent_counts);

	}
	for (pair<pair<int, int>, int> p : adjacent_counts) { //Adjust constraints for existing flags
		flag_count = 0;
		execute_callback(board, p.first.first, p.first.second, &count_known_mines, &flag_count);
		adjacent_counts[p.first] -= flag_count;
	}

	for (int i = 0; i < edge->size(); i++) { //Zero out probabilities of existing edge squares
		pair<int, int> p = (*edge)[i];
		m_probabilities[p.first][p.second] = 0;
	}

	double mine_count = 0;
	for (pair<pair<int, int>, int> p : adjacent_counts) { //Approximate mine count of edge
		mine_count += p.second;
	}

	//Generate edge subsets
	unordered_set<pair<int, int>, PairHashStruct>* s_map = new unordered_set <pair<int, int>, PairHashStruct>[adjacent_counts.size()];
	unordered_set<pair<int, int>, PairHashStruct>* i_map = new unordered_set <pair<int, int>, PairHashStruct>[adjacent_counts.size()];
	unordered_map<pair<int, int>, unordered_set<pair<int, int>, PairHashStruct>*, PairHashStruct> interior_to_edge_squares_map;
	unordered_map<pair<int, int>, unordered_set<pair<int, int>, PairHashStruct>*, PairHashStruct> interior_to_interior_squares_map;
	int ind = 0;
	for (pair<pair<int, int>, int> p : adjacent_counts) { //Get map of constraints to edge squares
		execute_callback(board, p.first.first, p.first.second, &build_adjacency_set, &s_map[ind]);
		interior_to_edge_squares_map[p.first] = &s_map[ind];
		ind += 1;
	}

	ind = 0;
	for (pair<pair<int, int>, int> p : adjacent_counts) { //Get map of interior squares to adjacent interior squares
		for (pair<int, int> c : *interior_to_edge_squares_map[p.first]) {
			for (pair<pair<int, int>, int> comp : adjacent_counts) {
				if (interior_to_edge_squares_map[comp.first]->find(c) != interior_to_edge_squares_map[comp.first]->end() && comp.first != p.first) {
					i_map[ind].insert(comp.first);
				}
			}
		}
		interior_to_interior_squares_map[p.first] = &i_map[ind];
		ind += 1;
	}

	pair<int, int>* adjacent_ordered = new pair<int, int>[adjacent_counts.size()];
	ind = 0;
	for (pair<pair<int, int>, int> p : adjacent_counts) { //Order the unordered map
		adjacent_ordered[ind] = p.first;
		ind += 1;
	}

	//Tree search for subsets satisfying edges size constraint
	vector<AdjacencyOrderingNode*> adjacency_build_queue;
	vector<AdjacencyOrderingNode*> roots;
	for (int i = 0; i < adjacent_counts.size(); i++) { //Initialize vector of root nodes for each constraint
		AdjacencyOrderingNode* root = new AdjacencyOrderingNode;
		root->coord = adjacent_ordered[i];
		root->parent = nullptr;
		adjacency_build_queue.push_back(root);
		roots.push_back(root);
	}

	while (!adjacency_build_queue.empty()) { //Depth first search through the nodes of the tree
		AdjacencyOrderingNode* node = adjacency_build_queue.back();
		adjacency_build_queue.pop_back();
		for (pair<int, int> p : *interior_to_edge_squares_map[node->coord]) { //Add all adjacent edge squares to the edge square set
			if (node->current_squares.find(p) == node->current_squares.end()) {
				node->current_squares.insert(p);
			}
		}
		if (node->current_squares.size() > MAX_SIZE) { //Exceeded number of maximum possible edge square, remove node from tree
			if (node->parent != nullptr) {
				node->parent->children.erase(find(node->parent->children.begin(), node->parent->children.end(), node));
			}
			delete node;
		}
		else {
			for (pair<int, int> p : *interior_to_interior_squares_map[node->coord]) { //Good number of squares, add children to tree
				if (node->visited.find(p) == node->visited.end()) {
					AdjacencyOrderingNode *n = new AdjacencyOrderingNode;
					n->parent = node;
					for (pair<int, int> v : node->visited) { //Copy visited set
						n->visited.insert(v);
					}
					n->visited.insert(node->coord);
					for (pair<int, int> c : node->current_squares) { //Copy current square set
						n->current_squares.insert(c);
					}
					n->coord = p;
					node->children.push_back(n);
					adjacency_build_queue.push_back(n); //Push to stack
				}
			}
		}
	}

	//Traverse tree to get the edge squares and constraints for each subset
	unordered_set<int> existing_traversals;
	vector<unordered_set<pair<int,int>, PairHashStruct>> adjacent_subsets;
	for (int i = 0; i < adjacent_counts.size(); i++) { 
		adjacency_build_queue.push_back(roots[i]);
	}

	while (!adjacency_build_queue.empty()) {
		AdjacencyOrderingNode* node = adjacency_build_queue.back();
		adjacency_build_queue.pop_back();
		if (node->children.size() == 0) { //Leaf node, add to set
			node->visited.insert(node->coord);
			if (existing_traversals.find(hash_set(node->visited)) == existing_traversals.end()) { //Check for duplicate set
				existing_traversals.insert(hash_set(node->visited));
				adjacent_subsets.push_back(node->visited);
			}
		}
		else {
			for (AdjacencyOrderingNode* n : node->children) {
				adjacency_build_queue.push_back(n); //Search children for non-leaf node
			}
		}
	}

	//Remove subsets
	for (vector<unordered_set<pair<int, int>, PairHashStruct>>::iterator itr = adjacent_subsets.begin(); itr != adjacent_subsets.end(); itr++) {
		for (vector<unordered_set<pair<int, int>, PairHashStruct>>::iterator del = itr + 1; del != adjacent_subsets.end(); del++) {
			if (is_subset(*itr, *del)) {
				del = adjacent_subsets.erase(del);
				del--;
			}
		}
	}

	cout << adjacent_subsets.size() << " unique subsets found" << endl;

	//Get edge squares from each constraint subset
	vector <unordered_set<pair<int, int>, PairHashStruct>*> sub_edges;
	for (unordered_set<pair<int, int>, PairHashStruct> s : adjacent_subsets) {
		unordered_set < pair<int, int>, PairHashStruct>* set = new unordered_set < pair<int, int>, PairHashStruct>;
		for (pair<int, int> p : s) {
			for (pair<int, int> l : *interior_to_edge_squares_map[p]) {
				set->insert(l);
			}
		}
		sub_edges.push_back(set);
	}

	int count_possibilities = 0;
	unordered_map<pair<int, int>, int, PairHashStruct> correction; //Initialize storage of count of number of subsets an edge square appears in
	for (pair<pair<int, int>, int> p : adjacent_counts) {
		correction[p.first] = 0;
	}

	unordered_set<pair<int, int>, PairHashStruct> subset_union; //Union of all edge squares for subsets
	for (unordered_set<pair<int, int>, PairHashStruct>* se: sub_edges) {
		for (pair<int, int> p : *se) {
			subset_union.insert(p);
		}
	}
	for (vector<pair<int, int>>::iterator itr = edge->begin(); itr != edge->end(); ) { //Remove all edge squares not in subsets from edge (to prevent later changes to probability)
		if (subset_union.find(*itr) == subset_union.end()) {
			itr = edge->erase(itr);
		}
		else {
			itr++;
		}
	}
	
	//Brute force check possibilities for each subset
	for (int k = 0; k < sub_edges.size(); k++) {
		vector<pair<int, int>> e;
		for (pair<int, int> p : *sub_edges[k]) { //Get sub-edge for this subset
			e.push_back(p);
		}
		for (pair<int, int> p : e) { //Increment count for relevant squares
			correction[p] += 1;
		}

		unordered_set<pair<int, int>, PairHashStruct> interior = adjacent_subsets[k];
		count_possibilities = 0;
		for (int i = 0; i < (int)pow(2, e.size()); i++) { //Check each possibility (exponential time)
			unordered_map<pair<int, int>, int, PairHashStruct> possibility_counts;
			for (pair<pair<int, int>, int> p : adjacent_counts) { //Deep copy of adjacent_count
				possibility_counts[p.first] = p.second;
			}

			for (int j = 0; j < e.size(); j++) { //Subtract counts if the current possibility has edge square as flag
				if (((i >> j) & 1) == 1) {
					execute_callback(board, e[j].first, e[j].second, &decrement_count, &possibility_counts);
				}
			}

			bool good_possibility = true;
			for (pair<int, int> p: interior) { //Check each count
				if (possibility_counts[p] != 0) {
					good_possibility = false;
				}
			}

			if (good_possibility) { //Good possibility
				count_possibilities += 1;
				for (int j = 0; j < e.size(); j++) { //Increment probability for flag squares on this possibility
					if (((i >> j) & 1) == 1) {
						pair<int, int> p = e[j];
						m_probabilities[p.first][p.second] += 1;
					}
				}
			}
		}
		cout << count_possibilities << " possibilities found for subset" << endl;

		for (int j = 0; j < e.size(); j++) { //Adjust probabilities
			pair<int, int> p = e[j];
			m_probabilities[p.first][p.second] /= count_possibilities;
		}
	}
	for (int j = 0; j < edge->size(); j++){ //Adjust probabilities for frequency
		pair<int, int> p = (*edge)[j];
		if (correction[p] != 0) {
			m_probabilities[p.first][p.second] /= correction[p];
		}
		cout << p.first << ", " << p.second << ":  " << m_probabilities[p.first][p.second] << endl;
	}

	//Cleanup
	//Cleanup tree
	for (int i = 0; i < adjacent_counts.size(); i++) {
		adjacency_build_queue.push_back(roots[i]);
	}

	while (!adjacency_build_queue.empty()) {
		AdjacencyOrderingNode* node = adjacency_build_queue.back();
		adjacency_build_queue.pop_back();
		if (node->children.size() == 0) { //Delete leaf node
			if (node->parent != nullptr) {
				node->parent->children.erase(find(node->parent->children.begin(), node->parent->children.end(), node));
				if (node->parent->children.size() == 0) {                                     
					adjacency_build_queue.push_back(node->parent); //Check parent of leaf node also
				}
			}
			delete node;
		}
		else {
			for (AdjacencyOrderingNode* n : node->children) {
				adjacency_build_queue.push_back(n); //Check children if not leaf node
			}
		}
	}
	delete[] s_map;
	delete[] i_map;
	delete[] good_count;
	delete[] adjacent_ordered;

	for (int i = 0; i < sub_edges.size(); i++) {
		delete sub_edges[i];
	}

	return mine_count / edge->size();
}