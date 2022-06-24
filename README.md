## Partitioning by Fiduccia-Mattheyses Algorithm
- This is a project from *CS313000,Introduction to Computer-Aided Design of Integrated Circuits* course at NTHU.
- Solve the 2-way partioning problem by [FM algorithm](https://).
- Solve the k-way partition by directly grouping all the cells into a valid partition. (so the k-way is not cost optimized!)
- Input: cells' area and nets' connections
- Output: partition results

#### Compile and Execute
```
    $ make
    $ ./pa2 <.in file> <.out file>
```
#### partition cost evaluation
```
    for cell i,
    C(i) = ( (# groups that net i spans)-1 )^2
```

#### Testcase format
- Sample input:
```
    71            // maximum area constraint per group
    .cell         // the following lines are 'cell' info
    10            // total number of cells, n=10
    0 8           // cell 0 with area = 8
    1 15          // cell 1 with area = 15
    2 6
    3 22
    4 11 
    5 8
    6 13
    7 18
    8 20
    9 7
    .net            // the following lines are 'net' info
    3               // total number of nets, m=3
    5               // net 0 contains 5 cells
    0 3 9 6 8       // net 0 connects to cell 0, 3, 9, 6 and 8
    4               // net 1 contains 4 cells
    1 2 3 7         // net 1 connects to cell 1, 2, 3, 7
    6
    4 5 6 7 8 9
```
- Sample output:
```
    3    // total cost = 3
    2    // total number of groups = 2
    1    // cell 0 belongs to group 1
    0    // cell 0 belongs to group 0
    0
    0
    0
    0
    1
    1
    1
    1
    
```