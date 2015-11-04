# (Fast) Experimental evidence of the Collatz conjecture


Student: Guillaume Leclerc (224338)

## Vocabulary

### Syracuse serie

The syracuse recurrence is defined this way:

$$ f(n) = n/2 \text{ if n is a multiple of 2 }$$
$$ f(n) = 3 n + 1 \text{ if n is not a multiple of 2}$$

The $k^{th}$ syracuse serie is defined the following way:

$$ u_0 = k $$
$$ u_{n+1} = f(u_n) $$

### Collatz conjecture

The Collatz conjecture (Still an open problem in mathematics) says that: For all integer $k$ the $t^{th}$ syracuse serie eventually reach 1.

### Flight time of a Syracuse serie

The flight time of a syracuse serie is the smallest $n$ such that $u_n = 1$


## Project proposition

The aim of this project is to check for as much possible integers that the Collatz conjecture is true and possibliy compute it's flight time.

## Why is this project interesting

At first sight it might seem that having a parallel implementation is trivial. But we can consider a very simple optimisation. We can maintain a set of integers that we know they satisfy the Collatz conjecture. If at some point we find that $u_n$ is in this set then there is no need to continue the computation we know that $k$ satisfies the conjecture too. The sequential implementation can be quite fast with this simple euristic. It is hard to have a parallel implementation with a good absolute speedup.
