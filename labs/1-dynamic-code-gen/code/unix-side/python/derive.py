import arm_none_eabi_config as config
# e.g. import risc_v_config as config ??

import bitwise
import instruction

import re

def solve_instruction(asm, instr_name, template, arg_names, options, regex):
    """Solves a template and prints the resulting assembly.
    
    Args:
        asm (Assembler): The assembler to use.
        instr_name (str): The name of the instruction.
        template (str): The template to solve.
        arg_names (list(str)): The names of the arguments specified in the template.
        options (dict): The options to choose from for the template arguments.
        regex (str): The regular expression to use to match the template arguments.

    Returns:
        Instruction: The instruction that was solved from the given information.
    """

    matches = list(re.finditer(regex, template))

    # should encode opcode bits
    instr_unchanged = None

    # dictionary mapping {argname: offset}
    args = {}

    if len(matches) != len(arg_names):
        raise RuntimeError(f"Template `{template}` has {len(matches)} arguments but there are {len(arg_names)} arg names: {arg_names}.")

    # iterate through each matched location in the template
    for match, arg_name in zip(matches, arg_names):

        # for the current location
        always_0 = None
        always_1 = None

        for option in options[match.group(0)]:
            # insert current option into selected location
            filled_template = template[:match.start()] + option + template[match.end():]
            for fmt in options:
                # set everything else to default value
                filled_template = filled_template.replace(fmt, options[fmt][0])
            
            # generate binary for instruction
            instr = asm.assemble(filled_template)

            # setup always_0 and always_1 to match length of instruction
            if instr_unchanged is None:
                instr_unchanged = bytes([0xFF for i in range(len(instr))]) 
            if always_0 is None:
                always_0 = bytes([0xFF for i in range(len(instr))])
                always_1 = bytes([0xFF for i in range(len(instr))])


            # print(filled_template, format(int.from_bytes(instr, byteorder="big"), 'x'))
            always_0 = bitwise.AND(always_0, bitwise.NOT(instr))
            always_1 = bitwise.AND(always_1, instr)

        both = bitwise.AND(always_0, always_1) 
        if (bitwise.to_int(both) != 0):
            raise RuntimeError(f"Impossible, some bits are both always 0 and 1: {both}")

        unchanged = bitwise.OR(always_0, always_1) # should be the instruction encoding
        my_bits = bitwise.NOT(unchanged)
        args[arg_name] = bitwise.FFS(my_bits) # assumes contiguous fields...


        instr_unchanged = bitwise.AND(instr_unchanged, unchanged) # update opcode bits

    opcode = bitwise.AND(instr_unchanged, instr)
    return instruction.Instruction(asm.arch(), instr_name, opcode, args)

def main():
    # Assembler object that gives .assemble and .cleanup methods
    asm = config.asm
    # List of (name, template, list of argument names)
    templates = config.templates
    # Dictionary of {argument: list of possible values}
    options = config.options
    # Regular expression to match arguments in templates
    regex = config.regex

    for instr_name, template, arg_names in templates:
        solved = solve_instruction(asm, instr_name, template, arg_names, options, regex)
        print(solved.to_c_function())


    asm.cleanup()

if __name__ == "__main__":
    main()