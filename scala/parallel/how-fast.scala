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

// sum a segment recursive
def segmentRec(a: Array[Int], p: Double, s: Int, t: Int): Int = {
    
    val threshold = 10

    // small segments: do it sequentially
    if (t - s < threshold) {
        sumSegment(a, p, s, t)
    }
    else {
        val m = s + (t - s) / 2
        val (sum1, sum2) = parallel(
            segmentRec(a, p, s, m), 
            segmentRec(a, p, m, t))

        sum1 + sum2
    }
}

// work W   : O(t - s)
// depth D  : O(log(t - s))
//
// c1 * log(t - s) + c2 + (c3(t - s) + c4) / P
//
// c is constant and P is threads
//
// Read more: Amdahl's Law


