// define parallel function
def parallel[A, B](taskA: => A, taskB: => B): (A, B) = (taskA, taskB)

// power function
def power(x: Int, p: Double): Int = {
    math.exp(p * math.log(math.abs(x))).toInt
}

// sum segment sequentially
def sumSegment(a: Array[Int], p: Double, s: Int, t: Int): Int = {
    var i = s
    var sum: Int = 0

    while (i < t) {
        sum = sum + power(a(i), p)
        i = i + 1
    }
    sum
}

def pNormRec(a: Array[Int], p: Double): Int = {
    power(segmentRec(a, p, 0, a.length), 1 / p)
}

def segmentRec(a: Array[Int], p: Double, s: Int, t: Int): Int = {
    
    val threshold = 10

    // small segments: do it sequentially
    if (t - s < threshold) {
        sumSegment(a, p, s, t)
    }
    else {
        val m = s + (t - s) / 2
        val (sum1, sum2) = parallel(segmentRec(a, p, s, m), segmentRec(a, p, m, t))

        sum1 + sum2
    }
}