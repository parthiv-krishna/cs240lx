import os
import subprocess

from assembler import Assembler

class ArmNoneEabiAssembler(Assembler):
    """arm-none-eabi assembler (for r/pi)"""

    def __init__(self, config_dict):
        """Sets up an instance of an ArmNoneEabiAssembler.
        
        Args:
            config_dict: A dictionary containing the assembler configuration.
        """

        self.memmap = config_dict["memmap"]
        self.asm_file = config_dict["filepath"] + ".S"
        self.obj_file = config_dict["filepath"] + ".o"
        self.bin_file = config_dict["filepath"] + ".bin"


    def assemble(self, instruction):
        """Assembles a given file and returns the resulting binary.

        Args:
            filename (str): The name of the file to assemble.

        Returns:
            bytes: The assembled binary.
        """

        # Create temporary assembly file
        with open(self.asm_file, "w+") as f:
            f.write(instruction + "\n")

        
        # Assemble the file
        retcode = subprocess.call(["arm-none-eabi-gcc", "-c", self.asm_file, "-o", self.obj_file])
        if retcode != 0:
            raise RuntimeError(f"Error assembling instruction: `{instruction}`")

        # Link the object file into a binary
        retcode = subprocess.call(["arm-none-eabi-objcopy", self.obj_file, "-O", "binary", self.bin_file])
        if retcode != 0:
            raise RuntimeError(f"Error objcopying: `{self.obj_file}`")

        # Read the binary
        with open(self.bin_file, "rb") as f:
            binary = f.read()

        if (len(binary) != 4):
            raise RuntimeError(f"Expected 4 byte instruction but got {len(binary)} bytes!")

        # convert from little endian
        return bytes([binary[3], binary[2], binary[1], binary[0]])


    def cleanup(self):
        """Cleans up any temporary files created during the assembly process."""

        os.remove(self.asm_file)
        os.remove(self.obj_file)
        os.remove(self.bin_file)

    def arch(self):
        """Returns a string denoting the assembler architecture."""
        return "arm"

asm = ArmNoneEabiAssembler({"memmap": "memmap",
                            "filepath": "./test"})

templates = [
    ("add", "add @r, @r, @r", ["dst", "src1", "src2"]),
    ("add_imm", "add @r, @r, #@i", ["dst", "src", "imm"]),
    ("sub", "sub @r, @r, @r", ["dst", "src1", "src2"]),
    ("sub_imm", "sub @r, @r, #@i", ["dst", "src", "imm"]),
    ("and", "and @r, @r, @r", ["dst", "src1", "src2"]),
    ("or", "orr @r, @r, @r", ["dst", "src1", "src2"]),
    ("mov_reg", "mov @r, @r", ["dst", "src"]),
    ("mov_imm", "mov @r, #@i", ["dst", "imm"]),
    ("ldr_no_off", "ldr @r, [@r]", ["dst", "addr"]),
    ("ldr_imm_off", "ldr @r, [@r, #@i]", ["dst", "addr", "offset"]),
    ("str_no_off", "str @r, [@r]", ["src", "addr"]),
    ("str_imm_off", "str @r, [@r, #@i]", ["src", "addr", "offset"]),
    ("nop", "nop", []),
    # ("b", "b @b", ["offset"]),
]

options = {
    # 16 registers
    "@r": ["r" + str(i) for i in range(16)],
    # 8 bit immediate
    "@i": [str(i) for i in range(256)],
    # branch offset: 24 bit immediate, just try each individual bit set
    # since 2^24 is a lot of possibilities
    "@b": [str(1 << i) for i in range(24)],
}

regex = r"@([a-zA-Z0-9]+)"

output_file = "./arm-none-eabi-insts.h"

test_cases = [
    ("add", [0, 0, 0], "add r0, r0, r0"),
    ("add", [4, 12, 1], "add r4, r12, r1"),
    ("add_imm", [0, 0, 0], "add r0, r0, #0"),
    ("add_imm", [5, 3, 212], "add r5, r3, #212"),
    ("sub", [0, 0, 0], "sub r0, r0, r0"),
    ("sub", [3, 2, 1], "sub r3, r2, r1"),
    ("sub_imm", [0, 0, 0], "sub r0, r0, #0"),
    ("sub_imm", [8, 2, 82], "sub r8, r2, #82"),
    ("and", [0, 0, 0], "and r0, r0, r0"),
    ("and", [1, 2, 3], "and r1, r2, r3"),
    ("or", [0, 0, 0], "orr r0, r0, r0"),
    ("or", [4, 5, 7], "orr r4, r5, r7"),
    ("mov_reg", [0, 0], "mov r0, r0"),
    ("mov_reg", [5, 10], "mov r5, r10"),
    ("mov_imm", [0, 0], "mov r0, #0"),
    ("mov_imm", [2, 91], "mov r2, #91"),
    ("ldr_no_off", [0, 0], "ldr r0, [r0]"),
    ("ldr_no_off", [9, 2], "ldr r9, [r2]"),
    ("ldr_imm_off", [0, 0, 0], "ldr r0, [r0, #0]"),
    ("ldr_imm_off", [1, 2, 3], "ldr r1, [r2, #3]"),
    ("str_no_off", [0, 0], "str r0, [r0]"),
    ("str_no_off", [3, 0], "str r3, [r0]"),
    ("str_imm_off", [0, 0, 0], "str r0, [r0, #0]"),
    ("str_imm_off", [5, 0, 14], "str r5, [r0, #14]"),
    ("nop", [], "nop")
]