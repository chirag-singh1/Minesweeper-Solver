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

## Optimizations

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
Possibilities are checked and generated simultaneously by iterating over the integers from 0 to 2^n-1, where n is the length of the edge. Bitwise shifts are used to determine whether each edge square is a mine. Then, iterate over each edge square, decrementing a temporary count for each constraint. The possibility is valid if each temporary count is 0 after decrementing this count. Early stopping can be used to slightly improve the performance of this checking process. Counting the number of possibilities and the number of times each edge square is a mine provides a probability for each edge square to be a mine. In most cases, this will generate a number of safe and guaranteed unsafe squares, creating several moves to safely queue. However, even when there are no safe moves known, this process still provides valuable information. First, we determine the probability that each edge square is a mine, providing a valuable resource for potentially guessing highly likely safe squares. We also determine an estimated value for the number of mines in the edge, which can be used to calculate the probability that any non-edge square is a mine. 

### Sub-Edge Search

### Potential Improvements

