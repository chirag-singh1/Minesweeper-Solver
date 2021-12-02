# Minesweeper-Solver

## Setup
After cloning the repository, run the commands: 
```
mkdir build
cd build
cmake ..
make
```
This will build the `minesweeper` executable. Running the minesweeper executable will open a terminal window with the Minesweeper game on it. Run the program with the `-h` flag for an idea of the setup options and user controls.

## Cleanup
Run `rm -rf build` to remove the build folder. 

# About

## Introduction

## Definitions

Known square: any square that is either known to be safe (having revealed the number of mines adjacent to it) or known to be a mine (having identified and marked it). 

Constraint: Any known square bordering at least one unknown square

Edge: Given a set of constraints C and the set E of all unknown squares bordering one or more constraint in C, E is an edge if and only if for any two squares in C or E, there exists a path between the two squares traveling only through the squares in C and E. In other words, an edge in this context is defined as a group of unknown squares that share a local set of constraints.

In the example, the red-circled squares make up the edge E, and the yellow-circled squares make up the corresponding constraints C. Note that the squares in C and E do not necessarily have to be conncted to each other, so long as the path requirement above is satisfied.

<p align="center">
  <img width="220" height="149" src="https://user-images.githubusercontent.com/66097224/144328352-6bc4c18a-76ee-47e2-85e3-92fb682286ca.png">
</p>

Sub-Edge: Given an edge E and corresponding set of constraints C, a sub-edge S is the set of all unknown squares in E bordering one or more constraints in C

Maximal sub-edge: For a given n, a maximal sub-edge S of an edge E is a sub-edge with cardinality less than or equal to n where adding any constraint to the set of corresponding set of constraints of S increase the cardinality of S to be greater than n

Coordinate system: All squares are 0-indexed with the first coordinate indicating the row (increasing from top to bottom) and the second coordinate indicating the column (increasing from left to right)

## Strategy

### Single Square Search
Often, the constraints on any given square are enough to guarantee that a square is a mine or not. In such cases, which often arise along large edges that would be expensive to search, searching for the constraints on a single square can quickly produce a significant queue of safe moves. Even when no safe moves can be identified, identifying and marking mines is an important step to support future edge searches. Because the single square search runs in linear time, the low cost of searching for safe and unsafe squares decreases the frequency of more expensive searches.

The single square search iterates over each constraint, checking the number of unknown squares adjacent to the constraint and the number of known mines adjacent to the constraint. If the number of adjacent known mines equals the constraint value, each adjacent unknown square is marked as safe and queued for a safe move. If the number of adjacent mines and the number of unknown squares sums to the constraint value, each adjacent unknown square is marked as a mine. 

In the example below, the constraints (1,3) and (3,2) both border only one unknown square, indicating that those squares must be flags. Therefore, the constraints on (1,4), (2,3), and (4,2) are satisfied, and any unknown square adjacent to any of these constraints is safely added to the queue. 

<p align="center">
  <img width="223" height="361" src="https://user-images.githubusercontent.com/66097224/144327314-ac717380-a2b2-4887-a623-2913f27c71e0.png">
</p>

### Edge Search
For most games, a single-square search will eventually stop making progress (and will no longer be able to identify guaranteed safe squares or mines). Once this occurs, we must turn to a more comprehensive search. While checking each possibility for the distribution of the remaining mines would allow for the precise calculation of the probability of a mine on each square, the number of possibilities to check increases with the factorial of the number of remaining squares and is generally impractical. Instead, we check each possibility for each edge under a certain maximum size. While the number of possibilities for each edge will grow exponentially with the size of the edge, for small edges, it is still a valuable tool to identify potentially beneficial moves, especially on small boards. 

#### Identifying Edges
To generate the possibilities for each edge, each edge first must be identified. To do so, for each square on the board:
```
1. If the square is unknown and has not been visited, push it to a search queue
2. While the search queue is not empty:
  a. Pop the top element from the queue
  b. If the top element from the queue has not been visited, get all surrounding known squares
    i. If there are surrounding known squares, push the unknown squares surrounding each known square to the search queue
    ii. Mark each known square as visited
    iii. Insert the current square to the current edge
  c. Mark the current square as visited
3. Append the current edge to the list of edges 
4. Mark the current square as visited
```

This algorithm will run in linear time. 

#### Checking Possibilties
Possibilities are checked and generated simultaneously by iterating over the integers from 0 to 2^n-1, where n is the length of the edge. Bitwise shifts are used to determine whether each edge square is a mine. Then, iterate over each edge square, decrementing a temporary count for each constraint. The possibility is valid if each temporary count is 0 after decrementing this count. Early stopping can be used to slightly improve the performance of this checking process. Counting the number of possibilities and the number of times each edge square is a mine provides a probability for each edge square to be a mine. In most cases, this will generate a number of safe and guaranteed unsafe squares, creating several moves to safely queue. However, even when there are no safe moves known, this process still provides valuable information. First, we determine the probability that each edge square is a mine, providing a valuable resource for potentially guessing highly likely safe squares. We also determine an estimated value for the number of mines in the edge, which can be used to calculate the probability that any non-edge square is a mine. However, as valuable as this algorithm is, its runtime increases with n* 2^n, and is therefore only suitable for small edges. 

#### Example
In the example below, there is not enough information for a single square search to identify a safe square or mine. As a result, a search is run on the edge surrounding the known squares. 4 possibilities are found for this edge, telling us that the square (2,2) is a guaranteed mine, while the other 5 squares have a 50% chance of being a mine. This allows us to safely mark the mine on (2,2), while also giving an estimate of the number of mines in the edge. With this estimate, we calculate that there is a 90.5797% chance that any non-edge square in the grid is safe, leading to the safe move of (0,4).  

<p align="center">
  <img width="541" height="517" src="https://user-images.githubusercontent.com/66097224/144331658-9bfee38d-da6a-42e0-b3a9-666881cf71d6.png">
</p>

### Sub-Edge Search
When an edge grows too large to search through brute force, we turn to the sub-edge search. For this search, we check the possiblities for each maximal sub-edge for a maximum length and edge, which limits the number of possibilities to be searched. 

#### Finding Maximal Sub-Edges
To find the maximal sub-edges of an edge, a brute force approach would be to check each sub-edge of the edge for its length. However, this approach would run with combinatorial time complexity. While the time complexity of the worst-case scenario cannot be improved significantly, we can significantly improve the time complexity of the average case by using a tree. Each tree node stores the coordinate of a constraint, its children, the constraints already visited in its path in the tree, the sub-edge corresponding to its path in the tree, and a pointer to its parent. By using a tree, we eliminate sub-edges that are too long immediately, preventing any potential computation of any related sub-edge. The algorithm for constructing this tree is the following:

```
1. Push each constraint orresponding to the edge into a search queue
2. While the search queue is not empty:
  a. Pop the top constraint from the queue
  b. Append all adjacent unknown square to this node's sub-edge list
  c. If the node's sub-edge list is greater than the maximum sub-edge size, delete the node from its parent's children
  d. Otherwise, for each adjacent constraint
    i. Create a new tree node and copy the visited list and sub-edge list from the current node
    ii. Add the current node to the visited list of the new node
    iii. Push the new node to the tree
```

The copying of the visited list and sub-edge list determines the time complexity of building the tree. We know that the sub-edge list cannot be larger than the edge size limit, while the visited list cannot be significantly longer (as each constraint must border at least one square and significant overlap between the edge squares of multiple constraints is also finite). Thus, the approximate average time complexity of this copying operation is linear with the size of the tree (as the edge size limit is constant). While the size of the tree could increase exponentially with edge size in the worst case, this is highly unlikely. In the average case, the depth of the tree is finite and determined by the edge size, edge size limit, and average number of additional sub-edge squares per constraint. The average depth of the tree will be the edge size limit divided by the average number of unique sub-edges per constraint. With a properly chosen edge size limit and the assumption that the number of added sub-edge squares per constraint remains relatively constant (an assumption that holds true in nearly all cases), the depth of the tree is a constant, relatively small, number that does not grow with the edge size. Now, we find the number of nodes per tree. As each constraint borders a maximum of 8 squares (and often significantly fewer), the maximum number of nodes per tree is 8^d, where d is the constant depth of the tree above. As there are n trees and a constant number of nodes per tree, the number of nodes in the tree grows with the edge size in the average case, meaning that the time complexity to generate the tree grows linearly with the edge size. 

To get the maximal sub-edges from this tree, we traverse the tree, taking each root to leaf path as a sub-edges. As shown above, the number of sub-edges will grow linearly with the edge size in the average case. The subsets are checked to find the maximal sub-edges (removing sub-edges that are the subset of another sub-edges). This takes quadratic time, leading to an overall quadratic time complexity for the generation of the maximal sub-edges. 

#### Checking Possibilities
The algorithm for checking the possibilities for each sub-edge is the same as for the brute-force edge search, with a few slight difference. First, because the size of each maximal sub-edge cannot grow, the time to check each sub-edge does not grow with edge size. Therefore, the time to check all possibilities of an edge with a sub-edge search grows linearly with the number of maximal sub-edges, which grows linearly with the edge size on average, producing an average linear time. Second, checking the possibilities of a maximal sub-edge produces the question of how to reconcile the probabilities of each sub-edge into an overall probability for the edge. When an edge square is only in one maximal sub-edge, the probability can simply be taken as-is, as there are no other constraints affecting it. Likewise, when an edge square is either guaranteed safe or known to be a mine from one maximal sub-edge, this is true for any sub-edge. However, for edges in multiple sub-edges with a non-guaranteed state, the geometric mean of each sub-edge probability appears an effective approximation of the probability for these squares. While this is not precise or optimal, it generally performs well enough, as there is generally either some guarantee or enough flags in the edge for a non-edge square to be more probable than any edge square. 
### Guessing
Once all available searches have been exhausted with no safe moves, a guess must be made. The obvious choice for a guess would be the likeliest safe square, which is chosen. However, if multiple squares have the same likelihood of being safe, the tiebreaker is the proximity of a square to the sides of the board. Squares closest to the sides of the board have the least information about them available (as the sides of the board do not provide any constraints), and are therefore the most likely to eventually create situations that require guessing. While this heuristic has not been rigorously proven to improve results, it does (anecdotally and through simulation) appear to improve success rates. 

### Potential Improvements

#### Backtracking Edge Possibility Generation
Instead of generating all possible edge possibilities, use a backtracking algorithm to dynamically place mines along the edge, checking each constraint during this process. Each possibility for each edge square would be considered in a tree, with bad possibilities causing a backtracking to the last valid state. This would avoid some of the repeated work done in checking similar possibilities, but would still have exponential worst case time. The average case time would likely improve significantly by a scalar factor even if the time complexity does not improve. 

#### Bad Possibility Caching
Storing a cache of the bad parts of bad possibilities where checking terminated early could help to save some time when checking future possibilities. It would take time linear with the edge size to check each possibility against the cache of bad possibilities using a hash-table implemented set. This would not improve the time complexity of generating and checking possibilities, but would likely improve the overall time by a large scalar complexity. 

#### Lookahead
In certain situations, a guess with a higher probability of being a mine may provide a higher probability of winning the game if it generates a position guaranteed to remove any future guesses from the current edge (while safer moves in the short-term may require more guessing in the future). By only choosing the higher probability move, the current method for guessing in unsafe situations does not support this tradeoff. However, with the exponential complexity of both possibilities in the short term and in the long term, looking ahead likely is not practical in any reasonable time. 
