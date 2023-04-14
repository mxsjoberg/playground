import scala.io.Source
// import scala.util.matching.Regex
import scala.io.Codec
import java.nio.charset.CodingErrorAction

object Main {
    // to avoid codec errors
    implicit val codec = Codec("UTF-8")
    codec.onMalformedInput(CodingErrorAction.REPLACE)
    codec.onUnmappableCharacter(CodingErrorAction.REPLACE)
    
    // regex
    val email = """([A-Z|a-z|0-9](\.|_){0,1})+[A-Z|a-z|0-9](\@|\(at\))([A-Z|a-z|0-9])+((\.){0,1}[A-Z|a-z|0-9]){2}\.[a-z]{2,3}""".r
    val script_tag = """<script[^<]+src+[^<]+</script>""".r
    val link_tag = """<link[^<]+href+[^<].+(</link>)*""".r

    // get url content
    val html = Source.fromURL("https://twitter.com")
    val s = html.mkString

    // regex matches
    val matches_email = email.findAllIn(s)
    val matches_script_tag = script_tag.findAllIn(s)
    val matches_link_tag = link_tag.findAllIn(s)

    def main(args: Array[String]) {
        println("Email(s) found:\n")
        matches_email.foreach(println)
        println("\nScript(s) found: \n")
        matches_script_tag.foreach(println)
        println("\nLink(s) found: \n")
        matches_link_tag.foreach(println)
    }
}