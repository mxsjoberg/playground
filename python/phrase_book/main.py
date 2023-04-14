# -*- coding: utf-8 -*-

import sys
import random

# print(sys.version)

# https://www.fluentin3months.com/common-spanish-phrases/
GREETINGS = {
    "Hola": "Hello",
    "¿Qué tal?": "How are you?",
    "¿Qué pasa?": "What’s up?",
    "¿Cómo te va?": "How’re you doing?",
    "Bien": "Good",
    "Muy bien": "Very good",
    "Así así": "So-so",
    "No tan bien": "Not so good",
    "¿Y tú?": "And you?",
    "Me llamo...": "My name is...",
    "¿Cómo te llamas?": "What’s your name?",
    "Mucho gusto": "Nice to meet you",
    "Placér": "A pleasure",
    "Encantado(a)": "Charmed/ Likewise",
    "Nos vemos mañana": "See you tomorrow",
    "Hasta luego": "See you later",
    "Hasta pronto amigo": "See you soon, friend",
    "Gracias": "Thank you",
    "De nada": "You’re welcome",
    "No hay de qué": "No problem",
    "Disculpe": "Excuse me",
    "Lo siento": "I’m sorry"
}

QUESTIONS = {
    "¿Cuánto cuesta?": "How much is this?",
    "¿Dónde está el baño?": "Where’s the bathroom?",
    "¿Qué hora es?": "What time is it?",
    "¿Pasa algo?": "Is something wrong?",
    "¿Es esto correcto?": "Is this right?",
    "¿Me equivocado?": "Was I wrong?",
    "¿Me puede ayudar con esto?": "Can you help me with this?",
    "¿Puedes traerme ... por favor?": "Can you bring me ... please?",
    "¿Puedo entrar?": "Can I come in?",
    "¿Quieres tomar una copa?": "Want to grab a drink?",
    "¿A dónde deberíamos ir a comer?": "Where should we go to eat?",
    "¿Estás listo?": "Are you ready?"
}

EXCLAMATIONS = {
    "¡Cuánto tiempo sin verlo(a)!": "Long time no see!",
    "¡Feliz cumpleaños!": "Happy birthday!",
    "¡Buena suerte!": "Good luck!",
    "¡Alto!": "Stop!",
    "¡Salud!": "Cheers!",
    "Que te mejores": "Get well soon",
    "Buen provecho": "Bon appetit",
    "Cuídate": "Take care",
    "Felicitaciones": "Congratulations",
    "¡Bien hecho!": "Well done!",
    "¡Genio!": "Genius!",
    "Estupendo": "Stupendous/ Amazing",
    "Genial": "Great/ Awesome",
    "¡Increíble!": "Incredible!/ Impressive!"
}

FILLERS = {
    "A ver...": "Let’s see...",
    "Pues...": "Well...",
    "Bueno...": "Well then...",
    "¿Sabes?": "You know?",
    "Por supuesto": "Of course",
    "Por otra parte...": "On another note...",
    "Pero...": "But...",
    "De verdad?": "Really?",
    "Dios mio": "Oh my god",
    "Entonces...": "So...",
    "Asi que...": "So... About that..."
}

HELPFUL = {
    "Necesito ayuda": "I need help",
    "Llámame cuando llegues": "Call me when you arrive",
    "Me voy a casa": "I’m going home",
    "Necesito ir a...": "I need to go to...",
    "¿Como llego hasta ahí?": "How do I get there?",
    "No lo sé": "I don´t know",
    "No tengo idea": "I have no idea",
    "¿Lo entiendes?": "Do you understand?",
    "No entiendo": "I don’t understand.",
    "Quiero...": "I want...",
    "¿Puede hablar más despacio, por favor?": "Can you speak slowly, please?"
}

ANY = { **GREETINGS, **QUESTIONS, **EXCLAMATIONS, **FILLERS, **HELPFUL }

def loop(desc, dct = ANY):
    print("\nSelected: " + str(desc) + "\n")
    print("Press ENTER to continue - press any other key followed by ENTER to exit")
    while True:
        # next item
        if (not input()):
            item = random.choice(list(dct.items()))
            print("---\n")
            print(item[0])
            # translation
            if (not input()):
                print(item[1])
            else:
                sys.exit()
        else:
            sys.exit()

if (__name__ == "__main__"):
    print("\n")
    print("1: Greetings\n")
    print("2: Questions\n")
    print("3: Exclamations\n")
    print("4: Fillers\n")
    print("5: Helpful\n")
    print("...or press ENTER for any.\n")
    input_ = input("> ")
    if (input_ == "1"):
        loop("Greetings", GREETINGS)
    elif (input_ == "2"):
        loop("Questions", QUESTIONS)
    elif (input_ == "3"):
        loop("Exclamations", EXCLAMATIONS)
    elif (input_ == "4"):
        loop("Fillers", FILLERS)
    elif (input_ == "5"):
        loop("Helpful", HELPFUL)
    else:
        loop("Any")
