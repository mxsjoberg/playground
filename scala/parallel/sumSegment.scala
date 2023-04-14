// define parallel function
def parallel[A, B](taskA: => A, taskB: => B): (A, B) = (taskA, taskB)

// power function
def power(x: Int, p: Double): Int = {
    math.exp(p * math.log(math.abs(x))).toInt
}

// sum a segment
def sumSegment(a: Array[Int], p: Double, s: Int, t: Int): Int = {
    var i = s
    var sum: Int = 0

    while (i < t) {
        sum = sum + power(a(i), p)
        i = i + 1
    }
    sum
}

// sequencial version
def pNorm(a: Array[Int], p: Double): Int = {
    power(sumSegment(a, p, 0, a.length), 1/p)
}

// two part parallel version
def pNormTwoPart(a: Array[Int], p: Double): Int = {
    // middle point
    val m = a.length / 2

    // sum segment in two parts
    val (sum1, sum2) = parallel(sumSegment(a, p, 0, m), sumSegment(a, p, m, a.length))

    power (sum1 + sum2, 1 / p)
}

// four part parallel version
def pNormFourPart(a: Array[Int], p: Double): Int = {
    // middle points (quartiles)
    val m1 = a.length / 4
    val m2 = a.length / 2
    val m3 = 3 * a.length / 4

    // sum segment in two parts
    val ((sum1, sum2), (sum3, sum4)) = parallel(
                                        parallel(sumSegment(a, p, 0, m1), sumSegment(a, p, m1, m2)),
                                        parallel(sumSegment(a, p, m2, m3), sumSegment(a, p, m3, a.length)))

    power (sum1 + sum2 + sum3 + sum4, 1 / p)
}

