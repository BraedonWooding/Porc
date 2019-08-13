#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <parser.hpp>
#include <ast.hpp>
#include <defs.hpp>

#include <sstream>

using namespace porc;

SCENARIO( "Constants should be loaded and output correctly", "[parser - Constant]" ) {
  GIVEN( "A loaded sample" ) {
      err::PipeOutput(std::cerr);
      WHEN ( "Some simple cases" ) {
        std::string input = "10;\ntrue;\n\"Hello\";\n";
        TokenStream stream(std::make_unique<StringReader>(input));
        stream.ignore_comments = true;
        Parser parser = Parser(std::move(stream));
        auto top_level = parser.ParseFileDecl();
        std::stringstream ss;

        THEN( "the output should match the input (relatively so)" ) {
          REQUIRE(top_level);
          ss << *top_level.value();
          REQUIRE(ss.str() == input);
        }
      }
  }
}