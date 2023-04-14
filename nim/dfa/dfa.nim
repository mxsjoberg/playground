# dfa

var
    text: string
    ch: char
    pos: int = 0

proc nextChar() =
    if pos < text.len:
        ch = text[pos]
        inc pos
    else:
        ch = '\0'

# input
text = "print 42\0"

# init
nextChar()

# main loop
while pos < text.len:
    # accepting states
    case ch
    of 'a'..'z', 'A'..'Z':
        echo "letter"
    of '0'..'9':
        echo "digit"
    of ' ', '\t':
        echo "whitespace"
    of '\0':
        echo "EOF"
    else:
        echo "Unknown"

    nextChar()
