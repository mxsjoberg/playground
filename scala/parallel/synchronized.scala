private val x = new AnyRef {}
private var uidCount = 0L

// use synchronized to avoid duplicates in two simultaneous threads
def getUniqueId(): Long = x.synchronized {
    uidCount = uidCount + 1
    uidCount
}

def startThread() = {
    val thread = new Thread {
        override def run() {
            // get unique id
            val uids = for (i <- 0 until 10) yield getUniqueId()
                println(uids)
        }
    }
    thread.start()
    thread
}

// different threads use the synchronized block to agree on unique value
startThread(); startThread()