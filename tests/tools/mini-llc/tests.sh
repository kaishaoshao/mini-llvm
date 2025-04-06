#!/bin/bash

set -e

tests=(
    circular_call
    irreducible_cfg
    phi_lost_copy
    phi_swap
    many_args
    many_regs
    many_regs_large_stack_frame
    arithmetic
    mul
    div
    rem
    relational
    casting
    select
    max
    sum
    fibonacci
    power
    is_prime
    gcd
    gcd_iterative
    hanoi
    partition_function
    n_queens
    array_sum
    matmul
    edit_distance
    sieve_of_eratosthenes
    bubble_sort
    selection_sort
    insertion_sort
    heapsort
    quicksort
    mergesort
    dfs
    bfs
    topological_sort
    tarjan_strongly_connected_components
    dijkstra_shortest_paths
    gaussian_elimination
)

for test_name in "${tests[@]}"; do
    echo "$test_name"
done
