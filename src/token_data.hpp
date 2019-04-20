/* Auto Generated File */
#ifndef TOKEN_DATA_HPP
#define TOKEN_DATA_HPP

#include "token.hpp"

#define ASCII_SET 128

namespace porc::internals {

static const char *tokenToStrMap[(int)Token::NumTokens] = {
  [(int)Token::Comma] = ",",
  [(int)Token::SemiColon] = ";",
  [(int)Token::LeftParen] = "(",
  [(int)Token::RightParen] = ")",
  [(int)Token::LeftBrace] = "{",
  [(int)Token::RightBrace] = "}",
  [(int)Token::LeftBracket] = "[",
  [(int)Token::RightBracket] = "]",
  [(int)Token::LineComment] = "#",
  [(int)Token::BlockComment] = "/*",
  [(int)Token::LessThan] = "<",
  [(int)Token::GreaterThan] = ">",
  [(int)Token::Equal] = "==",
  [(int)Token::NotEqual] = "!=",
  [(int)Token::LessThanEqual] = "<=",
  [(int)Token::GreaterThanEqual] = ">=",
  [(int)Token::Negate] = "!",
  [(int)Token::And] = "&&",
  [(int)Token::Or] = "||",
  [(int)Token::Add] = "+",
  [(int)Token::Subtract] = "-",
  [(int)Token::Divide] = "/",
  [(int)Token::Multiply] = "*",
  [(int)Token::Power] = "**",
  [(int)Token::Modulus] = "%",
  [(int)Token::IntegerDivide] = "//",
  [(int)Token::Assign] = "=",
  [(int)Token::AddAssign] = "+=",
  [(int)Token::SubtractAssign] = "-=",
  [(int)Token::MultiplyAssign] = "*=",
  [(int)Token::DivideAssign] = "/=",
  [(int)Token::PowerAssign] = "**=",
  [(int)Token::IntegerDivideAssign] = "%/=",
  [(int)Token::ModulusAssign] = "%=",
  [(int)Token::FatArrow] = "=>",
  [(int)Token::LeftArrow] = "<|",
  [(int)Token::RightArrow] = "|>",
  [(int)Token::ReturnType] = "->",
  [(int)Token::Colon] = ":",
  [(int)Token::DoubleColon] = "::",
  [(int)Token::Implements] = "^",
  [(int)Token::Variant] = "|",
  [(int)Token::Dot] = ".",
  [(int)Token::Range] = "..",
  [(int)Token::Macro] = "@",
  [(int)Token::True] = "true",
  [(int)Token::False] = "false",
  [(int)Token::Void] = "void",
  [(int)Token::Const] = "const",
  [(int)Token::Struct] = "struct",
  [(int)Token::Func] = "fn",
  [(int)Token::Var] = "var",
  [(int)Token::Return] = "return",
  [(int)Token::While] = "while",
  [(int)Token::For] = "for",
  [(int)Token::Break] = "break",
  [(int)Token::Continue] = "continue",
  [(int)Token::In] = "in",
  [(int)Token::If] = "if",
  [(int)Token::Else] = "else",
};

static const char *tokenToNameMap[(int)Token::NumTokens] = {
  [(int)Token::Undefined] = "Undefined",
  [(int)Token::Identifier] = "Identifier",
  [(int)Token::Str] = "Str",
  [(int)Token::Flt] = "Flt",
  [(int)Token::Int] = "Int",
  [(int)Token::Char] = "Char",
  [(int)Token::EndOfFile] = "EndOfFile",
  [(int)Token::Comma] = "Comma",
  [(int)Token::SemiColon] = "SemiColon",
  [(int)Token::LeftParen] = "LeftParen",
  [(int)Token::RightParen] = "RightParen",
  [(int)Token::LeftBrace] = "LeftBrace",
  [(int)Token::RightBrace] = "RightBrace",
  [(int)Token::LeftBracket] = "LeftBracket",
  [(int)Token::RightBracket] = "RightBracket",
  [(int)Token::LineComment] = "LineComment",
  [(int)Token::BlockComment] = "BlockComment",
  [(int)Token::LessThan] = "LessThan",
  [(int)Token::GreaterThan] = "GreaterThan",
  [(int)Token::Equal] = "Equal",
  [(int)Token::NotEqual] = "NotEqual",
  [(int)Token::LessThanEqual] = "LessThanEqual",
  [(int)Token::GreaterThanEqual] = "GreaterThanEqual",
  [(int)Token::Negate] = "Negate",
  [(int)Token::And] = "And",
  [(int)Token::Or] = "Or",
  [(int)Token::Add] = "Add",
  [(int)Token::Subtract] = "Subtract",
  [(int)Token::Divide] = "Divide",
  [(int)Token::Multiply] = "Multiply",
  [(int)Token::Power] = "Power",
  [(int)Token::Modulus] = "Modulus",
  [(int)Token::IntegerDivide] = "IntegerDivide",
  [(int)Token::Assign] = "Assign",
  [(int)Token::AddAssign] = "AddAssign",
  [(int)Token::SubtractAssign] = "SubtractAssign",
  [(int)Token::MultiplyAssign] = "MultiplyAssign",
  [(int)Token::DivideAssign] = "DivideAssign",
  [(int)Token::PowerAssign] = "PowerAssign",
  [(int)Token::IntegerDivideAssign] = "IntegerDivideAssign",
  [(int)Token::ModulusAssign] = "ModulusAssign",
  [(int)Token::FatArrow] = "FatArrow",
  [(int)Token::LeftArrow] = "LeftArrow",
  [(int)Token::RightArrow] = "RightArrow",
  [(int)Token::ReturnType] = "ReturnType",
  [(int)Token::Colon] = "Colon",
  [(int)Token::DoubleColon] = "DoubleColon",
  [(int)Token::Implements] = "Implements",
  [(int)Token::Variant] = "Variant",
  [(int)Token::Dot] = "Dot",
  [(int)Token::Range] = "Range",
  [(int)Token::Macro] = "Macro",
  [(int)Token::True] = "True",
  [(int)Token::False] = "False",
  [(int)Token::Void] = "Void",
  [(int)Token::Const] = "Const",
  [(int)Token::Struct] = "Struct",
  [(int)Token::Func] = "Func",
  [(int)Token::Var] = "Var",
  [(int)Token::Return] = "Return",
  [(int)Token::While] = "While",
  [(int)Token::For] = "For",
  [(int)Token::Break] = "Break",
  [(int)Token::Continue] = "Continue",
  [(int)Token::In] = "In",
  [(int)Token::If] = "If",
  [(int)Token::Else] = "Else",
};

struct TokenSet {
    const int *tokens;
    const TokenSet *child_tokens;
};

static const TokenSet tokenFromStrMap = {
  (int[ASCII_SET]){
    [','] = (int)Token::Comma,
    [';'] = (int)Token::SemiColon,
    ['('] = (int)Token::LeftParen,
    [')'] = (int)Token::RightParen,
    ['{'] = (int)Token::LeftBrace,
    ['}'] = (int)Token::RightBrace,
    ['['] = (int)Token::LeftBracket,
    [']'] = (int)Token::RightBracket,
    ['#'] = (int)Token::LineComment,
    ['<'] = (int)Token::LessThan,
    ['>'] = (int)Token::GreaterThan,
    ['!'] = (int)Token::Negate,
    ['+'] = (int)Token::Add,
    ['-'] = (int)Token::Subtract,
    ['/'] = (int)Token::Divide,
    ['*'] = (int)Token::Multiply,
    ['%'] = (int)Token::Modulus,
    ['='] = (int)Token::Assign,
    [':'] = (int)Token::Colon,
    ['^'] = (int)Token::Implements,
    ['|'] = (int)Token::Variant,
    ['.'] = (int)Token::Dot,
    ['@'] = (int)Token::Macro,
  },
  (TokenSet[ASCII_SET]){
    ['/'] = {
      (int[ASCII_SET]){
        ['*'] = (int)Token::BlockComment,
        ['/'] = (int)Token::IntegerDivide,
        ['='] = (int)Token::DivideAssign,
      },
      NULL,
    },
    ['='] = {
      (int[ASCII_SET]){
        ['='] = (int)Token::Equal,
        ['>'] = (int)Token::FatArrow,
      },
      NULL,
    },
    ['!'] = {
      (int[ASCII_SET]){
        ['='] = (int)Token::NotEqual,
      },
      NULL,
    },
    ['<'] = {
      (int[ASCII_SET]){
        ['='] = (int)Token::LessThanEqual,
        ['|'] = (int)Token::LeftArrow,
      },
      NULL,
    },
    ['>'] = {
      (int[ASCII_SET]){
        ['='] = (int)Token::GreaterThanEqual,
      },
      NULL,
    },
    ['&'] = {
      (int[ASCII_SET]){
        ['&'] = (int)Token::And,
      },
      NULL,
    },
    ['|'] = {
      (int[ASCII_SET]){
        ['|'] = (int)Token::Or,
        ['>'] = (int)Token::RightArrow,
      },
      NULL,
    },
    ['*'] = {
      (int[ASCII_SET]){
        ['*'] = (int)Token::Power,
        ['='] = (int)Token::MultiplyAssign,
      },
      (TokenSet[ASCII_SET]){
        ['*'] = {
          (int[ASCII_SET]){
            ['='] = (int)Token::PowerAssign,
          },
          NULL,
        },
      },
    },
    ['+'] = {
      (int[ASCII_SET]){
        ['='] = (int)Token::AddAssign,
      },
      NULL,
    },
    ['-'] = {
      (int[ASCII_SET]){
        ['='] = (int)Token::SubtractAssign,
        ['>'] = (int)Token::ReturnType,
      },
      NULL,
    },
    ['%'] = {
      (int[ASCII_SET]){
        ['='] = (int)Token::ModulusAssign,
      },
      (TokenSet[ASCII_SET]){
        ['/'] = {
          (int[ASCII_SET]){
            ['='] = (int)Token::IntegerDivideAssign,
          },
          NULL,
        },
      },
    },
    [':'] = {
      (int[ASCII_SET]){
        [':'] = (int)Token::DoubleColon,
      },
      NULL,
    },
    ['.'] = {
      (int[ASCII_SET]){
        ['.'] = (int)Token::Range,
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
                ['e'] = (int)Token::True,
              },
              NULL,
            },
          },
        },
      },
    },
    ['f'] = {
      (int[ASCII_SET]){
        ['n'] = (int)Token::Func,
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
                    ['e'] = (int)Token::False,
                  },
                  NULL,
                },
              },
            },
          },
        },
        ['o'] = {
          (int[ASCII_SET]){
            ['r'] = (int)Token::For,
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
                ['d'] = (int)Token::Void,
              },
              NULL,
            },
          },
        },
        ['a'] = {
          (int[ASCII_SET]){
            ['r'] = (int)Token::Var,
          },
          NULL,
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
                    ['t'] = (int)Token::Const,
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
                                ['e'] = (int)Token::Continue,
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
                        ['t'] = (int)Token::Struct,
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
                        ['n'] = (int)Token::Return,
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
                    ['e'] = (int)Token::While,
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
                    ['k'] = (int)Token::Break,
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
        ['n'] = (int)Token::In,
        ['f'] = (int)Token::If,
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
                ['e'] = (int)Token::Else,
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
