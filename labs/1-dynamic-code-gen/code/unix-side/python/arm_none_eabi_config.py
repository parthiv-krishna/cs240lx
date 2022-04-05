from assembler import ArmNoneEabiAssembler

asm = ArmNoneEabiAssembler({"memmap": "memmap",
                            "filepath": "./test"})

templates = {
    "add @r, @r, @r": ["dst", "src1", "src2"],
    "sub @r, @r, @r": ["dst", "src1", "src2"],
    "mov @r, @r" : ["dst", "src"],
    "ldr @r, [@r]" : ["dst", "addr"]
}

options = {
    "@r": ["r" + str(i) for i in range(16)]
}

regex = r"@([a-zA-Z0-9]+)"