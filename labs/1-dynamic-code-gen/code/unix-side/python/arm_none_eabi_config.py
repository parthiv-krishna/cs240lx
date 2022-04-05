from assembler import ArmNoneEabiAssembler

asm = ArmNoneEabiAssembler({"memmap": "memmap",
                            "filepath": "./test"})

templates = [
    ("add", "add @r, @r, @r", ["dst", "src1", "src2"]),
    ("sub", "sub @r, @r, @r", ["dst", "src1", "src2"]),
    ("and", "and @r, @r, @r", ["dst", "src1", "src2"]),
    ("or", "orr @r, @r, @r", ["dst", "src1", "src2"]),
    ("mov_reg", "mov @r, @r", ["dst", "src"]),
    ("mov_imm", "mov @r, #@i", ["dst", "imm"]),
    ("ldr_no_off", "ldr @r, [@r]", ["dst", "addr"]),
    ("str_no_off", "str @r, [@r]", ["src", "addr"]),
    ("add_imm", "add @r, @r, #@i", ["dst", "src", "imm"]),
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
    ("add", [0, 0, 0], 0xe0800000), # add r0, r0, r0
    ("sub", [0, 0, 0], 0xe0400000), # sub r0, r0, r0
    ("and", [0, 0, 0], 0xe0000000), # and r0, r0, r0
    ("or", [0, 0, 0], 0xe0100000),  # orr r0, r0, r0
    ("mov_reg", [0, 0], 0xe1a00000), # mov r0, r0
    ("mov_imm", [0, 0], 0xe3a00000), # mov r0, #0
    ("ldr_no_off", [0, 0], 0xe5900000), # ldr r0, [r0]
    ("str_no_off", [0, 0], 0xe5800000), # str r0, [r0]
    ("add_imm", [0, 0, 0], 0xe2800000), # add r0, r0, #0
]