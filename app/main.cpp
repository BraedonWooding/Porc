#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <csignal>
#include <iostream>
#include <string>
#include <vector>

#include <CLI11.hpp>
#include <rang.hpp>

#include <defs.hpp>
#include <parser.hpp>
#include <token_stream.hpp>

void signal_handler(int s);
void print_version();

int main(int argc, char *argv[]) {
#ifdef _WIN32
  signal(SIGINT, &signal_handler);
#else
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = &signal_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, nullptr);
#endif

  CLI::App app{"Porc, the compiler/interpreter for Porc"};
  std::vector<std::string> filenames;
  app.add_option("input files,-i", filenames, "Input Files")
      ->required(true)
      ->check(CLI::ExistingFile);
  app.require_subcommand(1);
  bool verbose = false;
  app.add_flag("-v,--verbose", verbose, "Verbose output");

  auto run = app.add_subcommand("run", "Run the given porc script")
                 ->ignore_case()
                 ->fallthrough();
  run->callback([&]() {
    if (verbose) std::cout << "Running subcommand `run`";
    if (verbose) std::cout << "Parsing files";
    // @PARALLELISE
    // CompilationUnit unit(filenames);
    using namespace porc;

    err::PipeOutput(std::cerr);
    for (auto file : filenames) {
      TokenStream stream(std::make_unique<CFileReader>(file.c_str()));
      Parser parser = Parser(std::move(stream));
      auto top_level = parser.ParseFileDecl();
      if (!top_level) {
        std::cerr << rang::fg::red << "Error couldn't parse file: " << file
                  << " due to errors shown above" << rang::style::reset
                  << std::endl;
      } else {
        // unit.Compile(std::move(*top_level));
      }
    }
  });

  auto build = app.add_subcommand("parse", "Parses the files")
                   ->ignore_case()
                   ->fallthrough();
  auto dev = app.add_subcommand("dev", "A series of development options")
                 ->ignore_case()
                 ->fallthrough();
  auto ast =
      dev->add_subcommand("ast", "Builds an AST")->ignore_case()->fallthrough();
  bool ast_output = false;
  ast->add_flag("-o,--output", ast_output,
                "Converts all the files then puts them into .json files rather "
                "than printing");
  ast->callback([&]() {
    if (verbose) std::cout << "Running subcommand `dev/ast`\n";
    for (auto file : filenames) {
      using namespace porc;

      TokenStream stream(std::make_unique<CFileReader>(file.c_str()));
      err::PipeOutput(std::cerr);
      stream.ignore_comments = true;
      Parser parser = Parser(std::move(stream));
      auto top_level = parser.ParseFileDecl();
      if (!top_level) {
        std::cerr << rang::fg::red << "Error couldn't parse file: " << file
                  << " due to errors shown above" << rang::style::reset
                  << std::endl;
      } else if (ast_output) {
        std::fstream out;
        out.open(file + ".json", std::ios::out);
        if (!out) {
          std::cerr << rang::fg::red << "Error couldn't open file: " << file
                    << ".json" << rang::style::reset << std::endl;
        } else {
          out << (*top_level)->GetMetaData().dump(4) << std::endl;
          out.close();
        }
      } else {
        std::cout << (*top_level)->GetMetaData().dump(4) << std::endl;
      }
    }
  });
  auto print =
      dev->add_subcommand(
             "print", "Converts to AST then outputs the AST as readable text")
          ->ignore_case()
          ->fallthrough();
  print->callback([&]() {
    if (verbose) std::cout << "Running subcommand `dev/print`\n";
    for (auto file : filenames) {
      using namespace porc;

      TokenStream stream(std::make_unique<CFileReader>(file.c_str()));
      err::PipeOutput(std::cerr);
      stream.ignore_comments = true;
      Parser parser = Parser(std::move(stream));
      auto top_level = parser.ParseFileDecl();
      if (!top_level) {
        std::cerr << rang::fg::red << "Error couldn't parse file: " << file
                  << " due to errors shown above" << rang::style::reset
                  << std::endl;
      } else {
        std::cout << "Succeeded the output is something along the lines of..."
                  << std::endl;
        std::cout << *top_level.value() << std::endl;
      }
    }
  });

  auto tokens = dev->add_subcommand("tokens", "Tokenizes the files")
                    ->ignore_case()
                    ->fallthrough();
  bool ignore_comments = false;
  tokens->add_flag("--remove-comments", ignore_comments,
                   "Remove comment tokens");
  tokens->callback([&]() {
    if (verbose) std::cout << "Running subcommand `dev/tokens`\n";

    for (auto file : filenames) {
      using namespace porc;

      std::cout << "\t== " << file << " ==" << std::endl;
      TokenStream stream(std::make_unique<CFileReader>(file.c_str()));
      stream.ignore_comments = ignore_comments;
      for (auto tok = stream.PopCur(); tok.type != Token::EndOfFile;
           tok = stream.PopCur()) {
        if (tok.type == Token::Undefined) {
          std::cerr << rang::fg::red << "ERROR: Undefined token"
                    << rang::style::reset << std::endl;
          break;
        }
        std::cout << "(" << tok.ToName() << "): " << tok.ToString()
                  << "\t:" << tok.pos << std::endl;
      }
      std::cout << "\t== Finished ==" << std::endl;
    }
  });

  app.callback([&] {
    if (verbose)
      std::cout << "Verbose mode activated"
                << "\n";
  });

  // so I don't destroy the terminals colours
  std::atexit([]() { std::cout << rang::style::reset; });
  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    if (verbose) print_version();

    // We will personally add colour to it since this makes it so ugly
    // std::cout << (e.get_exit_code()==0 ? rang::fg::blue : rang::fg::red);
    return app.exit(e);
  }

  return 0;
}

void print_version() {
  std::cout << rang::style::reset << rang::fg::blue << "Porc (CC) Version "
            << porc::kVersion << rang::style::reset << "\n";
}

void signal_handler(int s) {
  std::cout << rang::style::reset << rang::fg::red << rang::style::bold
            << "\n\nControl-C detected, exiting..." << rang::style::reset
            << std::endl;
  std::exit(1);
}
