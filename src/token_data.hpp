/* Auto Generated File */
#ifndef TOKEN_DATA_HPP
#define TOKEN_DATA_HPP

#include "token.hpp"

#define ASCII_SET 128

namespace porc::internals {

static const char *tokenToStrMap[(int)Token::Kind::NumTokens] = {
  [(int)Token::Kind::Comma] = ",",
  [(int)Token::Kind::SemiColon] = ";",
  [(int)Token::Kind::LeftParen] = "(",
  [(int)Token::Kind::RightParen] = ")",
  [(int)Token::Kind::LeftBrace] = "{",
  [(int)Token::Kind::RightBrace] = "}",
  [(int)Token::Kind::LeftBracket] = "[",
  [(int)Token::Kind::RightBracket] = "]",
  [(int)Token::Kind::LineComment] = "//",
  [(int)Token::Kind::BlockComment] = "/*",
  [(int)Token::Kind::LessThan] = "<",
  [(int)Token::Kind::GreaterThan] = ">",
  [(int)Token::Kind::Equal] = "==",
  [(int)Token::Kind::NotEqual] = "!=",
  [(int)Token::Kind::LessThanEqual] = "<=",
  [(int)Token::Kind::GreaterThanEqual] = ">=",
  [(int)Token::Kind::Negate] = "!",
  [(int)Token::Kind::And] = "&&",
  [(int)Token::Kind::Or] = "||",
  [(int)Token::Kind::Increment] = "++",
  [(int)Token::Kind::Decrement] = "--",
  [(int)Token::Kind::Add] = "+",
  [(int)Token::Kind::Subtract] = "-",
  [(int)Token::Kind::Divide] = "/",
  [(int)Token::Kind::Multiply] = "*",
  [(int)Token::Kind::Power] = "**",
  [(int)Token::Kind::Modulus] = "%",
  [(int)Token::Kind::IntegerDivide] = "%/",
  [(int)Token::Kind::Assign] = "=",
  [(int)Token::Kind::AddAssign] = "+=",
  [(int)Token::Kind::SubtractAssign] = "-=",
  [(int)Token::Kind::MultiplyAssign] = "*=",
  [(int)Token::Kind::DivideAssign] = "/=",
  [(int)Token::Kind::PowerAssign] = "**=",
  [(int)Token::Kind::IntegerDivideAssign] = "%/=",
  [(int)Token::Kind::ModulusAssign] = "%=",
  [(int)Token::Kind::FatArrow] = "=>",
  [(int)Token::Kind::LeftArrow] = "<|",
  [(int)Token::Kind::RightArrow] = "|>",
  [(int)Token::Kind::Colon] = ":",
  [(int)Token::Kind::DoubleColon] = "::",
  [(int)Token::Kind::Implements] = "^",
  [(int)Token::Kind::Variant] = "|",
  [(int)Token::Kind::Dot] = ".",
  [(int)Token::Kind::Range] = "..",
  [(int)Token::Kind::Macro] = "@",
  [(int)Token::Kind::Ternary] = "?",
  [(int)Token::Kind::True] = "true",
  [(int)Token::Kind::False] = "false",
  [(int)Token::Kind::Void] = "void",
  [(int)Token::Kind::Const] = "const",
  [(int)Token::Kind::Struct] = "struct",
  [(int)Token::Kind::Func] = "fn",
  [(int)Token::Kind::Let] = "let",
  [(int)Token::Kind::Return] = "return",
  [(int)Token::Kind::While] = "while",
  [(int)Token::Kind::For] = "for",
  [(int)Token::Kind::Break] = "break",
  [(int)Token::Kind::Continue] = "continue",
  [(int)Token::Kind::In] = "in",
  [(int)Token::Kind::If] = "if",
  [(int)Token::Kind::Else] = "else",
};

static const char *tokenToNameMap[(int)Token::Kind::NumTokens] = {
  [(int)Token::Kind::Undefined] = "Undefined",
  [(int)Token::Kind::Identifier] = "Identifier",
  [(int)Token::Kind::Str] = "Str",
  [(int)Token::Kind::Flt] = "Flt",
  [(int)Token::Kind::Int] = "Int",
  [(int)Token::Kind::Char] = "Char",
  [(int)Token::Kind::EndOfFile] = "EndOfFile",
  [(int)Token::Kind::Comma] = "Comma",
  [(int)Token::Kind::SemiColon] = "SemiColon",
  [(int)Token::Kind::LeftParen] = "LeftParen",
  [(int)Token::Kind::RightParen] = "RightParen",
  [(int)Token::Kind::LeftBrace] = "LeftBrace",
  [(int)Token::Kind::RightBrace] = "RightBrace",
  [(int)Token::Kind::LeftBracket] = "LeftBracket",
  [(int)Token::Kind::RightBracket] = "RightBracket",
  [(int)Token::Kind::LineComment] = "LineComment",
  [(int)Token::Kind::BlockComment] = "BlockComment",
  [(int)Token::Kind::LessThan] = "LessThan",
  [(int)Token::Kind::GreaterThan] = "GreaterThan",
  [(int)Token::Kind::Equal] = "Equal",
  [(int)Token::Kind::NotEqual] = "NotEqual",
  [(int)Token::Kind::LessThanEqual] = "LessThanEqual",
  [(int)Token::Kind::GreaterThanEqual] = "GreaterThanEqual",
  [(int)Token::Kind::Negate] = "Negate",
  [(int)Token::Kind::And] = "And",
  [(int)Token::Kind::Or] = "Or",
  [(int)Token::Kind::Increment] = "Increment",
  [(int)Token::Kind::Decrement] = "Decrement",
  [(int)Token::Kind::Add] = "Add",
  [(int)Token::Kind::Subtract] = "Subtract",
  [(int)Token::Kind::Divide] = "Divide",
  [(int)Token::Kind::Multiply] = "Multiply",
  [(int)Token::Kind::Power] = "Power",
  [(int)Token::Kind::Modulus] = "Modulus",
  [(int)Token::Kind::IntegerDivide] = "IntegerDivide",
  [(int)Token::Kind::Assign] = "Assign",
  [(int)Token::Kind::AddAssign] = "AddAssign",
  [(int)Token::Kind::SubtractAssign] = "SubtractAssign",
  [(int)Token::Kind::MultiplyAssign] = "MultiplyAssign",
  [(int)Token::Kind::DivideAssign] = "DivideAssign",
  [(int)Token::Kind::PowerAssign] = "PowerAssign",
  [(int)Token::Kind::IntegerDivideAssign] = "IntegerDivideAssign",
  [(int)Token::Kind::ModulusAssign] = "ModulusAssign",
  [(int)Token::Kind::FatArrow] = "FatArrow",
  [(int)Token::Kind::LeftArrow] = "LeftArrow",
  [(int)Token::Kind::RightArrow] = "RightArrow",
  [(int)Token::Kind::Colon] = "Colon",
  [(int)Token::Kind::DoubleColon] = "DoubleColon",
  [(int)Token::Kind::Implements] = "Implements",
  [(int)Token::Kind::Variant] = "Variant",
  [(int)Token::Kind::Dot] = "Dot",
  [(int)Token::Kind::Range] = "Range",
  [(int)Token::Kind::Macro] = "Macro",
  [(int)Token::Kind::Ternary] = "Ternary",
  [(int)Token::Kind::True] = "True",
  [(int)Token::Kind::False] = "False",
  [(int)Token::Kind::Void] = "Void",
  [(int)Token::Kind::Const] = "Const",
  [(int)Token::Kind::Struct] = "Struct",
  [(int)Token::Kind::Func] = "Func",
  [(int)Token::Kind::Let] = "Let",
  [(int)Token::Kind::Return] = "Return",
  [(int)Token::Kind::While] = "While",
  [(int)Token::Kind::For] = "For",
  [(int)Token::Kind::Break] = "Break",
  [(int)Token::Kind::Continue] = "Continue",
  [(int)Token::Kind::In] = "In",
  [(int)Token::Kind::If] = "If",
  [(int)Token::Kind::Else] = "Else",
};

struct TokenSet {
    const int *tokens;
    const TokenSet *child_tokens;
};

static const TokenSet tokenFromStrMap = {
  (int[ASCII_SET]){
    [','] = (int)Token::Kind::Comma,
    [';'] = (int)Token::Kind::SemiColon,
    ['('] = (int)Token::Kind::LeftParen,
    [')'] = (int)Token::Kind::RightParen,
    ['{'] = (int)Token::Kind::LeftBrace,
    ['}'] = (int)Token::Kind::RightBrace,
    ['['] = (int)Token::Kind::LeftBracket,
    [']'] = (int)Token::Kind::RightBracket,
    ['<'] = (int)Token::Kind::LessThan,
    ['>'] = (int)Token::Kind::GreaterThan,
    ['!'] = (int)Token::Kind::Negate,
    ['+'] = (int)Token::Kind::Add,
    ['-'] = (int)Token::Kind::Subtract,
    ['/'] = (int)Token::Kind::Divide,
    ['*'] = (int)Token::Kind::Multiply,
    ['%'] = (int)Token::Kind::Modulus,
    ['='] = (int)Token::Kind::Assign,
    [':'] = (int)Token::Kind::Colon,
    ['^'] = (int)Token::Kind::Implements,
    ['|'] = (int)Token::Kind::Variant,
    ['.'] = (int)Token::Kind::Dot,
    ['@'] = (int)Token::Kind::Macro,
    ['?'] = (int)Token::Kind::Ternary,
  },
  (TokenSet[ASCII_SET]){
    ['/'] = {
      (int[ASCII_SET]){
        ['/'] = (int)Token::Kind::LineComment,
        ['*'] = (int)Token::Kind::BlockComment,
        ['='] = (int)Token::Kind::DivideAssign,
      },
      NULL,
    },
    ['='] = {
      (int[ASCII_SET]){
        ['='] = (int)Token::Kind::Equal,
        ['>'] = (int)Token::Kind::FatArrow,
      },
      NULL,
    },
    ['!'] = {
      (int[ASCII_SET]){
        ['='] = (int)Token::Kind::NotEqual,
      },
      NULL,
    },
    ['<'] = {
      (int[ASCII_SET]){
        ['='] = (int)Token::Kind::LessThanEqual,
        ['|'] = (int)Token::Kind::LeftArrow,
      },
      NULL,
    },
    ['>'] = {
      (int[ASCII_SET]){
        ['='] = (int)Token::Kind::GreaterThanEqual,
      },
      NULL,
    },
    ['&'] = {
      (int[ASCII_SET]){
        ['&'] = (int)Token::Kind::And,
      },
      NULL,
    },
    ['|'] = {
      (int[ASCII_SET]){
        ['|'] = (int)Token::Kind::Or,
        ['>'] = (int)Token::Kind::RightArrow,
      },
      NULL,
    },
    ['+'] = {
      (int[ASCII_SET]){
        ['+'] = (int)Token::Kind::Increment,
        ['='] = (int)Token::Kind::AddAssign,
      },
      NULL,
    },
    ['-'] = {
      (int[ASCII_SET]){
        ['-'] = (int)Token::Kind::Decrement,
        ['='] = (int)Token::Kind::SubtractAssign,
      },
      NULL,
    },
    ['*'] = {
      (int[ASCII_SET]){
        ['*'] = (int)Token::Kind::Power,
        ['='] = (int)Token::Kind::MultiplyAssign,
      },
      (TokenSet[ASCII_SET]){
        ['*'] = {
          (int[ASCII_SET]){
            ['='] = (int)Token::Kind::PowerAssign,
          },
          NULL,
        },
      },
    },
    ['%'] = {
      (int[ASCII_SET]){
        ['/'] = (int)Token::Kind::IntegerDivide,
        ['='] = (int)Token::Kind::ModulusAssign,
      },
      (TokenSet[ASCII_SET]){
        ['/'] = {
          (int[ASCII_SET]){
            ['='] = (int)Token::Kind::IntegerDivideAssign,
          },
          NULL,
        },
      },
    },
    [':'] = {
      (int[ASCII_SET]){
        [':'] = (int)Token::Kind::DoubleColon,
      },
      NULL,
    },
    ['.'] = {
      (int[ASCII_SET]){
        ['.'] = (int)Token::Kind::Range,
      },
      NULL,
    },
    ['t'] = {
      NULL,
      (TokenSet[ASCII_SET]){
        ['r'] = {
          NULL,
          (TokenSet[ASCII_SET]){
            ['u'] = {
              (int[ASCII_SET]){
                ['e'] = (int)Token::Kind::True,
              },
              NULL,
            },
          },
        },
      },
    },
    ['f'] = {
      (int[ASCII_SET]){
        ['n'] = (int)Token::Kind::Func,
      },
      (TokenSet[ASCII_SET]){
        ['a'] = {
          NULL,
          (TokenSet[ASCII_SET]){
            ['l'] = {
              NULL,
              (TokenSet[ASCII_SET]){
                ['s'] = {
                  (int[ASCII_SET]){
                    ['e'] = (int)Token::Kind::False,
                  },
                  NULL,
                },
              },
            },
          },
        },
        ['o'] = {
          (int[ASCII_SET]){
            ['r'] = (int)Token::Kind::For,
          },
          NULL,
        },
      },
    },
    ['v'] = {
      NULL,
      (TokenSet[ASCII_SET]){
        ['o'] = {
          NULL,
          (TokenSet[ASCII_SET]){
            ['i'] = {
              (int[ASCII_SET]){
                ['d'] = (int)Token::Kind::Void,
              },
              NULL,
            },
          },
        },
      },
    },
    ['c'] = {
      NULL,
      (TokenSet[ASCII_SET]){
        ['o'] = {
          NULL,
          (TokenSet[ASCII_SET]){
            ['n'] = {
              NULL,
              (TokenSet[ASCII_SET]){
                ['s'] = {
                  (int[ASCII_SET]){
                    ['t'] = (int)Token::Kind::Const,
                  },
                  NULL,
                },
                ['t'] = {
                  NULL,
                  (TokenSet[ASCII_SET]){
                    ['i'] = {
                      NULL,
                      (TokenSet[ASCII_SET]){
                        ['n'] = {
                          NULL,
                          (TokenSet[ASCII_SET]){
                            ['u'] = {
                              (int[ASCII_SET]){
                                ['e'] = (int)Token::Kind::Continue,
                              },
                              NULL,
                            },
                          },
                        },
                      },
                    },
                  },
                },
              },
            },
          },
        },
      },
    },
    ['s'] = {
      NULL,
      (TokenSet[ASCII_SET]){
        ['t'] = {
          NULL,
          (TokenSet[ASCII_SET]){
            ['r'] = {
              NULL,
              (TokenSet[ASCII_SET]){
                ['u'] = {
                  NULL,
                  (TokenSet[ASCII_SET]){
                    ['c'] = {
                      (int[ASCII_SET]){
                        ['t'] = (int)Token::Kind::Struct,
                      },
                      NULL,
                    },
                  },
                },
              },
            },
          },
        },
      },
    },
    ['l'] = {
      NULL,
      (TokenSet[ASCII_SET]){
        ['e'] = {
          (int[ASCII_SET]){
            ['t'] = (int)Token::Kind::Let,
          },
          NULL,
        },
      },
    },
    ['r'] = {
      NULL,
      (TokenSet[ASCII_SET]){
        ['e'] = {
          NULL,
          (TokenSet[ASCII_SET]){
            ['t'] = {
              NULL,
              (TokenSet[ASCII_SET]){
                ['u'] = {
                  NULL,
                  (TokenSet[ASCII_SET]){
                    ['r'] = {
                      (int[ASCII_SET]){
                        ['n'] = (int)Token::Kind::Return,
                      },
                      NULL,
                    },
                  },
                },
              },
            },
          },
        },
      },
    },
    ['w'] = {
      NULL,
      (TokenSet[ASCII_SET]){
        ['h'] = {
          NULL,
          (TokenSet[ASCII_SET]){
            ['i'] = {
              NULL,
              (TokenSet[ASCII_SET]){
                ['l'] = {
                  (int[ASCII_SET]){
                    ['e'] = (int)Token::Kind::While,
                  },
                  NULL,
                },
              },
            },
          },
        },
      },
    },
    ['b'] = {
      NULL,
      (TokenSet[ASCII_SET]){
        ['r'] = {
          NULL,
          (TokenSet[ASCII_SET]){
            ['e'] = {
              NULL,
              (TokenSet[ASCII_SET]){
                ['a'] = {
                  (int[ASCII_SET]){
                    ['k'] = (int)Token::Kind::Break,
                  },
                  NULL,
                },
              },
            },
          },
        },
      },
    },
    ['i'] = {
      (int[ASCII_SET]){
        ['n'] = (int)Token::Kind::In,
        ['f'] = (int)Token::Kind::If,
      },
      NULL,
    },
    ['e'] = {
      NULL,
      (TokenSet[ASCII_SET]){
        ['l'] = {
          NULL,
          (TokenSet[ASCII_SET]){
            ['s'] = {
              (int[ASCII_SET]){
                ['e'] = (int)Token::Kind::Else,
              },
              NULL,
            },
          },
        },
      },
    },
  },
};

}

#endif
