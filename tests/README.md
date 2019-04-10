# Tests

Basically these tests are going to contain Porc code they are going to be ran in two ways;

## By manual style

```rust
porc = @import("Porc");

fn main() {
  parser = porc.Parser();
  runner = porc.Interpreter();

  context = @ContextCreate(Runner);
  // redirect output to log file
  @ContextSet(context, (stdout: io.File("tests_stdout.log", .Write),
                        stderr: io.File("tests_stderr.log", .Write)));
  // just make the asserts act as asserts and don't get all the extra input
  // silent like
  runner.load_context(context);
  parser.preload_context(context, .LoadedInInterpreter);
  // disable the testing macros and just expand the last arg as a replacement
  // this is so we don't get all the extra text output like `porc test -qq`
  // if you are just using asserts then you wouldn't need this line
  parser.disable_macros(("testing.test", "testing.bench"), expand_args: -1); // expand just the last arg
  tests_passed, total_tests = 0;

  // Presuming it is run in the base directory
  for test in (@io.CurDirectory() + Path("tests")).glob("**/*.porc") {
    // load but don't run it yet
    // it will just get the full path on conversion to string
    // but this way is much more explicit
    script = parser.parse(test.full_path());
    exit_status = runner.interpret(script);
    test_name = test.file_name(extension: false);
    total_tests++;
    // in reality you could force it to just support int/err/void
    // but this is a nice way to try to support as much as you can
    switch exit_status {
      err =>
        println("Error from ${test_name}: ${exit_status}");
      int if exit_status != 0 =>
        println("Got a non zero return code when running ${test_name}: {exit_status}");
      str =>
        println("Got a non-void/int/err return from ${test_name} that is string representable: ${exit_status}");
      else if exit_status {
        // can't convert to string but is non-void so we can try to do a debug
        // representation
        raw_form = @porc.debug_string(exit_status);
        if raw_form {
          println("Got a non-void/int/err return from ${test_name} that is not string representable; the 'raw' form is: ${raw_form}");
        }
      }
      else => {
        println("Tests passed for ${test_name}");
        tests_passed++;
      }
    }

    println("${tests_passed}/${total_tests} tests passed");
  }
}
```

## Using the inbuilt test library

You can give directories and it'll find all the porc files (recursively)

```bash
porc test tests
```

If you wanted to have the same output as the manual style then you just need the quiet option i.e. `porc test tests -qq`. (one `-q` means quiet but it'll still print out a lot more information such as indicating sub tests or using `.` for each assert where as two means no input but the bare minimum).

If you wanted literally no output whatsoever you use `-qqqq` and `-qqq` if you just want just the final line of the percentage passed.
