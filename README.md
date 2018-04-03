# lightsoutirregular
Cheating at lights out

why?
---

I wanted to be able to make sure that my lights out puzzles were actually
solvable, and I didn't want to be prone to human error, so I thought it
best to write something that would prove it one way or another.

Also, this means I can decide not to worry about solvability when designing the
puzzles, and just concentrate on making them look interesting or pretty.

how?
---

I started with a straight forward brute force approach. I used grey codes to
make the brute force use the smallest number of button presses to actually try
evey combination of the lights, but even then it was a very large number of
combinations for some boards. I looked into how people have solved the lights
out problem and found a rather lovely solution using Gaussian elimination.
Adding this solution took an hour or so, and means I can handle solving puzzles
that are rather much more large than before. For 5x5 there are some
unsolvables, and it can detect that, returning false and unsolvable.
