
// task interface
def task(c: => A): Task[A] = {
    trait Task[A] {
        def join: A
    }
}

// define parallel function
def parallel[A, B](cA: => A, cB: => B): (A, B) = {
    val taskB: Task[B] = task { cB }
    val taskA: A = cA

    (taskA, taskB.join)
}

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

//
val task1 = task { sumSegment(a, p, 0, mid1) }
val task2 = task { sumSegment(a, p, mid1, mid2) }
val task3 = task { sumSegment(a, p, mid2, mid3) }
val task4 = task { sumSegment(a, p, mid3, a.length) }

power(task1 + task2 + task3 + task4, 1 / p)