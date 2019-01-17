#include <iostream>
#include <string>
#include <vector>

#include "token_stream.hpp"
#include <CLI11.hpp>

/* this is an example of a custom error handler for when we want nicer crash messages */
// char *get_error_name(int err) {
//     if (err == IO_FILE_INVALID) {
//         return "File not found";
//     } else {
//         return "Unknown Error";
//     }
// }

int main(int argc, char *argv[]) {
  CLI::App app{"Porc, the compiler/interpreter for Porc"};
  std::vector<std::string> filenames;
  app.add_option("input files,-i", filenames, "Input Files")
    ->required(true)->check(CLI::ExistingFile);
  app.require_subcommand(1);

  auto run = app.add_subcommand("run", "Run the given porc script")
    ->ignore_case()->fallthrough();
  auto build = app.add_subcommand("parse", "Parses the files")
    ->ignore_case()->fallthrough();
  auto dev = app.add_subcommand("dev", "A series of development options")
    ->ignore_case()->fallthrough();
  auto token_stream = dev->add_subcommand("token_stream",
      "Shows the output of the tokenizer then exits")
    ->ignore_case()->fallthrough();
  token_stream->callback([&](){
    for (auto file: filenames) {
      std::cout << "\t== " << file << " ==" << std::endl;
      TokenStream stream(std::make_unique<CFileReader>(file.c_str()));
      for (auto tok = stream.Next(); !stream.IsDone(); tok = stream.Next()) {
        if (tok.type == TokenType::Undefined) {
          std::cerr << "ERROR: Undefined token" << std::endl;
          break;
        }
        std::cout << "(" << tok.ToName() << "): "
                  << tok.ToString() << std::endl;
      }
      std::cout << "\t== Finished ==" << std::endl;
    }
  });

  CLI11_PARSE(app, argc, argv);
  return 0;
}
