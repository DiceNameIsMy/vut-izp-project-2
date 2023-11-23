from dataclasses import dataclass
import subprocess

import pytest


@dataclass
class RunResult:
    stdout: str
    stderr: str
    code: int

    def __str__(self):
        return (
            f"Run result: stdout: `{self.stdout}` stderr: `{self.stderr}` "
            + f"exit code: {self.code}"
        )

    def debugless_stderr(self) -> str:
        all_lines = self.stderr.split("\\n")
        debugless_lines = filter(lambda line: not line.startswith("[INF]"), all_lines)
        return "\\n".join(debugless_lines)


def run_command(command: str) -> RunResult:
    process = subprocess.run(
        command,
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    return RunResult(
        process.stdout.decode(),
        process.stderr.decode(),
        process.returncode,
    )


def run_program(args: str = "") -> RunResult:
    return run_command(f"./bin/maze {args}")


@pytest.fixture(scope="session", autouse=True)
def make():
    result = run_command("make")
    if result.code != 0:
        raise Exception(f"Error making the program: {result.stdout} \n {result.stderr}")


class TestInvalidArgs:
    HELP_TEXT = """Usage: ./maze [options] file...
Options:
  --help                     Display this information.
  --test                     Test if given file is a valid maze.
  --rpath <row> <column>     Solve maze using the right hand rule.
  --lpath <row> <column>     Solve maze using the left hand rule.
  --shortest <row> <column>  Solve maze by finding the shortest path to the exit.
"""
    INVALID_ARGS_MESSAGE = (
        "Invalid argument `{}`. Try `maze --help` for more information.\n"
    )
    INVALID_ARGS_AMOUNT_MESSAGE = (
        "Invalid amount of arguments. Try `maze --help` for more information.\n"
    )
    UNKNOWN_STRATEGY_MESSAGE = (
        "Unknown strategy `{}`. Try `maze --help` for more information.\n"
    )

    def test_no_flags(self):
        result = run_program()

        assert result.stdout == self.HELP_TEXT
        assert result.debugless_stderr() == ""
        assert result.code == 0

    def test_help_flag(self):
        result = run_program("--help")

        assert result.stdout == self.HELP_TEXT
        assert result.debugless_stderr() == ""
        assert result.code == 0

    def test_containts_help_flag(self):
        result = run_program("arg1 arg2 1 343 --help laa")

        assert result.stdout == self.HELP_TEXT
        assert result.debugless_stderr() == ""
        assert result.code == 0

    def test_more_than_1_flag(self):
        result = run_program("--rpath --lpath 1 1 tests/mazes/valid_maze.txt")

        assert result.code == 1
        assert result.stdout == ""
        assert result.debugless_stderr() == self.INVALID_ARGS_AMOUNT_MESSAGE

    def test_more_than_1_maze(self):
        result = run_program(
            "--rpath 1 1 tests/mazes/valid_maze.txt tests/mazes/valid_maze.txt"
        )

        assert result.code == 1
        assert result.stdout == ""
        assert result.debugless_stderr() == self.INVALID_ARGS_AMOUNT_MESSAGE

    def test_unknown_strategy(self):
        result = run_program("--undefined 1 1 tests/mazes/valid_maze.txt")

        assert result.code == 1
        assert result.stdout == ""
        assert result.debugless_stderr() == self.UNKNOWN_STRATEGY_MESSAGE.format(
            "--undefined"
        )


class TestMazeTesting:
    FILE_NOT_READABLE_MESSAGE = "Failed to read file.\n"

    def test_valid(self):
        result = run_program("--test tests/mazes/valid_maze.txt")

        assert result.code == 0
        assert result.stdout == "Valid\n"
        assert result.debugless_stderr() == ""

    def test_spaces_in_size_line(self):
        result = run_program("--test tests/mazes/many_spaces_in_size_line.txt")

        assert result.code == 0
        assert result.stdout == "Valid\n"
        assert result.debugless_stderr() == ""

    def test_spaces_between_cells(self):
        result = run_program("--test tests/mazes/many_spaces_between_cells.txt")

        assert result.code == 0
        assert result.stdout == "Valid\n"
        assert result.debugless_stderr() == ""

    def test_side_borders_mismatch(self):
        result = run_program("--test tests/mazes/side_border_mismatch.txt")

        assert result.code == 0
        assert result.stdout == "Invalid\n"
        assert result.debugless_stderr() == ""

    def test_updown_borders_mismatch(self):
        result = run_program("--test tests/mazes/updown_border_mismatch.txt")

        assert result.code == 1
        assert result.stdout == ""
        assert result.debugless_stderr() == self.FILE_NOT_READABLE_MESSAGE

    def test_zero_sized_maze(self):
        result = run_program("--test tests/mazes/zero_maze.txt")

        assert result.code == 0
        assert result.stdout == "Invalid\n"
        assert result.debugless_stderr() == ""

    def test_empty_maze(self):
        result = run_program("--test tests/mazes/empty_maze.txt")

        assert result.code == 0
        assert result.stdout == "Invalid\n"
        assert result.debugless_stderr() == ""

    def test_missing_cell(self):
        result = run_program("--test tests/mazes/missing_cell.txt")

        assert result.code == 0
        assert result.stdout == "Invalid\n"
        assert result.debugless_stderr() == ""

    def test_invalid_cell(self):
        result = run_program("--test tests/mazes/invalid_cell.txt")

        assert result.code == 0
        assert result.stdout == "Invalid\n"
        assert result.debugless_stderr() == ""

    def test_file_ends_abruptly(self):
        result = run_program("--test tests/mazes/file_ends_abruptly.txt")

        assert result.code == 0
        assert result.stdout == "Invalid\n"
        assert result.debugless_stderr() == ""

    def test_no_file(self):
        result = run_program("--test tests/mazes/not_existing.txt")

        assert result.code == 1
        assert result.stdout == ""
        assert result.debugless_stderr() == self.FILE_NOT_READABLE_MESSAGE


class TestRun:
    RPATH_OUTPUT = """6,1
6,2
5,2
5,3
5,4
6,4
6,3
6,4
6,5
6,6
5,6
5,7
4,7
4,6
4,5
4,4
3,4
3,5
3,6
3,5
3,4
3,3
3,2
3,1
2,1
2,2
2,3
2,4
2,5
2,6
2,7
3,7
"""
    LPATH_OUTPUT = """6,1
6,2
5,2
5,3
5,4
6,4
6,5
6,6
5,6
5,7
4,7
4,6
4,5
5,5
4,5
4,4
3,4
3,3
3,2
4,2
4,1
5,1
4,1
4,2
3,2
3,1
2,1
2,2
2,3
2,4
1,4
1,3
1,2
1,1
"""

    SHORTEST_OUTPUT = """6,1
6,2
5,2
5,3
5,4
6,4
6,5
6,6
5,6
5,7
4,7
4,6
4,5
4,4
3,4
3,3
3,2
3,1
2,1
2,2
2,3
2,4
2,5
2,6
2,7
3,7
3,8
"""

    def test_run_right_path(self):
        result = run_program("--rpath 6 1 tests/mazes/sample_maze.txt")

        assert result.code == 0, result.stderr
        assert result.stdout == self.RPATH_OUTPUT

    def test_run_left_path(self):
        result = run_program("--lpath 6 1 tests/mazes/sample_maze.txt")

        assert result.code == 0, result.stderr
        assert result.stdout == self.LPATH_OUTPUT

    def test_run_shortest(self):
        result = run_program("--shortest 6 1 tests/mazes/sample_maze.txt")

        assert result.code == 0, result.stderr
        assert result.stdout == self.SHORTEST_OUTPUT
