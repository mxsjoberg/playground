class HelloThread extends Thread {
    override def run() {
        println("Hello")
        println("World!")
    }
}

def main() {
    val first = new HelloThread
    val second = new HelloThread

    first.start()
    second.start()
    first.join()
    second.join()
}

// first run
println("-- 1 --")
main()
// second run
println("-- 2 --")
main()
// third run
println("-- 3 --")
main()