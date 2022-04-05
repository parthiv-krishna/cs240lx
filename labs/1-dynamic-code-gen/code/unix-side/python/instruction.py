import bitwise

class Instruction():
    """Class representing an instruction"""
    arch = None
    name = None
    args = None
    opcode = None

    def __init__(self, arch, name, opcode, args):
        """Initializes the Instruction.

        Args:
            arch (str): The architecture name.
            name (str): The name of the instruction.
            opcode (bytes): The opcode of the instruction (always_0 | always_1)
            args (dict): Dictionary mapping argument names to their offsets
        """
        self.arch = arch
        self.name = name
        self.opcode = opcode
        self.args = args


    def to_c_function(self):
        result = f"static uint32_t {self.arch}_{self.name}("
        for i, arg in enumerate(self.args):
            if (i == 0):
                result += f"uint32_t {arg}"
            else:
                result += f", uint32_t {arg}"

        result += ") {\n"

        result += f"    return 0x{bitwise.to_hex(self.opcode)}"
        for i, arg in enumerate(self.args):
            result += f" |\n    ({arg} << {self.args[arg]})"
        
        result += ";\n}"

        return result

