# LaFuN programming language

LaFuN empowers you with the most **expressive** comments you have ever seen.
We have combined the power of LaTeX with a simple Javascript-like sublanguage named FuN
to bring you a new comment-writing experience that will improve code readability for everyone.

Here's a simple demonstration of what LaFuN brings to the table.

```
\section{Introduction}

The $L^2$-norm, or Euclidean distance, is often used to measure the distance
between two points in an $n$-dimensional vector space.
However, it can be slow to compute as it requires finding the square root of a number.
As such, it is common to use the squared $L^2$-norm instead,
which is often notated $\left\Vert \cdot \right\Vert_2^2$.

\section{Implementation}

Here is an implementation of the squared $L^2$-norm for 2D vectors.

\fun{SqL2Norm2D}{v}{
  \fun{Sq}{n}{ return n * n; }
  SqX := Sq(v.x);
  SqY := Sq(v.y);
  return SqX + SqY;
}

\section{Usage}

To use @SqL2Norm2D, you will need a class that encapsulates a vector.

\class{Vector2}{x, y}{
  self.x = x;
  self.y = y;
}

Let's test it out!

\fun{main}{}{
  vector := Vector2(3, 4);
  print("vector: x=" + vector.x + ", y=" + vector.y);
  print("SqL2Norm2D(vector):", SqL2Norm2D(vector));
}

This program should print $25$ to the console when run.

\section{Conclusion}

@SqL2Norm2D is a really simple function and is fast on most architectures.
Note that the factoring out of the @Sq function
may lead to a slight slowdown due to function call overhead.
As such, it may be good to consider
inlining the entire computation into a single return statement.
```

When compiling to LaTeX then to PDF, we can see just how beautiful our code has become:

![Sample LaFuN output](/images/readme-sample-latex-output.png)

Running it is also simple!

```
$ ./build/lafun examples/readme.fun -o examples/readme.js
$ node examples/readme.js
25
```

# Try LaFuN

To try LaFuN, simply clone this repository and try some of the example commands below to get started!

```
$ make
$ ./build/lafun examples/fibonacci.fun -o test.js
$ node test.js
vector: x=3, y=4
SqL2Norm2D(vector): 25
$ ./build/lafun examples/readme.fun --latex test.tex
$ # Compile test.tex with your LaTeX compiler of choice (e.g. Overleaf)
```

If you wish to use a custom LaTeX prelude, use `--no-latex-prelude`.

```
$ ./build/lafun examples/readme-with-prelude.fun --no-latex-prelude --latex test.tex
```
