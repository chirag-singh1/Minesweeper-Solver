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

Sub-Edge: Given an edge E and corresponding set of constraints C, a sub-edge S is the set of all unknown squares in E bordering one or more constraints in C

Maximal sub-edge: For a given n, a maximal sub-edge S of an edge E is a sub-edge with cardinality less than or equal to n where adding any constraint to the set of corresponding set of constraints of S increase the cardinality of S to be greater than n

## Optimizations

### Single Square Search
Often, the constraints on any given square are enough to guarantee that a square is a mine or not. In such cases, which often arise along large edges that would be expensive to search, searching for the constraints on a single square can quickly produce a significant queue of safe moves. Even when no safe moves can be identified, identifying and marking mines is an important step to support future edge searches. Because the single square search runs in linear time, the low cost of searching for safe and unsafe squares decreases the frequency of more expensive searches.

The single square search iterates over each constraint, checking the number of unknown squares adjacent to the constraint and the number of known mines adjacent to the constraint. If the number of adjacent known mines equals the constraint value, each adjacent unknown square is marked as safe and queued for a safe move. If the number of adjacent mines and the number of unknown squares sums to the constraint value, each adjacent unknown square is marked as a mine. 

For example, 

![image](https://user-images.githubusercontent.com/66097224/144300170-582b5f96-f46f-47e3-9e12-e08d5b745f2a.png)![image](https://user-images.githubusercontent.com/66097224/144300371-603fdd48-95fc-43a2-968c-a293e86334f1.png)



### Edge Search
The next most important optimization is considering only the possible arrangements of mines within squares where some information is known about the state of these squares. These squares comprise the edges of the board. 

### Sub-Edge Search
