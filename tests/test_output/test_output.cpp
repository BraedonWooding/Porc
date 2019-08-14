#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <parser.hpp>
#include <ast.hpp>
#include <defs.hpp>

#include <sstream>

using namespace porc;

TEST_CASE( "Constants are outputted correctly", "[dev-print ast]" ) {
  err::PipeOutput(std::cerr);
  SECTION ( "Some simple cases" ) {
    std::string input = "10;\ntrue;\n\"Hello\";\n";
    TokenStream stream(std::make_unique<StringReader>(input));
    stream.ignore_comments = true;
    Parser parser = Parser(std::move(stream));
    auto top_level = parser.ParseFileDecl();
    REQUIRE(top_level);
    std::stringstream ss;
    ss << *top_level.value();
    REQUIRE(ss.str() == input);
  }
}