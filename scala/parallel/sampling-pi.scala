import scala.util.Random

// define parallel function
def parallel[A, B](taskA: => A, taskB: => B): (A, B) = (taskA, taskB)

// count hits in quartile
def mcCount(iter: Int): Int = {
    val randomX = new Random
    val randomY = new Random

    var hits = 0
    for (i <- 0 until iter) {
        val x = randomX.nextDouble      // [0, 1]
        val y = randomY.nextDouble      // [0, 1]

        if (x*x + y*y < 1) {
            hits = hits + 1
        }
    }
    hits
}

// estimation of Pi sequentially
def montoCarloPiSeq(iter: Int): Double = {
    val start = System.nanoTime()

    // start: calculation
    val result = 4.0 * mcCount(iter) / iter
    // end: calculation

    val end = System.nanoTime()

    println(end - start)
    result
}

// estimation of Pi parallel
def montoCarloPiPar(iter: Int): Double = {
    val start = System.nanoTime()

    // start: calculation
    val ((pi1, pi2), (pi3, pi4)) = parallel(
        parallel(mcCount(iter / 4), mcCount(iter / 4)),
        parallel(mcCount(iter / 4), mcCount(iter - 3 * (iter / 4))))

    val result = 4.0 * (pi1 + pi2 + pi3 + pi4) / iter
    // end: calculation

    val end = System.nanoTime()

    println(end - start)
    result
}

println(montoCarloPiSeq(1000000))
println(montoCarloPiPar(1000000))