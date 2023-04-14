class Account(private var amount: Int = 0) {

    // get amount
    def getAmount(): Int = {
        this.amount
    }

    // get unique id to avoid deadlocks
    private val x = new AnyRef {}
    private var uidCount = 0L
    def getUniqueUid(): Long = x.synchronized {
        uidCount = uidCount + 1
        uidCount
    }

    // use unique id to ensure ordering
    val uid = getUniqueUid()
    private def lockAndTransfer(target: Account, n: Int) = {
        
        // nested synchronized blocks to assure atomicity 
        // on both this and target
        this.synchronized {
            target.synchronized {
                this.amount -= n
                target.amount += n
            }
        }
    }

    // transfer some amount n to a target account
    def transfer(target: Account, n: Int) = {
    
        if (this.uid < target.uid) this.lockAndTransfer(target, n)
        else (target.lockAndTransfer(this, -n))
    }
}

def startThread(a: Account, b: Account, n: Int) = {
    val thread = new Thread {
        override def run() {
            for (i <- 0 until n) {
                a.transfer(b, 1)
            }
        }
    }
    thread.start()
    thread
}

val firstAccount = new Account(1000)
val secondAccount = new Account(400)

val firstThread = startThread(firstAccount, secondAccount, 100)
val secondThread = startThread(secondAccount, firstAccount, 20)

firstThread.join()
secondThread.join()

println(firstAccount.getAmount())
println(secondAccount.getAmount())
