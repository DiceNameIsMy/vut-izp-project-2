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
        without_debug_lines = filter(
            lambda line: not line.startswith("src/"), all_lines
        )
        return "\\n".join(without_debug_lines)


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


class TestDisplayHelp:
    HELP_TEXT = """Usage: ./maze [options] file...
Options:
  --help
  --test
  --rpath
  --lpath
  --shortest
"""

    def test_provide_flag(self):
        result = run_program("--help")

        assert result.stdout == self.HELP_TEXT
        assert result.debugless_stderr() == ""
        assert result.code == 0

    def test_no_flags(self):
        result = run_program()

        assert result.stdout == self.HELP_TEXT
        assert result.debugless_stderr() == ""
        assert result.code == 0


class TestMazeTesting:
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

        assert result.code == 0
        assert result.stdout == "Invalid\n"
        assert result.debugless_stderr() == ""

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

    def test_no_file(self):
        result = run_program("--test tests/mazes/not_existing.txt")

        assert result.code == 0
        assert result.stdout == "Invalid\n"
        assert result.debugless_stderr() == ""


class TestRunSolver:
    INVALID_FLAG_MESSAGE = (
        "Only 1 flag can be provided. See --help for more information.\n"
    )
    INVALID_FILEPATH_MESSAGE = (
        "Only 1 maze file path can be provided. See --help for more information.\n"
    )

    def test_more_than_1_flag(self):
        result = run_program("--rpath --lpath tests/mazes/valid_maze.txt")

        assert result.code == 1
        assert result.stdout == ""
        assert result.debugless_stderr() == self.INVALID_FLAG_MESSAGE

    def test_more_than_1_maze_file_path(self):
        result = run_program(
            "--rpath tests/mazes/valid_maze.txt tests/mazes/valid_maze.txt"
        )

        assert result.code == 1
        assert result.stdout == ""
        assert result.debugless_stderr() == self.INVALID_FILEPATH_MESSAGE

    @pytest.mark.skip()
    def test_run_right_path(self):
        result = run_program("--rpath tests/mazes/valid_maze.txt")

        assert result.code == 0
        assert False

    @pytest.mark.skip()
    def test_run_left_path(self):
        result = run_program("--lpath tests/mazes/valid_maze.txt")

        assert result.code == 0
        assert False

    @pytest.mark.skip()
    def test_run_shortest(self):
        result = run_program("--shortest tests/mazes/valid_maze.txt")

        assert result.code == 0
        assert False
