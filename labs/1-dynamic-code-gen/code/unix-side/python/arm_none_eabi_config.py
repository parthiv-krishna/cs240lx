from assembler import ArmNoneEabiAssembler

asm = ArmNoneEabiAssembler({"memmap": "memmap",
                            "filepath": "./test"})

templates = [
    ("add", "add @r, @r, @r", ["dst", "src1", "src2"]),
    ("sub", "sub @r, @r, @r", ["dst", "src1", "src2"]),
    ("mov_reg", "mov @r, @r", ["dst", "src"]),
    ("ldr_no_off", "ldr @r, [@r]", ["dst", "addr"]),
]

options = {
    "@r": ["r" + str(i) for i in range(16)]
}

regex = r"@([a-zA-Z0-9]+)"