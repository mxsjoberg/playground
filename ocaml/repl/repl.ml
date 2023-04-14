(* input stream *)
type stream = { mutable line_num: int; mutable chr: char list; chan: in_channel };;

(* match next char in input *)
let read_char stm =
    match stm.chr with
    (* buffer empty : read char and increment line number newline *)
    | [] ->
        let c = input_char stm.chan in
        if c = '\n' then let _ = stm.line_num <- stm.line_num + 1 in c
        else c
    (* buffer not empty : remove from buffer *)
    | c::rest ->
        let _ = stm.chr <- rest in c

(* concatenate char to front of char buffer *)
let unread_char stm c = stm.chr <- c :: stm.chr;;

(* trim whitespace *)
let is_white c =
    c = ' ' || c = '\t' || c = '\n';;
let rec eat_whitespace stm =
    (* skip whitespace chars *)
    let c = read_char stm in
    if is_white c then
        eat_whitespace stm
    (* push non-whitespace char back to buffer *)
    else
        unread_char stm c;
        ();;

(* types *)
type lobject =
    | Fixnum of int
    | Boolean of bool
    | Symbol of string
    | Nil
    | Pair of lobject * lobject

(* parser *)
exception SyntaxError of string;;
exception UnknownError;;
(* alternative to Char.escaped *)
let charToString c = String.make 1 c;;
let rec read_sexp stm =
    (* check if char is digit *)
    let is_digit c =
        let code = Char.code c in
        code >= Char.code('0') && code <= Char.code('9')
    in
    (* read number *)
    let rec read_fixnum acc =
        let nc = read_char stm in
        if is_digit nc
        (* Char.escaped turns char into string *)
        then read_fixnum (acc ^ (Char.escaped nc))
        else
            let _ = unread_char stm nc in
            Fixnum(int_of_string acc)
    in
    (* read symbol *)
    let is_symstartchar =
        let isalpha = function
            | 'A'..'Z' | 'a'..'z' -> true
            | _ -> false
        in
        function
            | '*' | '/' | '>' | '<' | '=' | '?' | '!' | '-' | '+' -> true
            | c -> isalpha c
    in
    let rec read_symbol () =
        (* escape quote char *)
        let literalQuote = String.get "\"" 0 in
        let is_delimiter = function
            | '(' | ')' | '{' | '}' | ';' -> true
            | c when c = literalQuote -> true
            | c -> is_white c
        in
        let nc = read_char stm in
        (* concatenate chars until delimiter *)
        if is_delimiter nc then let _ = unread_char stm nc in ""
        else charToString nc ^ read_symbol ()
    in
    (* read list *)
    let rec read_list stm =
        eat_whitespace stm;
        let c = read_char stm in
        if c = ')' then Nil
        else
            let _ = unread_char stm c in
            let car = read_sexp stm in
            let cdr = read_list stm in
            Pair(car, cdr)
    in
    (* remove whitespace *)
    eat_whitespace stm;
    (* read char into c *)
    let c = read_char stm in
    (* symbol *)
    if is_symstartchar c then Symbol (charToString c ^ read_symbol ())
    (* number : note that ~ is negation *)
    else if (is_digit c) || (c = '~') then read_fixnum (Char.escaped (if c = '~' then '-' else c))
    (* boolean *)
    else if c = '#' then
        match (read_char stm) with
        | 't' -> Boolean(true)
        | 'f' -> Boolean(false)
        | x -> raise (SyntaxError ("Invalid boolean literal " ^ (Char.escaped x)))
    (* list *)
    else if c = '(' then read_list stm
    (* otherwise : error *)
    else raise (SyntaxError ("Unexpected char " ^ (Char.escaped c)));;

(* repl *)
let rec print_sexp e =
    (* match list *)
    let rec is_list e =
        match e with
        | Nil -> true
        | Pair(a, b) -> is_list b
        | _ -> false
    in
    (* list or pair *)
    let rec print_list l =
        match l with
        | Pair(a, Nil) -> print_sexp a;
        | Pair(a, b) -> print_sexp a; print_string " "; print_list b;
        | _ -> raise UnknownError;
    in
    (* pair *)
    let print_pair p =
        match p with
        | Pair(a, b) -> print_sexp a; print_string " . "; print_sexp b
        | _ -> raise UnknownError;
    in
    (* match types *)
    match e with
    | Fixnum(v) -> print_int v
    | Boolean(b) -> print_string (if b then "#t" else "#f")
    | Symbol(s) -> print_string s
    | Nil -> print_string "nil"
    | Pair(a, b) ->
        print_string "(";
        if is_list e then print_list e
        else print_pair e;
        print_string ")";;
let rec repl stm =
    print_string "> ";
    (* flush stdout to avoid print after input *)
    flush stdout;
    (*let Fixnum(v) = read_sexp stm in*)
    let sexp = read_sexp stm in
    (* print input *)
    print_sexp sexp;
    (*print_int v;*)
    (* print newline *)
    print_newline ();
    (* loop *)
    repl stm;;

(* main *)
let main =
    let stm = { chr=[]; line_num=1; chan=stdin } in
    repl stm;;






