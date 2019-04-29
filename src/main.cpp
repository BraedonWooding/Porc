#include <iostream>
#include <string>
#include <vector>
#include <csignal>

#include <CLI11.hpp>
#include <rang.hpp>

#include "token_stream.hpp"
#include "defs.hpp"
#include "parser.hpp"

void signal_handler(int s);
void print_version();

int main(int argc, char *argv[]) {
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = signal_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, nullptr);

  CLI::App app{"Porc, the compiler/interpreter for Porc"};
  std::vector<std::string> filenames;
  app.add_option("input files,-i", filenames, "Input Files")
    ->required(true)->check(CLI::ExistingFile);
  app.require_subcommand(1);
  bool verbose;
  app.add_flag("-v,--verbose", verbose, "Verbose output");

  auto run = app.add_subcommand("run", "Run the given porc script")
    ->ignore_case()->fallthrough();
  auto build = app.add_subcommand("parse", "Parses the files")
    ->ignore_case()->fallthrough();
  auto dev = app.add_subcommand("dev", "A series of development options")
    ->ignore_case()->fallthrough();
  auto ast = dev->add_subcommand("ast", "Builds an AST")
    ->ignore_case()->fallthrough();
  ast->callback([&](){
    if (verbose) std::cout << "Running subcommand `dev/ast`";

    for (auto file: filenames) {
      using namespace porc::internals;

      std::cout << "\t== " << file << " ==" << std::endl;
      TokenStream stream(std::make_unique<CFileReader>(file.c_str()));
      Parser parser = Parser(std::move(stream));
      auto top_level = parser.ParseFileDecl();
      if (!top_level) std::cerr << "Error couldn't parse file" << std::endl;
      else std::cout << (*top_level)->GetMetaData() << std::endl;

      std::cout << "\t== Finished ==" << std::endl;
    }
  });
  auto tokens = dev->add_subcommand("tokens", "Tokenizes the files")
    ->ignore_case()->fallthrough();
  tokens->callback([&](){
    if (verbose) std::cout << "Running subcommand `dev/tokens`";

    for (auto file: filenames) {
      using namespace porc::internals;

      std::cout << "\t== " << file << " ==" << std::endl;
      TokenStream stream(std::make_unique<CFileReader>(file.c_str()));
      for (auto tok = stream.PopCur(); tok; tok = stream.PopCur()) {
        if (tok.type == Token::Undefined) {
          std::cerr << "ERROR: Undefined token" << std::endl;
          break;
        }
        std::cout << "(" << tok.ToName() << "): "
                  << tok.ToString() << std::endl;
      }
      std::cout << "\t== Finished ==" << std::endl;
    }
  });

  app.callback([&]{
    if (verbose) std::cout << "Verbose mode activated" << "\n";
  });

  std::atexit([](){std::cout << rang::style::reset;});
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
  std::cout << std::endl << rang::style::reset << rang::fg::red << rang::style::bold;
  std::cout << "Control-C detected, exiting..." << rang::style::reset << std::endl;
  std::exit(1); // will call the correct exit func, no unwinding of the stack though
}
