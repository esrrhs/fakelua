use object::TString;
use zio::{Zio, zgetc};

const FIRST_RESERVED: u8 = std::u8::MAX;

enum RESERVED {
    /* terminal symbols denoted by reserved words */
    TK_AND = FIRST_RESERVED as isize,
    TK_BREAK,
    TK_DO,
    TK_ELSE,
    TK_ELSEIF,
    TK_END,
    TK_FALSE,
    TK_FOR,
    TK_FUNCTION,
    TK_GOTO,
    TK_IF,
    TK_IN,
    TK_LOCAL,
    TK_NIL,
    TK_NOT,
    TK_OR,
    TK_REPEAT,
    TK_RETURN,
    TK_THEN,
    TK_TRUE,
    TK_UNTIL,
    TK_WHILE,
    /* other terminal symbols */
    TK_IDIV,
    TK_CONCAT,
    TK_DOTS,
    TK_EQ,
    TK_GE,
    TK_LE,
    TK_NE,
    TK_SHL,
    TK_SHR,
    TK_DBCOLON,
    TK_EOS,
    TK_FLT,
    TK_INT,
    TK_NAME,
    TK_STRING,
}

/* semantics information */
struct SemInfo {
    r: f64,
    i: i64,
    ts: TString,
}

struct Token {
    token: u32,
    seminfo: SemInfo,
}


struct LexState {
    /* current character (charint) */
    current: u32,
    /* input line counter */
    linenumber: u32,
    /* line of last token 'consumed' */
    lastline: u32,
    /* current token */
    t: Token,
    /* look ahead token */
    lookahead: Token,
    /* input stream */
    z: Zio,
}

const const_r: u32 = '\r' as u32;
const const_n: u32 = '\n' as u32;

fn next(ls: &mut LexState) {
    ls.current = zgetc(&mut ls.z)
}

fn currIsNewline(ls: &LexState) -> bool {
    ls.current == const_r || ls.current == const_n
}

fn inclinenumber(ls: &mut LexState) {
    let old = ls.current;
    next(ls);  /* skip '\n' or '\r' */
    if currIsNewline(ls) && ls.current != old {
        next(ls);  /* skip '\n\r' or '\r\n' */
    }
    ls.linenumber += 1;
}

fn llex(ls: &mut LexState) {
    loop {
        match ls.current {
            const_r | const_n => {
                inclinenumber(ls);
                break;
            }
            _ => {}
        }
    }
}
