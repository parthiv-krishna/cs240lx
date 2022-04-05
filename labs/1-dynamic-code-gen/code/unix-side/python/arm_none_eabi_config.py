from assembler import ArmNoneEabiAssembler

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